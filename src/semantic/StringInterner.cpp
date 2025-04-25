#include "StringInterner.h"

namespace umbra {
StringInterner::StringInterner() {}

const std::string& 
StringInterner::intern(const std::string& str) {
    auto it = internedStrings.insert(str);
    return *it.first;
}

const std::string* 
StringInterner::get(const std::string& str) const {
    auto it = internedStrings.find(str);
    return (it != internedStrings.end()) ? &(*it) : nullptr;
}

}