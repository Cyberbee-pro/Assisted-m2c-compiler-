#include "../include/code_generator.h"
#include <fstream>
#include <stdexcept>
#include <unordered_set>

void CodeGenerator::generate(const std::vector<Statement> &statements, const std::string &outputFileName) const
{
    std::ofstream output(outputFileName);
    if (!output.is_open())
    {
        throw std::runtime_error("Could not open output file: " + outputFileName);
    }

    const OffsetMap offsets = buildVariableOffsets(statements);
    const int stackSize = stackSizeForVariableCount(offsets.size());

    output << ".section .rodata\n";

    std::vector<std::string> stringLabels(statements.size());
    int stringIndex = 0;
    for (std::size_t index = 0; index < statements.size(); ++index)
    {
        if (statements[index].type != StatementType::PrintString)
        {
            continue;
        }

        stringLabels[index] = "str" + std::to_string(stringIndex++);
        output << stringLabels[index] << ":\n";
        output << "    .string \"" << escapeAssemblyString(statements[index].text) << "\"\n";
    }

    output << "\n.text\n";
    output << ".globl main\n";
    output << "main:\n";
    output << "    pushq %rbp\n";
    output << "    movq %rsp, %rbp\n";
    if (stackSize > 0)
    {
        output << "    subq $" << stackSize << ", %rsp\n";
    }

    for (std::size_t index = 0; index < statements.size(); ++index)
    {
        const Statement &statement = statements[index];
        switch (statement.type)
        {
        case StatementType::PrintString:
            output << "    leaq " << stringLabels[index] << "(%rip), %rdi\n";
            output << "    call m2c_print\n";
            break;
        case StatementType::PrintValue:
            emitPrintValue(output, statement.firstOperand, offsets);
            break;
        case StatementType::VariableAssignment:
            emitLoadOperandToRegister(output, statement.firstOperand, offsets, "%eax");
            emitStoreFromEax(output, statement.target, offsets);
            break;
        case StatementType::ArithmeticAssignment:
            emitArithmeticCall(output, statement, offsets);
            break;
        }
    }

    output << "    movl $0, %eax\n";
    output << "    leave\n";
    output << "    ret\n";
}

std::string CodeGenerator::escapeAssemblyString(const std::string &value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (char current : value)
    {
        switch (current)
        {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += current;
            break;
        }
    }

    return escaped;
}

CodeGenerator::OffsetMap CodeGenerator::buildVariableOffsets(const std::vector<Statement> &statements)
{
    OffsetMap offsets;
    int nextOffset = 4;

    for (const Statement &statement : statements)
    {
        if ((statement.type == StatementType::VariableAssignment || statement.type == StatementType::ArithmeticAssignment) &&
            offsets.find(statement.target) == offsets.end())
        {
            offsets.emplace(statement.target, nextOffset);
            nextOffset += 4;
        }
    }

    return offsets;
}

int CodeGenerator::stackSizeForVariableCount(std::size_t variableCount)
{
    const int rawSize = static_cast<int>(variableCount * 4);
    if (rawSize == 0)
    {
        return 0;
    }

    const int alignment = 16;
    return ((rawSize + alignment - 1) / alignment) * alignment;
}

std::string CodeGenerator::memoryOperand(const OffsetMap &offsets, const std::string &identifier)
{
    return "-" + std::to_string(offsets.at(identifier)) + "(%rbp)";
}

void CodeGenerator::emitLoadOperandToRegister(std::ostream &output, const Operand &operand,
                                              const OffsetMap &offsets, const std::string &reg32)
{
    if (operand.type == TokenType::Number)
    {
        output << "    movl $" << operand.value << ", " << reg32 << "\n";
    }
    else
    {
        output << "    movl " << memoryOperand(offsets, operand.value) << ", " << reg32 << "\n";
    }
}

void CodeGenerator::emitStoreFromEax(std::ostream &output, const std::string &target, const OffsetMap &offsets)
{
    output << "    movl %eax, " << memoryOperand(offsets, target) << "\n";
}

void CodeGenerator::emitPrintValue(std::ostream &output, const Operand &operand, const OffsetMap &offsets)
{
    if (operand.type == TokenType::Number)
    {
        output << "    movl $" << operand.value << ", %edi\n";
    }
    else
    {
        output << "    movl " << memoryOperand(offsets, operand.value) << ", %edi\n";
    }

    output << "    call m2c_print_int\n";
}

void CodeGenerator::emitArithmeticCall(std::ostream &output, const Statement &statement, const OffsetMap &offsets)
{
    emitLoadOperandToRegister(output, statement.firstOperand, offsets, "%edi");
    emitLoadOperandToRegister(output, statement.secondOperand, offsets, "%esi");

    if (statement.operatorSymbol == "+")
    {
        output << "    call m2c_add\n";
    }
    else if (statement.operatorSymbol == "-")
    {
        output << "    call m2c_sub\n";
    }
    else if (statement.operatorSymbol == "*")
    {
        output << "    call m2c_mul\n";
    }
    else
    {
        output << "    call m2c_div\n";
    }

    emitStoreFromEax(output, statement.target, offsets);
}
