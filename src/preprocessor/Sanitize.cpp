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

    std::string Sanitizer::removeComments(std::string& inputBytes) {
        bool comentario = false;
        int indiceInicioComentario = -1;
        for (size_t i = 0; i < inputBytes.length(); i++) {
            if (i < inputBytes.length() - 1 && inputBytes[i] == '/' && inputBytes[i + 1] == '/' && !comentario) {
                indiceInicioComentario = i;
                comentario = true;
            }
            if (comentario) { // se está escaneando un comentario si comentario = true
                if (inputBytes[i] == '\n' || i == inputBytes.length() - 1) { // se llegó al final del comentario
                    if (i == inputBytes.length() - 1) {
                        inputBytes.erase(indiceInicioComentario, i - indiceInicioComentario + 1);
                    }
                    else {
                        inputBytes.erase(indiceInicioComentario, i - indiceInicioComentario); // si el comentario termina en "\n"
                                                                                              // se deja el salto de línea 
                    }
                    comentario = false;
                    i = indiceInicioComentario - 1;
                }
            }
        }
        return inputBytes;
    }

}
