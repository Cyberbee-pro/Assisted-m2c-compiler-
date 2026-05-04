#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>
#include <vector>
#include "parser.h"

class CodeGenerator
{
public:
    CodeGenerator(const std::vector<Statement> &statements, std::string outputFileName);

    void write() const;

private:
    const std::vector<Statement> &statements_;
    std::string outputFileName_;

    static std::string escapeCString(const std::string &value);
    static std::string renderOperand(const Operand &operand);
    static void emitArithmeticAssembly(std::ofstream &output, const Statement &statement, int tempIndex);
};

#endif
