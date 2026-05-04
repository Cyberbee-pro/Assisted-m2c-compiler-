#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

#include <string>
#include <utility>

enum class TokenType
{
    MainMarker,
    LoopMarker,
    ConditionalMarker,
    ExitMarker,
    LetKeyword,
    PrintStart,
    PrintEnd,
    Separator,
    BlockStart,
    BlockEnd,
    OpenParenthesis,
    CloseParenthesis,
    MorseString,
    Identifier,
    Number,
    Assignment,
    ArithmeticOperator,
    Comment
};

struct Token
{
    TokenType type;
    std::string value;
    int line;

    Token(TokenType tokenType, std::string tokenValue, int lineNumber)
        : type(tokenType), value(std::move(tokenValue)), line(lineNumber)
    {
    }
};

#endif
