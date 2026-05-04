#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "code_generator.h"
#include "excptsextra.h"
#include "lexer.h"
#include "parser.h"

namespace
{
struct CliOptions
{
    std::string inputFileName;
    std::string outputBinaryName;
};

CliOptions parseArguments(int argc, char *argv[])
{
    if (argc != 4 || std::string(argv[2]) != "-o")
    {
        throw std::invalid_argument("Usage: m2c <input.cym2c> -o <output_binary>");
    }

    return {argv[1], argv[3]};
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

        Lexer lexer;
        const std::vector<Token> tokens = lexer.lex(options.inputFileName);

        Parser parser;
        const std::vector<Statement> statements = parser.parse(tokens);

        CodeGenerator generator;
        const std::string assemblyFile = "temp.s";
        generator.generate(statements, assemblyFile);

        const std::string compileCommand =
            "gcc " + shellQuote(assemblyFile) +
            " " + shellQuote("runtime/io.s") +
            " " + shellQuote("runtime/math.s") +
            " -no-pie -o " + shellQuote(options.outputBinaryName);

        if (std::system(compileCommand.c_str()) != 0)
        {
            throw std::runtime_error("Failed to assemble and link output with command: " + compileCommand);
        }

        std::cout << "Generated assembly file: " << assemblyFile << std::endl;
        std::cout << "Generated executable: " << options.outputBinaryName << std::endl;
        return 0;
    }
    catch (const CompileError &error)
    {
        std::cerr << error.what() << std::endl;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << std::endl;
    }

    return 1;
}
