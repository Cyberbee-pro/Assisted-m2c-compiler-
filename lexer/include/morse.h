#ifndef MORSE_H
#define MORSE_H

#include <map>
#include <string>

// Keep enums if you need them elsewhere, but the maps will now use 'char'
enum TokenChar { A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z };
enum TokenNumeric { ZERO,ONE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE };

// FIX: Map Morse strings directly to char literals ('A', 'B', etc.)
inline std::map<std::string, char> charMap = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
    {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'},
    {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
    {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
    {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'}, {"--..", 'Z'}
};

inline std::map<std::string, char> numericMap = {
    {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'}, {"....-", '4'},
    {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'}, {"----.", '9'}
};

// Function prototypes
std::string morseType(std::string buffer);
bool isChar(std::string buffer);
bool isNum(std::string buffer);
bool isMorse(std::string buffer);
char morseToChar(std::string buffer);
std::string morse_parse(std::string &readLine,int &i);
std::string morseToStr(std::string buffer);

namespace morseVars{
    static inline std::string translatedToken = "String Not initialized";
    static inline std::string buffer = "";
    static inline std::string wrod = "";
}

#endif