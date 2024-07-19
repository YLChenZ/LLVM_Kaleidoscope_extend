#ifndef Z_LEXER_H
#define Z_LEXER_H

#include "token.h"
#include <iostream>
#include <fstream>
#include <cctype>


class Lexer {
private:
    std::ifstream file;
    char currentChar;

    void advance();  //向前读一个字符
    Token identifier();
    Token number();
    bool isOperator(char c);
    bool isParenthesis(char c);
public:
    
    Lexer(const std::string& filename);

    ~Lexer();

    Token getToken();  //从文件中获得Token
    void PrintTokens(); //打印文件包含的所有Tokens
    
    
};

#endif
