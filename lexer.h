#ifndef Z_LEXER_H
#define Z_LEXER_H

#include "token.h"
#include <iostream>
#include <fstream>
#include <cctype>
#include <vector>

class Lexer {
private:
    std::ifstream file;
    char currentChar;

    void advance();
    Token identifier();
    Token number();
    bool isOperator(char c);
    bool isParenthesis(char c);
public:
    
    Lexer(const std::string& filename);

    ~Lexer();

    Token getToken();
    
    std::vector<Token> getTokenVec();
    
    void PrintTokens();
    
    
};

#endif
