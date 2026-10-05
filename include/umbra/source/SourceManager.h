#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace umbra {

using FileId = std::uint32_t;

struct Span {
    FileId file;
    std::uint32_t begin;
    std::uint32_t end;
};

struct LineColumn {
    std::uint32_t line;   // One-based.
    std::uint32_t column; // One-based Unicode scalar position, not display width.
};

struct SourceError {
    std::filesystem::path path;
    std::string message;
    std::uint32_t byteOffset = 0;
};

class SourceFile {
public:
    FileId id() const noexcept { return id_; }
    const std::filesystem::path& path() const noexcept { return path_; }
    std::string_view text() const noexcept;
    std::string_view originalBytes() const noexcept { return bytes_; }
    std::uint32_t originalOffset(std::uint32_t textOffset) const;
    LineColumn lineColumn(std::uint32_t textOffset) const;

private:
    friend class SourceManager;
    SourceFile(FileId id, std::filesystem::path path, std::string bytes,
               std::uint32_t bomBytes);

    FileId id_;
    std::filesystem::path path_;
    std::string bytes_;
    std::uint32_t bomBytes_;
    std::vector<std::uint32_t> lineStarts_;
};

class SourceManager {
public:
    using LoadResult = std::variant<FileId, SourceError>;

    // Relative paths are resolved against the importer's directory, or the
    // process working directory for the entry file.
    LoadResult loadFile(const std::filesystem::path& path,
                        std::optional<FileId> importer = std::nullopt);
    const SourceFile& file(FileId id) const;
    std::size_t fileCount() const noexcept { return files_.size(); }

private:
    std::vector<std::unique_ptr<SourceFile>> files_;
    std::unordered_map<std::string, FileId> idsByPath_;
};

} // namespace umbra
