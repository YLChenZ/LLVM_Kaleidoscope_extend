#include "parser.h"


std::unique_ptr<ProgNode> InitAst() {
    return std::make_unique<ProgNode>(std::vector<std::unique_ptr<Node>>{});
}


void addNode(std::unique_ptr<ProgNode> root, std::unique_ptr<Node> node) {
    if (!root) {
        root = InitAst();
        root->defs.push_back(std::move(node));
    }
    root->defs.push_back(std::move(node));
}


void Parser::getNextToken(){
  CurTok = lexer.getToken();
}

Parser::Parser(const std::string& filename) : filename(filename), CurTok(Token(TokenAttr::Unknown," ")), Root(InitAst()), lexer {Lexer(filename)}
{
	//file = std::move(lexer.file);
	file.open(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file" << std::endl;
		exit(1);
	}
	
	getNextToken();
	
}

Parser::~Parser(){
	if (file.is_open()) {
		file.close();
	}
}


std::unique_ptr<ProgNode> Parser::getRoot(){
  return std::move(Root);
}


std::unique_ptr<Node> Parser::ParseError(const std::string& message){
  std::cout<<"Error: "<< message << '\n';
  return nullptr;
}

std::unique_ptr<Node> Parser::ParseNumExp(){
  if (CurTok.Attr == TokenAttr::Number){
    std::string numstr = CurTok.name;
    getNextToken();
    return std::make_unique<NumNode> (std::stoi(numstr));
  }  
  return ParseError("current Token is not a number!");
}

std::unique_ptr<Node> Parser::ParseVarExp(){
  if (CurTok.Attr == TokenAttr::Identifier){
    std::string vn = CurTok.name;
    getNextToken();
    return std::make_unique<VarNode> (vn);
  }  
  return ParseError("current Token is not a variable!");
}

std::unique_ptr<Node> Parser::ParseBinExp() {
  std::unique_ptr<Node> lhs;
  std::cout << CurTok.name <<'\n';
  if (CurTok.Attr == TokenAttr::Number)
    lhs = ParseNumExp();
  else if (CurTok.Attr == TokenAttr::Identifier)
    lhs = ParseVarExp();
  else 
    return ParseError("current Token is not an expression in BinExp LHS!");
  
  getNextToken();
  
  char Op;
  if (CurTok.Attr == TokenAttr::Operator)
    Op = CurTok.name[0];
  else 
    return ParseError("current Token is not an operator!");
  
  getNextToken();
  
  std::unique_ptr<Node> rhs;
  if (CurTok.Attr == TokenAttr::Number)
    rhs = ParseNumExp();
  else if (CurTok.Attr == TokenAttr::Identifier)
    rhs = ParseVarExp();
  else 
    return ParseError("current Token is not an expression in BinExp RHS!");
  
  getNextToken();
  
  return std::make_unique<BinExpNode> (Op, std::move(lhs), std::move(rhs));
}

std::unique_ptr<Node> Parser::ParseCalleeExp() {
  std::string Callee;
  if (CurTok.Attr == TokenAttr::Identifier)
    Callee = CurTok.name;
  else 
    return ParseError("current Token is not an Identifier in Callee!");
  
  getNextToken();
  
  if (CurTok.Attr == TokenAttr::Parenthesis)
    getNextToken();
  else 
    return ParseError("current Token is not a left Parenthesis in Callee!");
  
  std::vector<std::unique_ptr<Node>> Args;
  
  while (CurTok.Attr == TokenAttr::Number || CurTok.Attr == TokenAttr::Identifier) {
    if (CurTok.Attr == TokenAttr::Number) {
      auto arg = ParseNumExp();
      Args.push_back(std::move(arg));
      getNextToken();
    } else if (CurTok.Attr == TokenAttr::Identifier) {
      auto arg = ParseVarExp();
      Args.push_back(std::move(arg));
      getNextToken();
    }
  }
  
  if (CurTok.Attr == TokenAttr::Parenthesis) {
    getNextToken();
    return std::make_unique<CalleeExpNode> (Callee, std::move(Args));
  } else 
    return ParseError("current Token is not a right Parenthesis in Callee!");
}


std::unique_ptr<Node> Parser::ParseExp(){
  std::streampos prevPos = file.tellg();
  if (CurTok.Attr == TokenAttr::Number || CurTok.Attr == TokenAttr::Identifier)
    getNextToken();

  if (CurTok.Attr == TokenAttr::Operator) {
    file.seekg(prevPos);
    ParseBinExp();
  }
  
  else if (CurTok.Attr == TokenAttr::Parenthesis) {
    file.seekg(prevPos);
    ParseCalleeExp();
  }
  
  else {
    file.seekg(prevPos);
    if (CurTok.Attr == TokenAttr::Number)
      ParseNumExp();
    else if (CurTok.Attr == TokenAttr::Identifier)
      ParseVarExp();
  }
}

std::unique_ptr<Node> Parser::ParseFunDef(){
  if (CurTok.name == "def")
    getNextToken(); 
  else return ParseError("current Token is not def!");
  
  std::string fname;
  
  //std::cout<<CurTok.name<<'\n';
  
  if (CurTok.Attr == TokenAttr::Identifier){
    fname = CurTok.name;
    getNextToken();
  }
  else return ParseError("current Token is not an Identifier in Function!"); 
  
  if (CurTok.Attr == TokenAttr::Parenthesis)
    getNextToken();
  else return ParseError("current Token is not a left Parenthesis in Fun!");
  
  std::vector<std::string> Args;
  
  while (CurTok.Attr == TokenAttr::Identifier) {
    auto arg = CurTok.name;
    Args.push_back(arg);
    getNextToken();
  }
  
  if (CurTok.Attr == TokenAttr::Parenthesis)
    getNextToken();
  else return ParseError("current Token is not a right Parenthesis in Fun!");
  
  std::cout << CurTok.name << '\n';
  if (auto body = ParseExp())
    return std::make_unique<FunDefNode> (fname,std::move(Args),std::move(body));
  else return ParseError("The body of function: "+ fname + " can't be parsed!");
}

void Parser::ParseProgram(){
  std::unique_ptr<Node> funnode;
  if (CurTok.name == "def") {
    if (funnode = ParseFunDef())
      addNode(std::move(Root),std::move(funnode));
    else return;
  }
  getNextToken();
  
}

void Parser::PrintAst(std::unique_ptr<ProgNode> root){
  root->printinfo();
}
