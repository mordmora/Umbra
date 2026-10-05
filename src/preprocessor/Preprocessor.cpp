#include "umbra/preprocessor/Preprocessor.h"
#include <sstream>
#include <stdexcept>
#include <iostream> // Para depuración

namespace umbra {

Preprocessor::Preprocessor(const std::string& mainFilePath) {
    auto loaded = sourceManager.loadFile(mainFilePath);
    if (auto* error = std::get_if<SourceError>(&loaded)) {
        throw std::runtime_error(error->path.string() + ": " + error->message);
    }
    processedContent = processFile(std::get<FileId>(loaded), 0);
}

std::string Preprocessor::getProcessedContent() const {
    return processedContent;
}

std::optional<std::string> Preprocessor::parseUseDirective(const std::string& line) {
    std::stringstream ss(line);
    std::string keyword;
    ss >> keyword; // Lee la primera palabra

    if (keyword == "use") {
        std::string filePathWithQuotes;
        size_t start_pos = line.find_first_not_of(" \t", keyword.length());
        if (start_pos == std::string::npos) return std::nullopt;

        filePathWithQuotes = line.substr(start_pos);

        if (filePathWithQuotes.length() >= 2 && filePathWithQuotes.front() == '"' && filePathWithQuotes.back() == '"') {
            return filePathWithQuotes.substr(1, filePathWithQuotes.length() - 2);
        } else if (!filePathWithQuotes.empty() && filePathWithQuotes.find(' ') == std::string::npos) {
            return filePathWithQuotes;
        }
        std::cerr << "Warning: Malformed 'use' directive or unquoted path with spaces: " << line << std::endl;
        return std::nullopt;
    }
    return std::nullopt;
}

std::string Preprocessor::processFile(FileId fileId, int level) {
    const auto& source = sourceManager.file(fileId);
    if (level > MAX_INCLUDE_DEPTH) {
        throw std::runtime_error("Profundidad máxima de inclusión excedida, posible inclusión cíclica involucrando: " + source.path().string());
    }

    if (!includedFiles.insert(fileId).second) {
        return ""; // Ya incluido
    }

    std::stringstream content_buffer;
    std::string line;

    std::istringstream iss(std::string(source.text()));
    while (std::getline(iss, line)) {
        if (auto included_file_directive_path = parseUseDirective(line)) {
            auto loaded = sourceManager.loadFile(*included_file_directive_path, fileId);
            if (auto* error = std::get_if<SourceError>(&loaded)) {
                throw std::runtime_error(error->path.string() + ": " + error->message);
            }
            content_buffer << processFile(std::get<FileId>(loaded), level + 1);
        } else {
            content_buffer << line << '\n';
        }
    }

    return content_buffer.str();
}

} // namespace umbra
