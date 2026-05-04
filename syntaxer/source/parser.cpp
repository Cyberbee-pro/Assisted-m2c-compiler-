#include "../include/parser.h"
#include "../../lexer/include/excptsextra.h"
#include <sstream>

std::vector<Statement> Parser::parse(const std::vector<Token> &tokens)
{
    tokens_ = &tokens;
    currentIndex_ = 0;
    declaredVariables_.clear();

    std::vector<Statement> statements;

    while (const Token *token = peek())
    {
        switch (token->type)
        {
        case TokenType::MainMarker:
        case TokenType::BlockStart:
        case TokenType::BlockEnd:
        case TokenType::Comment:
        case TokenType::Separator:
            advance();
            break;
        case TokenType::PrintStart:
            statements.push_back(parsePrintStatement());
            break;
        case TokenType::LetKeyword:
            statements.push_back(parseAssignmentStatement());
            break;
        default:
            throw CompileError("***[Unexpected token in parser]***", buildLineSnippet(token->line), token->line);
        }
    }

    return statements;
}

Statement Parser::parsePrintStatement()
{
    const Token &printStart = expect(TokenType::PrintStart, "***[Expected < to start print statement]***");
    Statement statement;
    statement.line = printStart.line;

    const Token *valueToken = peek();
    if (valueToken == nullptr)
    {
        throw CompileError("***[Expected value after <]***", buildLineSnippet(printStart.line), printStart.line);
    }

    if (valueToken->type == TokenType::MorseString)
    {
        statement.type = StatementType::PrintString;
        statement.text = valueToken->value;
        advance();
    }
    else if (valueToken->type == TokenType::Identifier || valueToken->type == TokenType::Number)
    {
        statement.type = StatementType::PrintValue;
        statement.firstOperand = {valueToken->type, valueToken->value, valueToken->line};
        ensureIdentifierIsDeclared(statement.firstOperand, "***[Undeclared identifier in print statement]***");
        advance();
    }
    else
    {
        throw CompileError("***[Expected Morse string, identifier, or number after <]***",
                           buildLineSnippet(printStart.line), printStart.line);
    }

    expect(TokenType::PrintEnd, "***[Expected > after print value]***");

    const Token &terminator = expect(TokenType::Separator, "***[Expected ; after print statement]***");
    if (terminator.value != ";")
    {
        throw CompileError("***[Expected ; after print statement]***", buildLineSnippet(terminator.line), terminator.line);
    }

    return statement;
}

Statement Parser::parseAssignmentStatement()
{
    const Token &letToken = expect(TokenType::LetKeyword, "***[Expected let keyword]***");
    const Token &identifier = expect(TokenType::Identifier, "***[Expected identifier after let]***");
    expect(TokenType::Assignment, "***[Expected = after identifier]***");

    Statement statement;
    statement.target = identifier.value;
    statement.line = letToken.line;
    statement.firstOperand = parseOperand("***[Expected number or identifier after =]***");
    ensureIdentifierIsDeclared(statement.firstOperand, "***[Undeclared identifier in assignment]***");

    const Token *token = peek();
    if (token != nullptr && token->type == TokenType::ArithmeticOperator)
    {
        statement.type = StatementType::ArithmeticAssignment;
        statement.operatorSymbol = token->value;
        advance();
        statement.secondOperand = parseOperand("***[Expected number or identifier after arithmetic operator]***");
        ensureIdentifierIsDeclared(statement.secondOperand, "***[Undeclared identifier in arithmetic expression]***");
    }
    else
    {
        statement.type = StatementType::VariableAssignment;
    }

    const Token &terminator = expect(TokenType::Separator, "***[Expected ; after assignment]***");
    if (terminator.value != ";")
    {
        throw CompileError("***[Expected ; after assignment]***", buildLineSnippet(terminator.line), terminator.line);
    }

    declaredVariables_.insert(statement.target);
    return statement;
}

Operand Parser::parseOperand(const std::string &messagePrefix)
{
    const Token *token = peek();
    if (token == nullptr || (token->type != TokenType::Identifier && token->type != TokenType::Number))
    {
        const int line = token == nullptr ? 0 : token->line;
        throw CompileError(messagePrefix, buildLineSnippet(line), line);
    }

    Operand operand{token->type, token->value, token->line};
    advance();
    return operand;
}

const Token &Parser::expect(TokenType type, const std::string &message)
{
    const Token *token = peek();
    if (token == nullptr || token->type != type)
    {
        const int line = token == nullptr ? 0 : token->line;
        throw CompileError(message, buildLineSnippet(line), line);
    }

    advance();
    return (*tokens_)[currentIndex_ - 1];
}

const Token *Parser::peek() const
{
    if (tokens_ == nullptr || currentIndex_ >= tokens_->size())
    {
        return nullptr;
    }

    return &(*tokens_)[currentIndex_];
}

void Parser::advance()
{
    if (tokens_ != nullptr && currentIndex_ < tokens_->size())
    {
        ++currentIndex_;
    }
}

void Parser::ensureIdentifierIsDeclared(const Operand &operand, const std::string &messagePrefix) const
{
    if (operand.type == TokenType::Identifier && declaredVariables_.find(operand.value) == declaredVariables_.end())
    {
        throw CompileError(messagePrefix, buildLineSnippet(operand.line), operand.line);
    }
}

std::string Parser::buildLineSnippet(int line) const
{
    if (tokens_ == nullptr)
    {
        return "<unknown>";
    }

    std::ostringstream snippet;
    bool needsSpace = false;

    for (const Token &token : *tokens_)
    {
        if (token.line != line)
        {
            continue;
        }

        if (needsSpace && token.type != TokenType::Separator &&
            token.type != TokenType::PrintEnd && token.type != TokenType::CloseParenthesis &&
            token.type != TokenType::BlockEnd)
        {
            snippet << ' ';
        }

        snippet << token.value;
        needsSpace = true;
    }

    return snippet.str().empty() ? "<unknown>" : snippet.str();
}
