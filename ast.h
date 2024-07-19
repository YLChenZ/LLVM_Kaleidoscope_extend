#ifndef Z_AST_H
#define Z_AST_H

#include <memory>
#include <vector>
#include <string>
#include <iostream>

class Node{

public:
  virtual ~Node()=default;
  virtual void printinfo(int depth = 0) const = 0;  //打印节点信息
};


class ProgNode : public Node{
  
public:
  std::vector<std::unique_ptr<Node>> defs;
  ProgNode(std::vector<std::unique_ptr<Node>> defs) : defs{std::move(defs)} {}
  void printinfo(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "ProgNode\n";
        for (const auto& def : defs) {
            def->printinfo(depth + 2);
        }
  }
};

class VarNode : public Node{
public:
  std::string VarName;
  
  VarNode(const std::string& name) : VarName(name) {}
  void printinfo(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "VarNode: " << VarName << '\n';
  }
};


class NumNode : public Node{
public:
  int NumVal;
  
  NumNode(int num) : NumVal(num) {}
  void printinfo(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "NumNode: " << NumVal << '\n';
    }
};

class BinExpNode : public Node{
public:
  char Op;
  std::unique_ptr<Node> LHS,RHS;
  
  BinExpNode(char op,std::unique_ptr<Node> lhs,std::unique_ptr<Node> rhs)
  	: Op(op),LHS{std::move(lhs)},RHS{std::move(rhs)} {}
  	
  void printinfo(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "BinExpNode\n";
        
        LHS->printinfo(depth + 2);
        RHS->printinfo(depth + 2);
        
  }
};

class CalleeExpNode : public Node{
public:
  std::string Callee;
  std::vector<std::unique_ptr<Node>> CalleeArgs;
  
  CalleeExpNode(const std::string& name,std::vector<std::unique_ptr<Node>> args)
  	: Callee(name), CalleeArgs{std::move(args)} {}
  void printinfo(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "CalleeExpNode: " << Callee << '\n';
        for (const auto& arg : CalleeArgs) {
            arg->printinfo(depth + 2);
        }
  }
};

class FunDefNode : public Node{
public:
  std::string FunDefName;
  std::vector<std::string> FunDefArgs;
  std::unique_ptr<Node> FunDefBody;
  
  FunDefNode(const std::string& name,std::vector<std::string> args,std::unique_ptr<Node> body)
  	: FunDefName(name), FunDefArgs{std::move(args)}, FunDefBody{std::move(body)} {}
  	
   void printinfo(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "FunctionNode: " << FunDefName << '\n';
        std::cout << std::string(depth + 2, ' ') << "Params: ";
        for (const auto& arg : FunDefArgs) {
            std::cout << arg << " ";
        }
        std::cout << '\n';
        
        FunDefBody->printinfo(depth + 2);
    }
};

std::unique_ptr<ProgNode> InitAst();


void addNode(std::unique_ptr<ProgNode> root, std::unique_ptr<Node> node);



#endif
