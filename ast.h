#ifndef Z_AST_H
#define Z_AST_H

#include <memory>
#include <vector>
#include <string>
#include <iostream>


///*
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
//*/

using namespace llvm;

extern std::unique_ptr<LLVMContext> TheContext;
extern std::unique_ptr<IRBuilder<>> Builder;
extern std::unique_ptr<Module> TheModule;
extern std::map<std::string, AllocaInst*> NamedValues;



class Node{

public:
  virtual ~Node()=default;
  virtual void printinfo(int depth = 0) const = 0;
  virtual Value *codegen() = 0;
};


class ProgNode : public Node{
  
public:
  std::vector<std::unique_ptr<Node>> defs;
  ProgNode(std::vector<std::unique_ptr<Node>> defs);
  void printinfo(int depth = 0) const override;
  Value *codegen() override;
};


class StmtListNode : public Node{
  
public:
  std::vector<std::unique_ptr<Node>> stmts;
  StmtListNode(std::vector<std::unique_ptr<Node>> stmts);
  void printinfo(int depth = 0) const override;
  Value* codegen() override;
};

class VarNode : public Node{
public:
  std::string VarName;
  
  VarNode(const std::string& name);
  void printinfo(int depth = 0) const override;
  Value* codegen() override;
};


class NumNode : public Node{
public:
  int NumVal;
  
  NumNode(int num);
  void printinfo(int depth = 0) const override;
  Value* codegen() override;
};

class BinExpNode : public Node{
public:
  char Op;
  std::unique_ptr<Node> LHS,RHS;
  
  BinExpNode(char op,std::unique_ptr<Node> lhs,std::unique_ptr<Node> rhs);
  	
  void printinfo(int depth = 0) const override;
  Value* codegen() override;

};

class CalleeExpNode : public Node{
public:
  std::string Callee;
  std::vector<std::unique_ptr<Node>> CalleeArgs;
  
  CalleeExpNode(const std::string& name,std::vector<std::unique_ptr<Node>> args);
  void printinfo(int depth = 0) const override;
  Value* codegen() override;
};

class LetExpNode : public Node{
public:
  std::unique_ptr<Node> LetVar;
  std::unique_ptr<Node> LetBody;
  
  LetExpNode(std::unique_ptr<Node> var,std::unique_ptr<Node> exp);
  void printinfo(int depth = 0) const override;
  Value* codegen() override;
};


class FunDefNode : public Node{
public:
  std::string FunDefName;
  std::vector<std::string> FunDefArgs;
  std::unique_ptr<Node> FunDefBody;
  
  FunDefNode(const std::string& name,std::vector<std::string> args,std::unique_ptr<Node> body);

  	
  void printinfo(int depth = 0) const override;
  Function* codegen() override;

};


class IfExpNode : public Node{
public:
  std::unique_ptr<Node> Cond,Then,Else;
  
  IfExpNode(std::unique_ptr<Node> cond,std::unique_ptr<Node> then,std::unique_ptr<Node> els);
  
  void printinfo(int depth = 0) const override;
  Value* codegen() override;
};



std::unique_ptr<ProgNode> InitAst();


void addNode(std::unique_ptr<ProgNode>& root, std::unique_ptr<Node> node);

Value* Ast2IRError(const std::string& message);

void InitializeModule();

AllocaInst* CreateEntryBlockAlloca(Function *TheFunction, const std::string &VarName);


#endif
