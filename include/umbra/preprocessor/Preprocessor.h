#pragma once

#include <string>
#include <set>
#include <optional>
#include <filesystem> // C++17

namespace umbra {

const int MAX_INCLUDE_DEPTH = 32;

class Preprocessor {
public:

    explicit Preprocessor(const std::string& mainFilePath);
    std::string getProcessedContent() const;

private:
    std::string processedContent;
    std::set<std::string> includedFilesCanonicalPaths; // Almacena rutas canónicas

    std::string processFile(const std::filesystem::path& currentFileCanonicalPath, int level);

    std::optional<std::string> parseUseDirective(const std::string& line);

    std::filesystem::path resolveIncludePath(const std::filesystem::path& currentFileCanonicalPath,
                                             const std::string& includeDirectivePath);
};

} // namespace umbra
