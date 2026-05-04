#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "parser.h"

class CodeGenerator
{
public:
    void generate(const std::vector<Statement> &statements, const std::string &outputFileName) const;

private:
    using OffsetMap = std::unordered_map<std::string, int>;

    static std::string escapeAssemblyString(const std::string &value);
    static OffsetMap buildVariableOffsets(const std::vector<Statement> &statements);
    static int stackSizeForVariableCount(std::size_t variableCount);
    static std::string memoryOperand(const OffsetMap &offsets, const std::string &identifier);
    static void emitLoadOperandToRegister(std::ostream &output, const Operand &operand,
                                          const OffsetMap &offsets, const std::string &reg32);
    static void emitStoreFromEax(std::ostream &output, const std::string &target, const OffsetMap &offsets);
    static void emitPrintValue(std::ostream &output, const Operand &operand, const OffsetMap &offsets);
    static void emitArithmetic(std::ostream &output, const Statement &statement, const OffsetMap &offsets);
};

#endif
