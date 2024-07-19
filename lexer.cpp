#include "lexer.h"


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

void Lexer::advance() {
	currentChar = file.get();
}

Token Lexer::identifier() {
	std::string result;
	while (std::isalpha(currentChar) || std::isdigit(currentChar)) {
		result += currentChar;
		advance();
	}
	if (result == "def" || result == "return")
		return Token(TokenAttr::Keyword, result);
	return Token(TokenAttr::Identifier, result);
}

Token Lexer::number() {
	std::string result;
	while (std::isdigit(currentChar)) {
		result += currentChar;
		advance();
	}
	return Token(TokenAttr::Number, result);
}

bool Lexer::isOperator(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/';
}

bool Lexer::isParenthesis(char c) {
	return c == '(' || c == ')' || c == '{' || c == '}';
}

Lexer::Lexer(const std::string& filename) {
	file.open(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file" << std::endl;
		exit(1);
	}
	advance();
}

Lexer::~Lexer() {
	if (file.is_open()) {
		file.close();
	}
}

Token Lexer::getToken() {
	while (std::isspace(currentChar)) {
       	advance();
       }

	if (std::isalpha(currentChar)) {
		return identifier();
	}

	if (std::isdigit(currentChar)) {
		return number();
	}

	if (isOperator(currentChar)) {
		char op = currentChar;
		advance();
		return Token(TokenAttr::Operator, std::string(1, op));
	}

	if (isParenthesis(currentChar)) {
		char par = currentChar;
		advance();
		return Token(TokenAttr::Parenthesis, std::string(1, par));
	}
        
	if (currentChar == '='){
		advance();
		return Token(TokenAttr::Assignment, "=");
	}
	if (currentChar == EOF) {
		return Token(TokenAttr::EndOfFile, "");
	}

	char unknownChar = currentChar;
	advance();
	return Token(TokenAttr::Unknown, std::string(1, unknownChar));
}
        
void Lexer::PrintTokens()
{
	TokenTytoStr ttos = AttrToStringDic();
	Token tok = getToken();
	while (tok.Attr != TokenAttr::EndOfFile) {
		std::cout << "Token: { name: " << tok.name << ", Attr: " << ttos[tok.Attr] << " }\n";
		tok = getToken();
	} 
}


