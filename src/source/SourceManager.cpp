#include "umbra/source/SourceManager.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <system_error>

#include <utf8proc.h>

namespace umbra {

SourceFile::SourceFile(FileId id, std::filesystem::path path, std::string bytes,
                       std::uint32_t bomBytes)
    : id_(id), path_(std::move(path)), bytes_(std::move(bytes)),
      bomBytes_(bomBytes), lineStarts_{0} {
    const auto content = text();
    for (std::size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\n') {
            lineStarts_.push_back(static_cast<std::uint32_t>(i + 1));
        }
    }
}

std::string_view SourceFile::text() const noexcept {
    return std::string_view(bytes_).substr(bomBytes_);
}

std::uint32_t SourceFile::originalOffset(std::uint32_t textOffset) const {
    if (textOffset > text().size()) {
        throw std::out_of_range("Source offset exceeds file size");
    }
    return textOffset + bomBytes_;
}

LineColumn SourceFile::lineColumn(std::uint32_t textOffset) const {
    const auto content = text();
    if (textOffset > content.size()) {
        throw std::out_of_range("Source offset exceeds file size");
    }

    const auto line = std::upper_bound(lineStarts_.begin(), lineStarts_.end(), textOffset);
    const auto index = static_cast<std::size_t>(line - lineStarts_.begin() - 1);
    std::uint32_t column = 1;
    const auto* bytes = reinterpret_cast<const utf8proc_uint8_t*>(content.data());
    for (std::size_t i = lineStarts_[index]; i < textOffset;) {
        utf8proc_int32_t codepoint;
        const auto length = utf8proc_iterate(bytes + i,
            static_cast<utf8proc_ssize_t>(textOffset - i), &codepoint);
        if (length <= 0) {
            // A location inside a multibyte code point still refers to its
            // column. All valid tokens should use code point boundaries.
            break;
        }
        i += static_cast<std::size_t>(length);
        ++column;
    }
    return {static_cast<std::uint32_t>(index + 1), column};
}

SourceManager::LoadResult SourceManager::loadFile(
    const std::filesystem::path& path, std::optional<FileId> importer) {
    if (path.empty()) {
        return SourceError{path, "Source path is empty"};
    }
    if (importer && *importer >= files_.size()) {
        return SourceError{path, "Unknown importing file"};
    }

    std::error_code ec;
    auto resolved = path;
    if (resolved.is_relative()) {
        const auto base = importer ? file(*importer).path().parent_path()
                                   : std::filesystem::current_path(ec);
        if (ec) return SourceError{path, ec.message()};
        resolved = base / resolved;
    }
    resolved = std::filesystem::weakly_canonical(resolved, ec);
    if (ec) return SourceError{path, ec.message()};
    if (!std::filesystem::is_regular_file(resolved, ec)) {
        return SourceError{resolved, ec ? ec.message() : "Source is not a regular file"};
    }

    const auto key = resolved.string();
    if (const auto found = idsByPath_.find(key); found != idsByPath_.end()) {
        return found->second;
    }

    std::ifstream input(resolved, std::ios::binary);
    if (!input) return SourceError{resolved, "Could not open source file"};
    std::string bytes(std::istreambuf_iterator<char>{input}, {});
    if (input.bad()) return SourceError{resolved, "Could not read source file"};
    if (bytes.size() > std::numeric_limits<std::uint32_t>::max() - 3) {
        return SourceError{resolved, "Source file is too large"};
    }

    const bool hasBom = bytes.size() >= 3 &&
        static_cast<unsigned char>(bytes[0]) == 0xEF &&
        static_cast<unsigned char>(bytes[1]) == 0xBB &&
        static_cast<unsigned char>(bytes[2]) == 0xBF;
    const std::uint32_t bomBytes = hasBom ? 3 : 0;
    const auto* data = reinterpret_cast<const utf8proc_uint8_t*>(bytes.data());
    for (std::size_t offset = bomBytes; offset < bytes.size();) {
        utf8proc_int32_t codepoint;
        const auto length = utf8proc_iterate(data + offset,
            static_cast<utf8proc_ssize_t>(bytes.size() - offset), &codepoint);
        if (length <= 0) {
            return SourceError{resolved, "Invalid UTF-8 at byte " +
                                   std::to_string(offset) + " in source file",
                               static_cast<std::uint32_t>(offset)};
        }
        offset += static_cast<std::size_t>(length);
    }

    const auto id = static_cast<FileId>(files_.size());
    files_.push_back(std::unique_ptr<SourceFile>(
        new SourceFile(id, resolved, std::move(bytes), bomBytes)));
    idsByPath_.emplace(key, id);
    return id;
}

const SourceFile& SourceManager::file(FileId id) const {
    if (id >= files_.size()) {
        throw std::out_of_range("Unknown source file ID");
    }
    return *files_[id];
}

} // namespace umbra
