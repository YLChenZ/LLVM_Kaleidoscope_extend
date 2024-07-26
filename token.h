#ifndef Z_TOKEN_H
#define Z_TOKEN_H


#include <string>
#include <map>

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

typedef std::map<TokenAttr,std::string> TokenTytoStr;

TokenTytoStr AttrToStringDic();

class Token {
public:
    TokenAttr Attr;
    std::string name;

    Token(TokenAttr t, const std::string& n);
};

#endif
