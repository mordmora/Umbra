#pragma once
#include<string>
#include<vector>
#include<map>

namespace umbra {

struct Diagnosis {
    enum class Level {Error, Warning};
    Level level;
    size_t line;
    size_t col;
    std::string msg;
};

struct OutBuffer {
    std::string buffer;
    std::vector<Diagnosis> diagnosis;
};

enum class Encoding {
    UTF8,
    UTF16,
    UNICODE
};

class Sanitizer {
    public:

    Sanitizer(std::string& input) : input(input) {}

    std::string getNormalizedBuffer();

    //Detectar tipo de codificacion (UTF-8, UTF-16, Unicode etc)
    Encoding detectEnconding(std::string& inputBytes);

    //Remover Byte Order Mark (UTF-8, UTF-16)
    std::string stripBOM(std::string& inputBytes);

    //Convertir a UTF-8 si no lo está
    std::string convertInternalEncoding(std::string& inputBytes);

    //Aplicar NFC para normalizar el string
    std::string normalizeEncondign(std::string& inputBytes);

    //Limpiar el codigo fuente de caracteres de control
    std::string removeControlChars(std::string& inputBytes);

    //Eliminar comentarios (tener en cuenta comentarios de varias lineas)
    std::string removeComments(std::string& inputBytes);

    //Eliminar caracteres invisibles o no imprimibles
    std::string stripInvisibleChars(std::string& inputBytes);

    private:
    std::string& input;
    OutBuffer outBuff;
};

}
