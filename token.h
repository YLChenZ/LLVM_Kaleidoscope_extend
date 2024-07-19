#ifndef Z_TOKEN_H
#define Z_TOKEN_H


#include <string>
#include <map>

//Token的属性
enum class TokenAttr {
    Identifier,
    Number,
    Operator,
    Keyword,
    Parenthesis,
    Assignment,
    EndOfFile,
    Unknown
};

typedef std::map<TokenAttr,std::string> TokenTytoStr;   //为了方便打印Lexer的结果，将TokenAttr映射到string。

TokenTytoStr AttrToStringDic();

//暂时Token的设计，后面会加行号和位置
struct Token {
    TokenAttr Attr;
    std::string name;

    Token(TokenAttr t, const std::string& n) : Attr(t), name(n) {}
};

#endif
