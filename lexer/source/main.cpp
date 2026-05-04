#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "../include/code_generator.h"
#include "../include/excptsextra.h"
#include "../include/morse.h"
#include "../include/parser.h"
#include "../include/token_types.h"

namespace
{
struct CliOptions
{
    std::string inputFileName;
    std::string outputBinaryName;
};

class FileReader
{
public:
    explicit FileReader(std::string fileName) : fileName_(std::move(fileName)), inputFile_(fileName_)
    {
        if (!inputFile_.is_open())
        {
            throw std::runtime_error("Error: Could not open the file: " + fileName_);
        }
    }

    std::vector<Token> readFile()
    {
        tokens_.clear();

        std::string readLine;
        int lineNumber = 0;
        while (std::getline(inputFile_, readLine))
        {
            tokenizeLine(readLine, lineNumber);
            ++lineNumber;
        }

        return tokens_;
    }

private:
    std::string fileName_;
    std::ifstream inputFile_;
    std::vector<Token> tokens_;

    static std::string trim(const std::string &value)
    {
        const std::size_t begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
        {
            return "";
        }

        const std::size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(begin, end - begin + 1);
    }

    static bool isIdentifierToken(const std::string &buffer)
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

    static bool isNumberToken(const std::string &buffer)
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

    static std::size_t nextNonWhitespaceIndex(const std::string &line, std::size_t index)
    {
        while (index < line.length() && std::isspace(static_cast<unsigned char>(line[index])))
        {
            ++index;
        }

        return index;
    }

    static int previousNonWhitespaceIndex(const std::string &line, std::size_t index)
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

    static bool isDivisionOperator(const std::string &line, std::size_t index)
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

    void appendBufferToken(std::string &buffer, int lineNumber)
    {
        if (buffer.empty())
        {
            return;
        }

        if (buffer == "let")
        {
            tokens_.emplace_back(TokenType::LetKeyword, buffer, lineNumber);
        }
        else if (isNumberToken(buffer))
        {
            tokens_.emplace_back(TokenType::Number, buffer, lineNumber);
        }
        else if (isIdentifierToken(buffer))
        {
            tokens_.emplace_back(TokenType::Identifier, buffer, lineNumber);
        }
        else
        {
            throw CompileError("***[Invalid token encountered]***", buffer, lineNumber);
            exit(1); // <--- THIS IS THE CRITICAL FIX
        }

        buffer.clear();
    }

    void validateLineEnding(const std::string &line, int lineNumber) const
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
            exit(1); // <--- THIS IS THE CRITICAL FIX
        }
    }

    void tokenizeLine(const std::string &line, int lineNumber)
    {
        validateLineEnding(line, lineNumber);

        std::string buffer;
        for (std::size_t index = 0; index < line.length(); ++index)
        {
            const char current = line[index];

            if (current == '/' && index + 1 < line.length() && line[index + 1] == '/')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::Comment, line.substr(index), lineNumber);
                break;
            }

            if (std::isspace(static_cast<unsigned char>(current)))
            {
                appendBufferToken(buffer, lineNumber);
                continue;
            }

            if (current == '"')
            {
                appendBufferToken(buffer, lineNumber);
                int morseIndex = static_cast<int>(index) + 1;
                std::string mutableLine = line;
                const std::string translated = morse_parse(mutableLine, morseIndex);
                tokens_.emplace_back(TokenType::MorseString, translated, lineNumber);
                index = static_cast<std::size_t>(morseIndex);
                continue;
            }

            if (current == '/')
            {
                appendBufferToken(buffer, lineNumber);

                if (isDivisionOperator(line, index))
                {
                    tokens_.emplace_back(TokenType::ArithmeticOperator, "/", lineNumber);
                }
                else
                {
                    tokens_.emplace_back(TokenType::MainMarker, "/", lineNumber);
                }
                continue;
            }

            if (current == '{')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::BlockStart, "{", lineNumber);
                continue;
            }

            if (current == '}')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::BlockEnd, "}", lineNumber);
                continue;
            }

            if (current == ';' || current == ',')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::Separator, std::string(1, current), lineNumber);
                continue;
            }

            if (current == '<')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::PrintStart, "<", lineNumber);
                continue;
            }

            if (current == '>')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::PrintEnd, ">", lineNumber);
                continue;
            }

            if (current == '(')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::OpenParenthesis, "(", lineNumber);
                continue;
            }

            if (current == ')')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::CloseParenthesis, ")", lineNumber);
                continue;
            }

            if (current == '=')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::Assignment, "=", lineNumber);
                continue;
            }

            if (current == '+' || current == '-' || current == '*')
            {
                appendBufferToken(buffer, lineNumber);
                tokens_.emplace_back(TokenType::ArithmeticOperator, std::string(1, current), lineNumber);
                continue;
            }

            if (current == '%' || current == '~' || current == '^')
            {
                appendBufferToken(buffer, lineNumber);
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

                tokens_.emplace_back(type, value, lineNumber);
                continue;
            }

            buffer += current;
        }

        appendBufferToken(buffer, lineNumber);
    }
};

CliOptions parseArguments(int argc, char *argv[])
{
    if (argc != 4 || std::string(argv[2]) != "-o")
    {
        throw std::invalid_argument("Usage: m2c <input> -o <output_binary_name>");
    }

    return {argv[1], argv[3]};
}

std::string generatedSourceName(const std::string &outputBinaryName)
{
    return outputBinaryName + "_generated.cpp";
}

std::string shellQuote(const std::string &value)
{
    std::string quoted = "'";
    for (char current : value)
    {
        if (current == '\'')
        {
            quoted += "'\\''";
        }
        else
        {
            quoted += current;
        }
    }

    quoted += "'";
    return quoted;
}
}

int main(int argc, char *argv[])
{
    try
    {
        const CliOptions options = parseArguments(argc, argv);
        FileReader fileReader(options.inputFileName);
        const std::vector<Token> tokens = fileReader.readFile();

        Parser parser(tokens);
        const std::vector<Statement> statements = parser.parse();

        const std::string generatedCppFile = generatedSourceName(options.outputBinaryName);
        CodeGenerator generator(statements, generatedCppFile);
        generator.write();

        const std::string compileCommand =
            "g++ -std=c++17 " + shellQuote(generatedCppFile) + " -o " + shellQuote(options.outputBinaryName);
        const int compileResult = std::system(compileCommand.c_str());
        if (compileResult != 0)
        {
            throw std::runtime_error("Failed to build final executable with command: " + compileCommand);
        }

        std::cout << "Generated C++ file: " << generatedCppFile << std::endl;
        std::cout << "Generated executable: " << options.outputBinaryName << std::endl;
        return 0;
    }
    catch (const CompileError &error)
    {
        std::cerr << error.what() << std::endl;
        return 1; // <--- THIS IS THE CRITICAL FIX
    }
    catch (const std::invalid_argument &error)
    {
        std::cerr << error.what() << std::endl;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << std::endl;
    }

    return 1;
}
