#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <unordered_map>
#include <string>
#include<optional>
#include<memory>

namespace umbra {

using FILE_PATH = std::string;
/*
Declaration file for Umbra preprocessor
*/

/*
Estructura para mantener un control de donde se incluye el archivo
*/

struct SourceLocation {
    std::string path;
    unsigned int col;
    unsigned int line;
    SourceLocation(const std::string& path, std::size_t col, std::size_t line);
    SourceLocation();
};

/*
Para un mejor control, se define la estructure @File para tener una
representación mas completa del archivo, desde su ubicación en el
codigo fuente, su nombre y si fue resuelto o no.

Opcionalmente guarda el nombre del archivo que lo llamó en algún
lugar del codigo
*/

struct File {
    std::string fileName;
    SourceLocation location;
    bool resolved;
    std::optional<std::string> callBy = std::nullopt;

    bool equals(File);

    File();

    File(const std::string& fileName, SourceLocation location, bool resolved,
            const std::optional<std::string> &callBy);
};

/*
Se realizó una implementación de inclusión de archivos algo primitiva pero eficaz
y directa.

Para evitar las inclusiones ciclicas debido a la naturaleza recursiva de la
inclusión de archivos, se usó un Set de archivos para mantener un registro
de que archivos se han incluido
*/

class Preprocessor {
    private:
        File origin;
        bool fileExist(const std::ifstream &file);
        void getRelativePath();
        void detectByteOrderMark(std::ifstream& f);

        void getWorkingPath();
        std::unordered_map<std::string, File> included;
        std::pair<std::string, std::size_t> getWord(const std::string &input_str, std::size_t index);
        bool contains(File f);

    public:
        Preprocessor(File origin);
        FILE_PATH relativePath;
        FILE_PATH workingPath;
        void markAsResolved(File file);
        std::string out;
        std::string includeFiles(File inputFile, int level);
        void diagnosticTool();
};

}; // namespace umbra

#endif
