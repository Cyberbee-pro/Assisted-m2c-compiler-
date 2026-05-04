#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>
#include "token_types.h"

enum class StatementType
{
    PrintString,
    PrintValue,
    VariableAssignment,
    ArithmeticAssignment
};

struct Operand
{
    TokenType type = TokenType::Identifier;
    std::string value;
    int line = -1;
};

struct Statement
{
    StatementType type = StatementType::PrintString;
    std::string target;
    Operand firstOperand;
    Operand secondOperand;
    std::string operatorSymbol;
    std::string text;
    int line = -1;
};

class Parser
{
public:
    std::vector<Statement> parse(const std::vector<Token> &tokens);

private:
    const std::vector<Token> *tokens_ = nullptr;
    std::size_t currentIndex_ = 0;
    std::unordered_set<std::string> declaredVariables_;

    Statement parsePrintStatement();
    Statement parseAssignmentStatement();
    Operand parseOperand(const std::string &messagePrefix);
    const Token &expect(TokenType type, const std::string &message);
    const Token *peek() const;
    void advance();
    void ensureIdentifierIsDeclared(const Operand &operand, const std::string &messagePrefix) const;
    std::string buildLineSnippet(int line) const;
};

#endif
