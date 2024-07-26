#include "parser.h"


void Parser::getNextToken(){
  index++;
  CurTok = TokVec[index];
}

Parser::Parser(const std::string& filename) 
	: index(-1), filename(filename), CurTok(Token(TokenAttr::Unknown," ")), Root(InitAst()), lexer {Lexer(filename)}, TokVec{lexer.getTokenVec()}
{
	getNextToken();
}

Parser::~Parser(){};


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
    //getNextToken();
    return std::make_unique<NumNode> (std::stoi(numstr));
  }  
  return ParseError("Excepted a number!");
}

std::unique_ptr<Node> Parser::ParseVarExp(){
  if (CurTok.Attr == TokenAttr::Identifier){
    std::string vn = CurTok.name;
    //getNextToken();
    return std::make_unique<VarNode> (vn);
  }  
  return ParseError("Excepted a variable!");
}

std::unique_ptr<Node> Parser::ParseBinExp() {
  std::unique_ptr<Node> lhs;
  
  if (CurTok.Attr == TokenAttr::Number)
    lhs = ParseNumExp();
  else if (CurTok.Attr == TokenAttr::Identifier)
    lhs = ParseVarExp();
  else 
    return ParseError("Excepted an expression in BinExp's LHS!");
  
  getNextToken();
  
  char Op;
  if (CurTok.Attr == TokenAttr::Operator)
    Op = CurTok.name[0];
  else 
    return ParseError("Excepted an operator!");
  
  getNextToken();
  
  std::unique_ptr<Node> rhs;
  if (CurTok.Attr == TokenAttr::Number)
    rhs = ParseNumExp();
  else if (CurTok.Attr == TokenAttr::Identifier)
    rhs = ParseVarExp();
  else 
    return ParseError("Excepted an expression in BinExp's RHS!");
  
  //getNextToken();
  
  return std::make_unique<BinExpNode> (Op, std::move(lhs), std::move(rhs));
}

std::unique_ptr<Node> Parser::ParseCalleeExp() {
  std::string Callee;
  if (CurTok.Attr == TokenAttr::Identifier)
    Callee = CurTok.name;
  else 
    return ParseError("Excepted an Identifier in Callee!");
  
  getNextToken();
  
  if (CurTok.Attr == TokenAttr::Parenthesis && CurTok.name == "(")
    getNextToken();
  else 
    return ParseError("Excepted a left Parenthesis in Callee!");
  
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
  
  if (CurTok.Attr == TokenAttr::Parenthesis && CurTok.name == ")") {
    //getNextToken();
    return std::make_unique<CalleeExpNode> (Callee, std::move(Args));
  } else 
    return ParseError("Excepted a right Parenthesis in Callee!");
}


std::unique_ptr<Node> Parser::ParseLetExp(){
  if (CurTok.name == "let")
    getNextToken();
  else return ParseError("Excepted let!");
  
  std::unique_ptr<Node> letvar;
  
  if (CurTok.Attr == TokenAttr::Identifier){
    letvar = ParseVarExp();
    getNextToken();
  }
  else return ParseError("Excepted a variable!");
  
  if (CurTok.name == "=")
    getNextToken();
  else return ParseError("Excepted assignment '=' ");
  
  auto letbody = ParseExp();
  
  return std::make_unique<LetExpNode> (std::move(letvar),std::move(letbody));
    
}

std::unique_ptr<Node> Parser::ParseIfExp(){
  if (CurTok.name == "if")
    getNextToken();
  else return ParseError("Excepted if");
  
  auto cond = ParseExp();
  if (!cond) return ParseError("Excepted an expression in Condition");
  
  getNextToken();
  if (CurTok.name == "then")
    getNextToken();
  auto then = ParseExp();
  if (!then) return ParseError("Excepted an expression in then branch");
  
  getNextToken();
  if (CurTok.name == "else")
    getNextToken();
  auto els = ParseExp();
  if (!els) return ParseError("Excepted an expression in else branch");
  
  return std::make_unique<IfExpNode> (std::move(cond),std::move(then),std::move(els)); 
  
}


std::unique_ptr<Node> Parser::ParseExp(){
  if (CurTok.name == "let"){
    return ParseLetExp();
  }
  
  if (CurTok.name == "if"){
    return ParseIfExp();
  }
  
  if (CurTok.Attr == TokenAttr::Number || CurTok.Attr == TokenAttr::Identifier)
    getNextToken();
  
  if (CurTok.Attr == TokenAttr::Operator) {
    index--;
    CurTok = TokVec[index];
    return ParseBinExp();
  }
  
  if (CurTok.Attr == TokenAttr::Parenthesis) {
    index--;
    CurTok = TokVec[index];
    return ParseCalleeExp();
  }
  
  else {
    index--;
    CurTok = TokVec[index];
    if (CurTok.Attr == TokenAttr::Number)
      return ParseNumExp();
    else if (CurTok.Attr == TokenAttr::Identifier)
      return ParseVarExp();
  }
  return ParseError("Parse expression failed!");
}

std::unique_ptr<Node> Parser::ParseStmtList(){
  std::unique_ptr<Node> stmt;
  std::vector<std::unique_ptr<Node>> stmtlist;
  while (CurTok.name != ";"){
    stmt = ParseExp();
    stmtlist.push_back(std::move(stmt));
    getNextToken();
  }
  getNextToken();
  return std::make_unique<StmtListNode> (std::move(stmtlist));
}

std::unique_ptr<Node> Parser::ParseFunDef(){
  if (CurTok.name == "def")
    getNextToken(); 
  else return ParseError("Excepted def!");
  
  std::string fname;
  
  
  if (CurTok.Attr == TokenAttr::Identifier){
    fname = CurTok.name;
    getNextToken();
  }
  else return ParseError("Excepted an Identifier in Function!"); 
  
  if (CurTok.Attr == TokenAttr::Parenthesis && CurTok.name == "(")
    getNextToken();
  else return ParseError("Excepted a left Parenthesis in Fun!");
  
  std::vector<std::string> Args;
  
  while (CurTok.Attr == TokenAttr::Identifier) {
    auto arg = CurTok.name;
    Args.push_back(arg);
    getNextToken();
  }
  
  if (CurTok.Attr == TokenAttr::Parenthesis && CurTok.name == ")")
    getNextToken();
  else return ParseError("Excepted a right Parenthesis in Fun!");
  
  if (auto body = ParseStmtList())
    return std::make_unique<FunDefNode> (fname,std::move(Args),std::move(body));
  else return ParseError("The body of function: "+ fname + " can't be parsed!");
}

void Parser::ParseProgram(){
  std::unique_ptr<Node> funnode;
  while (CurTok.name != "$"){
    if (CurTok.name == "def") {
      if (funnode = ParseFunDef()) {
        std::cout<<"parse fun ok"<<'\n';
        addNode(Root,std::move(funnode));
        std::cout<<"add ok"<<'\n';
        std::cout<<CurTok.name<<'\n';
      }
      else return;
    }
    //getNextToken();
  }
  return;
}

void Parser::PrintAst(std::unique_ptr<ProgNode>& root){
  std::cout<<"start printing Ast"<<'\n';
  root->printinfo();
}

void Parser::PrintIR(std::unique_ptr<ProgNode>& root){
  std::cout<<"start printing IR"<<'\n';
  root->codegen();
}
