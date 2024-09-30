#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <set>
#include <string>
#include<experimental/optional>
#include<memory>

namespace umbra {
/*
Declaration file for Umbra preprocessor
*/
using namespace std::experimental;

struct SourceLocation {
    std::string path;
    unsigned int col;
    unsigned int line;
    SourceLocation(const std::string& path, unsigned int col, unsigned int line);
};

struct File {
    std::string fileName;
    SourceLocation location;
    bool resolved;
    optional<std::string> callBy = nullopt;

    File(const std::string& fileName, SourceLocation location, bool resolved,
         const optional<std::string> &callBy);
};

class Preprocessor {
    private:
        File origin;

        bool fileExist(const std::ifstream &file);
        std::set<File> included;
        std::pair<std::string, std::size_t> getWord(const std::string &input_str, std::size_t index);
        std::unique_ptr<std::string> readfile(std::string &filename);
        bool contains(File f);

        Preprocessor(File origin);
    public:
        std::string includeFiles(const std::string &filename, int leve);
};

}; // namespace umbra

#endif