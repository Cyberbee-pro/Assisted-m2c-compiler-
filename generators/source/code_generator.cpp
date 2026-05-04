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
    output << "fmt_int:\n";
    output << "    .string \"%d\\n\"\n";

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
            output << "    call puts@PLT\n";
            break;
        case StatementType::PrintValue:
            emitPrintValue(output, statement.firstOperand, offsets);
            break;
        case StatementType::VariableAssignment:
            emitLoadOperandToRegister(output, statement.firstOperand, offsets, "%eax");
            emitStoreFromEax(output, statement.target, offsets);
            break;
        case StatementType::ArithmeticAssignment:
            emitArithmetic(output, statement, offsets);
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
        output << "    movl $" << operand.value << ", %esi\n";
    }
    else
    {
        output << "    movl " << memoryOperand(offsets, operand.value) << ", %esi\n";
    }

    output << "    leaq fmt_int(%rip), %rdi\n";
    output << "    xorl %eax, %eax\n";
    output << "    call printf@PLT\n";
}

void CodeGenerator::emitArithmetic(std::ostream &output, const Statement &statement, const OffsetMap &offsets)
{
    emitLoadOperandToRegister(output, statement.firstOperand, offsets, "%eax");

    if (statement.operatorSymbol == "+")
    {
        if (statement.secondOperand.type == TokenType::Number)
        {
            output << "    addl $" << statement.secondOperand.value << ", %eax\n";
        }
        else
        {
            output << "    addl " << memoryOperand(offsets, statement.secondOperand.value) << ", %eax\n";
        }
    }
    else if (statement.operatorSymbol == "-")
    {
        if (statement.secondOperand.type == TokenType::Number)
        {
            output << "    subl $" << statement.secondOperand.value << ", %eax\n";
        }
        else
        {
            output << "    subl " << memoryOperand(offsets, statement.secondOperand.value) << ", %eax\n";
        }
    }
    else if (statement.operatorSymbol == "*")
    {
        emitLoadOperandToRegister(output, statement.secondOperand, offsets, "%ecx");
        output << "    imull %ecx, %eax\n";
    }
    else
    {
        output << "    cltd\n";
        emitLoadOperandToRegister(output, statement.secondOperand, offsets, "%ecx");
        output << "    idivl %ecx\n";
    }

    emitStoreFromEax(output, statement.target, offsets);
}
