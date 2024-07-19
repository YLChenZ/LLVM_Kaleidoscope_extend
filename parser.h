#ifndef Z_PARSER_H
#define Z_PARSER_H


#include "ast.h"
#include "lexer.h"


class Parser{
  std::ifstream file;
  Token CurTok;
  std::string filename;
  std::unique_ptr<ProgNode> Root;
  Lexer lexer;
public:
  Parser(const std::string& filename);

  ~Parser();
  
  std::unique_ptr<ProgNode> getRoot();
  
  void getNextToken();

  std::unique_ptr<Node> ParseError(const std::string& message);

  std::unique_ptr<Node> ParseVarExp();

  std::unique_ptr<Node> ParseNumExp();

  std::unique_ptr<Node> ParseBinExp();

  std::unique_ptr<Node> ParseCalleeExp();

  std::unique_ptr<Node> ParseExp();

  std::unique_ptr<Node> ParseFunDef();
  
  void ParseProgram();
  
  void PrintAst(std::unique_ptr<ProgNode> root);
 
};






#endif
