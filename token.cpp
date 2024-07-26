#include "token.h"

TokenTytoStr AttrToStringDic() 
{
  TokenTytoStr ttos = {{TokenAttr::Identifier,"ID"},
  			{TokenAttr::Number,"NUM"},
  			{TokenAttr::Operator,"OP"},
  			{TokenAttr::Keyword,"KEYWORD"},
  			{TokenAttr::Parenthesis,"PAREN"},
  			{TokenAttr::Assignment,"ASSIGN"},
  			{TokenAttr::EndOfFile,"EOF"},
  			{TokenAttr::Unknown,"UNKNOWN"}};
  return ttos;
}


Token::Token(TokenAttr t, const std::string& n) : Attr(t), name(n) { }
