#pragma once

#include <string>
#include <unordered_set>


namespace umbra
{
    
/**
 * Clase que interna cadenas para optimizar el uso de memoria 
 * y mejorar la eficiencia de comparaciones
 */
class StringInterner {
    public:
        StringInterner();
        
        // Interna una cadena y devuelve una referencia a la versión internada
        const std::string& intern(const std::string& str);
        
        // Obtiene un puntero a la cadena internada, o nullptr si no existe
        const std::string* get(const std::string& str) const;
    
    private:
        std::unordered_set<std::string> internedStrings;
    };
    
} // namespace umbra
