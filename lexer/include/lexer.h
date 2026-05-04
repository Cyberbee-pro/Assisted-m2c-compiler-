#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token_types.h"

class Lexer
{
public:
    std::vector<Token> lex(const std::string &fileName) const;

private:
    static std::string trim(const std::string &value);
    static bool isIdentifierToken(const std::string &buffer);
    static bool isNumberToken(const std::string &buffer);
    static std::size_t nextNonWhitespaceIndex(const std::string &line, std::size_t index);
    static int previousNonWhitespaceIndex(const std::string &line, std::size_t index);
    static bool isDivisionOperator(const std::string &line, std::size_t index);

    void tokenizeLine(const std::string &line, int lineNumber, std::vector<Token> &tokens) const;
    void appendBufferToken(std::string &buffer, int lineNumber, std::vector<Token> &tokens) const;
    void validateLineEnding(const std::string &line, int lineNumber) const;
};

#endif
