#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <optional>
#include <system_error>
#include "umbra/error/ErrorManager.h"

namespace umbra {

/**
 * @file UmbraIO.h
 * @brief Utilidades de lectura de archivos fuente para Umbra.
 * @details
 * - Lee el contenido completo de un archivo a memoria.
 * - Valida existencia y tipo de archivo (regular).
 * - Intenta reservar memoria según el tamaño para minimizar realojos.
 * - Elimina BOM UTF-8 si está presente.
 * - Reporta errores vía ErrorManager (opcional).
 *
 * Nota: No soporta lectura desde stdin por decisión de diseño actual.
 */
class UmbraIO {
public:
    /**
     * @brief Lee todo el archivo en 'out'.
     * @param file Ruta del archivo a leer.
     * @param out Buffer de salida (se limpia antes de leer).
     * @param err Gestor de errores opcional para reportar fallos.
     * @return true si la lectura fue exitosa; false si hubo error.
     */
    static bool readAll(const std::filesystem::path& file, std::string& out, ErrorManager* err = nullptr) {
        std::string msg;
        out.clear();

        std::error_code ec;
        if (!std::filesystem::exists(file, ec)) {
            msg = "El archivo " + file.string() + " no existe";
            reportError(err, msg);
            return false;
        }

        if (!std::filesystem::is_regular_file(file, ec)) {
            msg = "El archivo " + file.string() + " no es regular";
            reportError(err, msg);
            return false;
        }

        std::ifstream ifs(file, std::ios::binary);
        if (!ifs) {
            msg = "No se pudo abrir el archivo " + file.string();
            reportError(err, msg);
            return false;
        }

        std::uintmax_t sz = std::filesystem::file_size(file, ec);
        if (!ec && sz > 0) {
            try {
                out.resize(static_cast<size_t>(sz));
            } catch (...) {
                // Fallback a lectura por streambuf abajo
            }
        }

        if (!out.empty()) {
            ifs.read(out.data(), static_cast<std::streamsize>(out.size()));
            if (!ifs) {
                ifs.clear();
                ifs.seekg(0, std::ios::beg);
                std::ostringstream oss;
                oss << ifs.rdbuf();
                out = std::move(oss).str();
            }
        } else {
            std::ostringstream oss;
            oss << ifs.rdbuf();
            out = std::move(oss).str();
        }

        if (!ifs.good() && !ifs.eof()) {
            reportError(err, "Hubo un error al leer " + file.string());
            return false;
        }

        stripUtf8BOM(out);
        return true;
    }

    /**
     * @brief Variante que retorna el contenido o std::nullopt.
     * @param file Ruta del archivo a leer.
     * @param errMsg Mensaje de error opcional.
     */
    static std::optional<std::string> readAll(const std::filesystem::path& file, std::string* errMsg) {
        std::string buf;
        if (readAll(file, buf, nullptr)) {
            return buf;
        }
        if (errMsg) *errMsg = "UMBRA::IO::EXCEPTION: Fallo al leer " + file.string() + ".\n";
        return std::nullopt;
    }

private:
    /// Elimina un BOM UTF-8 (0xEF 0xBB 0xBF) al inicio del buffer si existe.
    static void stripUtf8BOM(std::string& s) {
        if (s.size() >= 3 &&
            static_cast<unsigned char>(s[0]) == 0xEF &&
            static_cast<unsigned char>(s[1]) == 0xBB &&
            static_cast<unsigned char>(s[2]) == 0xBF) {
            s.erase(0, 3);
        }
    }

    /// Reporta un error al ErrorManager (si no es nulo) con un prefijo consistente.
    static void reportError(ErrorManager* err, const std::string& msg) {
        if (!err) return;
        const std::string prefix = "UMBRA::IO::ERROR -> ";
        err->addError(std::make_unique<CompilerError>(ErrorType::IO, prefix + msg + "\n", 0, 0));
    }
};

} // namespace umbra
