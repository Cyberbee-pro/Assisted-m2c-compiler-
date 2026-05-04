#ifndef TOKENMAKER_H 
#define TOKENMAKER_H
#include <string>



class TokenMaker
{

private:
    std::string value;          // e.g., "%%"
    std::string type;           // e.g., "WHILE" or "KEYWORD"
    TokenMaker *next = nullptr; // Pointer to the next token in the linked list
    TokenMaker *prev = nullptr; // Pointer to the previous token in the linked list

    int Line; // Line number where the token was found

public:
    TokenMaker();

    TokenMaker(std::string value, std::string type, int Line);
    TokenMaker(std::string value, std::string type, int Line,TokenMaker *prev);

    // Getters and setters for value, type, next, prev, and Line
    std::string getValue() const;
    std::string getType() const;
    int getLine() const;
    TokenMaker* getNext() const;
    TokenMaker* getPrev() const;


    void setPrev(TokenMaker &) ;
    void setNext(TokenMaker &) ;




    // insert a new token after the current token
    void insertAfter(TokenMaker &,std::string,std::string,int);

    static void traverseLines(TokenMaker &);

    ~TokenMaker();
};


#endif