#include <string>
#include <iostream>
#include "../include/tokenMaker.h"
#include "../include/excptsextra.h"

TokenMaker::TokenMaker()
{
    next = nullptr;
    prev = nullptr;
    value = "";
    type = "";
    Line = -1;
}

TokenMaker::TokenMaker(std::string value, std::string type, int Line)
{
    this->value = value;
    this->type = type;
    this->Line = Line;
    next = nullptr;
    prev = nullptr;
}

TokenMaker::TokenMaker(std::string value, std::string type, int Line, TokenMaker *prev)
{
    this->value = value;
    this->type = type;
    this->Line = Line;
    next = nullptr;
    this->prev = prev;
}

// Getters and setters for value, type, next, prev, and Line
std::string TokenMaker::getValue() const { return value; }
std::string TokenMaker::getType() const { return type; }
int TokenMaker::getLine() const { return Line; }
TokenMaker *TokenMaker::getNext() const { return next; }
TokenMaker *TokenMaker::getPrev() const { return prev; }

// Removed trailing semicolons from these definitions
void TokenMaker::setPrev(TokenMaker &prev) { this->prev = &prev; }
void TokenMaker::setNext(TokenMaker &next) { this->next = &next; }

// insert a new token after the current token
void TokenMaker::insertAfter(TokenMaker &prevToken, std::string type, std::string value, int Line)
{
    if (&prevToken == nullptr)
    {
        std::cout << "Creating New Token stream . . . .  . " << std::endl;
        TokenMaker *newToken = new TokenMaker(value, type, Line);
    }
    else
    {
        std::cout << "Inserting at Line : " << Line << std::endl;
        TokenMaker *newToken = new TokenMaker(value, type, Line, &prevToken);
        TokenMaker *tempPrevToken = &prevToken;
        tempPrevToken->setNext(*newToken);
    }
}

void TokenMaker::traverseLines(TokenMaker &Head)
{
    TokenMaker *CurrentHead = &Head;
    if (CurrentHead->getPrev() != nullptr)
    {
        std::cout << "Provided Token Object is not a CurrentHead pointer " << std::endl;
        std::cout << "Proceeding Form Line:" << CurrentHead->getLine();
    }
    for (int i = 0; CurrentHead != nullptr; i++)
    {
        try
        {
            if (CurrentHead->getLine() == -1)
            {
                std::cout << "Token incorrectly made || does not belong to a line " << std::endl;
                throw CompileError("Token incorrectly made || line value = -1", CurrentHead->getType() + CurrentHead->getValue(), -1);
            }
            else if (CurrentHead->getNext() == nullptr && CurrentHead->getPrev() == nullptr)
            {
                std::cout << "Provided list is incomplete or disconected | no parent or clild tokens" << std::endl;
                std::cout << "Token type : " << CurrentHead->getType();
                std::cout << "Token Value : " << CurrentHead->getValue();
                std::cout << "Token in Line : " << CurrentHead->getLine();
            }
            else if (CurrentHead->getLine() != -1 && CurrentHead->getType() != "")
            {
                std::cout << "Token type : " << CurrentHead->getType();
                std::cout << "Token Value : " << CurrentHead->getValue();
                std::cout << "Token in Line : " << CurrentHead->getLine();
                if (CurrentHead->getPrev() == nullptr)
                {
                    std::cout << "CurrentHead Block . . . . . . " << std::endl;
                }
                else if (CurrentHead->getNext() == nullptr)
                {
                    std::cout << "Traverse end . . . . . . " << std::endl;
                }
            }
        }
        catch (const CompileError &e)
        {
            std::cerr << e.what() << std::endl;
        }
        CurrentHead = CurrentHead->getNext();
    }
}

TokenMaker::~TokenMaker()
{
    std::cout << "\nTokenMaker destructor called!!" << std::endl;
}