#pragma once

#include <string>
#include <set>
#include <optional>
#include <filesystem> // C++17
#include "umbra/source/SourceManager.h"

namespace umbra {

const int MAX_INCLUDE_DEPTH = 32;

class Preprocessor {
public:

    explicit Preprocessor(const std::string& mainFilePath);
    std::string getProcessedContent() const;

private:
    std::string processedContent;
    SourceManager sourceManager;
    std::set<FileId> includedFiles;

    std::string processFile(FileId fileId, int level);

    std::optional<std::string> parseUseDirective(const std::string& line);
};

} // namespace umbra
