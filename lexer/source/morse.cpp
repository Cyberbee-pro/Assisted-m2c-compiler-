#include "../include/morse.h"
#include <string>
#include <iostream>
#include <map>
#include "../include/morse.h"

bool isChar(std::string buffer)
{
    return charMap.find(buffer) != charMap.end();
}

bool isNum(std::string buffer)
{
    return numericMap.find(buffer) != numericMap.end();
}

bool isMorse(std::string buffer)
{
    return isChar(buffer) || isNum(buffer);
}

// Fixed morseToChar to return from our updated map
char morseToChar(std::string buffer)
{
    if (isChar(buffer))
        return charMap[buffer];
    if (isNum(buffer))
        return numericMap[buffer];
    return '?';
}

// Updated morseToStr to return the actual letter, not its ASCII number
std::string morseToStr(std::string buffer)
{
    if (isChar(buffer))
    {
        // This creates a string containing the actual letter (e.g., "A")
        return std::string(1, charMap[buffer]);
    }
    else if (isNum(buffer))
    {
        return std::string(1, numericMap[buffer]);
    }
    return "Not A Morse char/num";
}

std::string morseType(std::string buffer)
{
    if (isChar(buffer))
        return "Morse : Character";
    if (isNum(buffer))
        return "Morse : Numeric";
    return "Unknown";
}

std::string morse_parse(std::string &readLine, int &i)
{
    if (readLine[i + 1] == '\"')
    {
        std::cout<<"Skipping. . .. ";
        return morseVars::translatedToken;
    }
    morseVars::translatedToken = "";
    morseVars::buffer="";
    morseVars::wrod="";

    for (; i < static_cast<int>(readLine.length()); i++)
    {
        if(morseVars::buffer == "" && readLine[i] == ' '){
            morseVars::buffer = "";
        }else if (readLine[i] == ' ')
        {   
            if(isMorse(morseVars::buffer)){
                morseVars::wrod += morseToStr(morseVars::buffer);
                if (readLine[i + 1] == ' '||readLine[i + 1] == '\"'){
                    morseVars::translatedToken += morseVars::wrod;
                    morseVars::translatedToken += " ";
                    morseVars::wrod = "";
                }    
                morseVars::buffer = "";
            }
        }
        else if (readLine[i] == '\"')
        {
            if(isMorse(morseVars::buffer)){
                morseVars::wrod += morseToStr(morseVars::buffer);
                morseVars::translatedToken += morseVars::wrod;
                return morseVars::translatedToken;
            }else if(readLine[i-1] == ' ' && morseVars::buffer == ""){
                morseVars::translatedToken += morseVars::wrod;
                return morseVars::translatedToken;
            }
            return morseVars::translatedToken;
        }else{   
            morseVars::buffer += readLine[i];
        }

    }

    return morseVars::translatedToken;
}
