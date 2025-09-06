#include "umbra/preprocessor/Sanitize.h"
#include<string>

namespace umbra {

    std::string Sanitizer::convertInternalEncoding(std::string& inputBytes) {

    }

    std::string Sanitizer::stripBOM(std::string& inputBytes) {
        if (inputBytes.size() >= 3 &&
            static_cast<unsigned char>(inputBytes[0]) == 0xEF &&
            static_cast<unsigned char>(inputBytes[1]) == 0xBB &&
            static_cast<unsigned char>(inputBytes[2]) == 0xBF) {
            inputBytes.erase(0, 3);
        }
        else if (inputBytes.size() >= 2 &&
            ((static_cast<unsigned char>(inputBytes[0]) == 0xFF &&
            static_cast<unsigned char>(inputBytes[1]) == 0xFE) ||
            (static_cast<unsigned char>(inputBytes[0]) == 0xFE &&
            static_cast<unsigned char>(inputBytes[1]) == 0xFF))) {
            inputBytes.erase(0, 2);
        }
        return inputBytes;
    }
}
