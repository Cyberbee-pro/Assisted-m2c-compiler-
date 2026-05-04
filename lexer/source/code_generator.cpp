#include "../include/code_generator.h"
#include <fstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

CodeGenerator::CodeGenerator(const std::vector<Statement> &statements, std::string outputFileName)
    : statements_(statements), outputFileName_(std::move(outputFileName))
{
}

void CodeGenerator::write() const
{
    std::ofstream output(outputFileName_);
    if (!output.is_open())
    {
        throw std::runtime_error("Could not open output file: " + outputFileName_);
    }

    output << "#include <iostream>\n\n";
    output << "int main()\n";
    output << "{\n";

    std::unordered_set<std::string> declaredVariables;
    int tempIndex = 0;

    for (const Statement &statement : statements_)
    {
        switch (statement.type)
        {
        case StatementType::PrintString:
            output << "    std::cout << \"" << escapeCString(statement.text) << "\" << std::endl;\n";
            break;
        case StatementType::PrintValue:
            output << "    std::cout << " << renderOperand(statement.firstOperand) << " << std::endl;\n";
            break;
        case StatementType::VariableAssignment:
            if (declaredVariables.insert(statement.target).second)
            {
                output << "    int " << statement.target << " = " << renderOperand(statement.firstOperand) << ";\n";
            }
            else
            {
                output << "    " << statement.target << " = " << renderOperand(statement.firstOperand) << ";\n";
            }
            break;
        case StatementType::ArithmeticAssignment:
            if (declaredVariables.insert(statement.target).second)
            {
                output << "    int " << statement.target << " = 0;\n";
            }
            emitArithmeticAssembly(output, statement, tempIndex++);
            break;
        }
    }

    output << "    return 0;\n";
    output << "}\n";
}

std::string CodeGenerator::escapeCString(const std::string &value)
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

std::string CodeGenerator::renderOperand(const Operand &operand)
{
    return operand.value;
}

void CodeGenerator::emitArithmeticAssembly(std::ofstream &output, const Statement &statement, int tempIndex)
{
    const std::string leftName = "__m2c_left_" + std::to_string(tempIndex);
    const std::string rightName = "__m2c_right_" + std::to_string(tempIndex);

    output << "    int " << leftName << " = " << renderOperand(statement.firstOperand) << ";\n";
    output << "    int " << rightName << " = " << renderOperand(statement.secondOperand) << ";\n";
    output << "    __asm__ (\n";

    if (statement.operatorSymbol == "+")
    {
        output << "        \"movl %1, %%eax;\"\n";
        output << "        \"addl %2, %%eax;\"\n";
        output << "        \"movl %%eax, %0;\"\n";
        output << "        : \"=r\" (" << statement.target << ")\n";
        output << "        : \"r\" (" << leftName << "), \"r\" (" << rightName << ")\n";
        output << "        : \"%eax\"\n";
    }
    else if (statement.operatorSymbol == "-")
    {
        output << "        \"movl %1, %%eax;\"\n";
        output << "        \"subl %2, %%eax;\"\n";
        output << "        \"movl %%eax, %0;\"\n";
        output << "        : \"=r\" (" << statement.target << ")\n";
        output << "        : \"r\" (" << leftName << "), \"r\" (" << rightName << ")\n";
        output << "        : \"%eax\"\n";
    }
    else if (statement.operatorSymbol == "*")
    {
        output << "        \"movl %1, %%eax;\"\n";
        output << "        \"imull %2, %%eax;\"\n";
        output << "        \"movl %%eax, %0;\"\n";
        output << "        : \"=r\" (" << statement.target << ")\n";
        output << "        : \"r\" (" << leftName << "), \"r\" (" << rightName << ")\n";
        output << "        : \"%eax\"\n";
    }
    else
    {
        output << "        \"movl %1, %%eax;\"\n";
        output << "        \"cltd;\"\n";
        output << "        \"idivl %2;\"\n";
        output << "        \"movl %%eax, %0;\"\n";
        output << "        : \"=r\" (" << statement.target << ")\n";
        output << "        : \"r\" (" << leftName << "), \"r\" (" << rightName << ")\n";
        output << "        : \"%eax\", \"%edx\"\n";
    }

    output << "    );\n";
}
