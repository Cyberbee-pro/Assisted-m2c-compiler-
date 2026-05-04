#ifndef TOKENS_H
#define TOKENS_H

#include <map>
#include <string>

namespace M2C {
    // --- KEYWORDS ---
    static inline std::map<std::string, std::string> keywordsMap = {
        {"/",  "MAIN"},
        {"%",  "FOR"},
        {"%%", "WHILE"},
        {"~",  "IF"},
        {"~~", "ELSE"},
        {"<",  "PRINTST"},
        {">",  "PRINTEND"},
        {"^",  "EXIT"}
    };

    // --- SEPARATORS ---
    static inline std::map<std::string, std::string> separatorMap = {
        {"{", "CURL_ST"},
        {"}", "CURL_END"},
        {",", "COMA"},
        {";", "SEMI_COLON"},
        {" ", "SPACE"}
    };

    // --- CATEGORIES ---
    static inline std::map<std::string, std::string> categoryMap = {
        {"MAIN", "KEYWORD"}, {"FOR", "KEYWORD"}, {"WHILE", "KEYWORD"},
        {"IF", "KEYWORD"}, {"ELSE", "KEYWORD"}, {"PRINTST", "KEYWORD"}, {"PRINTEND", "KEYWORD"},
        {"EXIT", "KEYWORD"}, {"CURL_ST", "SEPARATOR"}, {"CURL_END", "SEPARATOR"},
        {"COMA", "SEPARATOR"}, {"SEMI_COLON", "SEPARATOR"}
    };
}

#endif