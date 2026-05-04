#include "../include/lexer.h"
#include "../include/excptsextra.h"
#include "../include/morse.h"
#include <cctype>
#include <fstream>
#include <stdexcept>

std::vector<Token> Lexer::lex(const std::string &fileName) const
{
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open())
    {
        throw std::runtime_error("Error: Could not open the file: " + fileName);
    }

    std::vector<Token> tokens;
    std::string readLine;
    int lineNumber = 0;

    while (std::getline(inputFile, readLine))
    {
        tokenizeLine(readLine, lineNumber, tokens);
        ++lineNumber;
    }

    return tokens;
}

std::string Lexer::trim(const std::string &value)
{
    const std::size_t begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
    {
        return "";
    }

    const std::size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

bool Lexer::isIdentifierToken(const std::string &buffer)
{
    if (buffer.empty() || (!std::isalpha(static_cast<unsigned char>(buffer[0])) && buffer[0] != '_'))
    {
        return false;
    }

    for (char current : buffer)
    {
        if (!std::isalnum(static_cast<unsigned char>(current)) && current != '_')
        {
            return false;
        }
    }

    return true;
}

bool Lexer::isNumberToken(const std::string &buffer)
{
    if (buffer.empty())
    {
        return false;
    }

    for (char current : buffer)
    {
        if (!std::isdigit(static_cast<unsigned char>(current)))
        {
            return false;
        }
    }

    return true;
}

std::size_t Lexer::nextNonWhitespaceIndex(const std::string &line, std::size_t index)
{
    while (index < line.length() && std::isspace(static_cast<unsigned char>(line[index])))
    {
        ++index;
    }

    return index;
}

int Lexer::previousNonWhitespaceIndex(const std::string &line, std::size_t index)
{
    if (index == 0)
    {
        return -1;
    }

    int currentIndex = static_cast<int>(index) - 1;
    while (currentIndex >= 0 && std::isspace(static_cast<unsigned char>(line[static_cast<std::size_t>(currentIndex)])))
    {
        --currentIndex;
    }

    return currentIndex;
}

bool Lexer::isDivisionOperator(const std::string &line, std::size_t index)
{
    const int previousIndex = previousNonWhitespaceIndex(line, index);
    const std::size_t nextIndex = nextNonWhitespaceIndex(line, index + 1);

    if (previousIndex < 0 || nextIndex >= line.length())
    {
        return false;
    }

    const char previous = line[static_cast<std::size_t>(previousIndex)];
    const char next = line[nextIndex];
    const bool leftValid = std::isalnum(static_cast<unsigned char>(previous)) || previous == '_' || previous == ')';
    const bool rightValid = std::isalnum(static_cast<unsigned char>(next)) || next == '_' || next == '(';
    return leftValid && rightValid;
}

void Lexer::appendBufferToken(std::string &buffer, int lineNumber, std::vector<Token> &tokens) const
{
    if (buffer.empty())
    {
        return;
    }

    if (buffer == "let")
    {
        tokens.emplace_back(TokenType::LetKeyword, buffer, lineNumber);
    }
    else if (isNumberToken(buffer))
    {
        tokens.emplace_back(TokenType::Number, buffer, lineNumber);
    }
    else if (isIdentifierToken(buffer))
    {
        tokens.emplace_back(TokenType::Identifier, buffer, lineNumber);
    }
    else
    {
        throw CompileError("***[Invalid token encountered]***", buffer, lineNumber);
    }

    buffer.clear();
}

void Lexer::validateLineEnding(const std::string &line, int lineNumber) const
{
    std::string lineWithoutComment = line;
    const std::size_t commentPosition = lineWithoutComment.find("//");
    if (commentPosition != std::string::npos)
    {
        lineWithoutComment = lineWithoutComment.substr(0, commentPosition);
    }

    const std::string trimmed = trim(lineWithoutComment);
    if (trimmed.empty())
    {
        return;
    }

    const char lastCharacter = trimmed.back();
    if (lastCharacter != ';' && lastCharacter != '{' && lastCharacter != '}')
    {
        throw CompileError("***[Expected ; in the end of line]***", line, lineNumber);
    }
}

void Lexer::tokenizeLine(const std::string &line, int lineNumber, std::vector<Token> &tokens) const
{
    validateLineEnding(line, lineNumber);

    std::string buffer;
    for (std::size_t index = 0; index < line.length(); ++index)
    {
        const char current = line[index];

        if (current == '/' && index + 1 < line.length() && line[index + 1] == '/')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::Comment, line.substr(index), lineNumber);
            break;
        }

        if (std::isspace(static_cast<unsigned char>(current)))
        {
            appendBufferToken(buffer, lineNumber, tokens);
            continue;
        }

        if (current == '"')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            int morseIndex = static_cast<int>(index) + 1;
            std::string mutableLine = line;
            const std::string translated = morse_parse(mutableLine, morseIndex);
            tokens.emplace_back(TokenType::MorseString, translated, lineNumber);
            index = static_cast<std::size_t>(morseIndex);
            continue;
        }

        if (current == '/')
        {
            appendBufferToken(buffer, lineNumber, tokens);

            if (isDivisionOperator(line, index))
            {
                tokens.emplace_back(TokenType::ArithmeticOperator, "/", lineNumber);
            }
            else
            {
                tokens.emplace_back(TokenType::MainMarker, "/", lineNumber);
            }
            continue;
        }

        if (current == '{')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::BlockStart, "{", lineNumber);
            continue;
        }

        if (current == '}')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::BlockEnd, "}", lineNumber);
            continue;
        }

        if (current == ';' || current == ',')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::Separator, std::string(1, current), lineNumber);
            continue;
        }

        if (current == '<')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::PrintStart, "<", lineNumber);
            continue;
        }

        if (current == '>')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::PrintEnd, ">", lineNumber);
            continue;
        }

        if (current == '(')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::OpenParenthesis, "(", lineNumber);
            continue;
        }

        if (current == ')')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::CloseParenthesis, ")", lineNumber);
            continue;
        }

        if (current == '=')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::Assignment, "=", lineNumber);
            continue;
        }

        if (current == '+' || current == '-' || current == '*')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            tokens.emplace_back(TokenType::ArithmeticOperator, std::string(1, current), lineNumber);
            continue;
        }

        if (current == '%' || current == '~' || current == '^')
        {
            appendBufferToken(buffer, lineNumber, tokens);
            TokenType type = TokenType::LoopMarker;
            if (current == '~')
            {
                type = TokenType::ConditionalMarker;
            }
            else if (current == '^')
            {
                type = TokenType::ExitMarker;
            }

            std::string value(1, current);
            if ((current == '%' || current == '~') && index + 1 < line.length() && line[index + 1] == current)
            {
                value += current;
                ++index;
            }

            tokens.emplace_back(type, value, lineNumber);
            continue;
        }

        buffer += current;
    }

    appendBufferToken(buffer, lineNumber, tokens);
}
