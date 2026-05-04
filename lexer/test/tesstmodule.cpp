#include <iostream>
#include <string>
#include "../include/tokenMaker.h"
#include "../include/excptsextra.h"

int main() {
    std::cout << "--- Starting TokenMaker Tests ---\n" << std::endl;

    // 1. Test basic constructor and initialization
    std::cout << "[Test 1] Creating Head Token" << std::endl;
    TokenMaker head("int", "KEYWORD", 1);

    // 2. Test insertAfter method
    std::cout << "\n[Test 2] Inserting Tokens via insertAfter()" << std::endl;
    
    // Insert 'main' after 'int'
    head.insertAfter(head, "IDENTIFIER", "main", 1);

    // Insert '(' after 'main'
    TokenMaker* second = head.getNext();
    if (second != nullptr) {
        second->insertAfter(*second, "SYMBOL", "(", 1);
    }

    // Insert ')' after '('
    TokenMaker* third = second != nullptr ? second->getNext() : nullptr;
    if (third != nullptr) {
        third->insertAfter(*third, "SYMBOL", ")", 1);
    }

    // 3. Test traverseLines method
    std::cout << "\n[Test 3] Traversing Tokens" << std::endl;
    head.traverseLines(head);

    // 4. Test error handling (Line = -1 will trigger CompileError)
    std::cout << "\n[Test 4] Testing Exception Handling (Line = -1)" << std::endl;
    TokenMaker badToken("bad_val", "ERROR", -1);
    
    // traverseLines catches CompileError internally based on your implementation
    badToken.traverseLines(badToken);

    // 5. Clean up heap allocated tokens from insertAfter
    std::cout << "\n--- Cleaning up heap memory ---" << std::endl;
    TokenMaker* current = head.getNext();
    while (current != nullptr) {
        TokenMaker* nextToken = current->getNext();
        delete current;
        current = nextToken;
    }

    std::cout << "\n--- Tests Completed ---" << std::endl;
    return 0;
}