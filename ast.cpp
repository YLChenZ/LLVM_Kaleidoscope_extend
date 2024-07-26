#include "ast.h"


std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<IRBuilder<>> Builder;
std::unique_ptr<Module> TheModule;
std::map<std::string, AllocaInst*> NamedValues;


/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
AllocaInst* CreateEntryBlockAlloca(Function *TheFunction, const std::string &VarName) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),TheFunction->getEntryBlock().begin());
  
  return TmpB.CreateAlloca(Type::getInt32Ty(*TheContext), nullptr,VarName);
}




void InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("my cool jit", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

ProgNode::ProgNode(std::vector<std::unique_ptr<Node>> defs) : defs{std::move(defs)} {}

void ProgNode::printinfo(int depth) const {
  std::cout << std::string(depth, ' ') << "ProgNode:\n";
  for (const auto& def : defs) {
    def->printinfo(depth + 2);
  }
}

Value* ProgNode::codegen() {
  for (auto &def : defs) {
    if (!def->codegen()) {
      return nullptr;
    }
  }
  return Constant::getNullValue(Type::getInt32Ty(*TheContext)); // 或其他适当的返回值
}


StmtListNode::StmtListNode(std::vector<std::unique_ptr<Node>> stmts) : stmts{std::move(stmts)} {}

void StmtListNode::printinfo(int depth) const {
  std::cout << std::string(depth, ' ') << "StmtListNode:\n";
  for (const auto& stmt : stmts) {
    stmt->printinfo(depth + 2);
  }
}

Value* StmtListNode::codegen() {
  Value* lastValue = nullptr;
  for (auto& stmt : stmts) {
    lastValue = stmt->codegen();
    if (!lastValue) {
      return nullptr;
    }
  }
  return lastValue; // 返回最后一个语句的值
}
  
VarNode::VarNode(const std::string& name) : VarName(name) {}

void VarNode::printinfo(int depth) const {
  std::cout << std::string(depth, ' ') << "VarNode: " << VarName << '\n';
}  

Value* VarNode::codegen() {
  // Look this variable up in the function.
  AllocaInst *A = NamedValues[VarName];
  if (!A)
    return Ast2IRError("Unknown variable name");

  // Load the value.
  return Builder->CreateLoad(A->getAllocatedType(), A, VarName.c_str());
}

  
NumNode::NumNode(int num) : NumVal(num) {}

void NumNode::printinfo(int depth) const{
  std::cout << std::string(depth, ' ') << "NumNode: " << NumVal << '\n';
}

Value* NumNode::codegen() {
  return ConstantInt::get(Type::getInt32Ty(*TheContext), NumVal);
}
  
BinExpNode::BinExpNode(char op,std::unique_ptr<Node> lhs,std::unique_ptr<Node> rhs)
	: Op(op), LHS{std::move(lhs)}, RHS{std::move(rhs)} {}
  	
void BinExpNode::printinfo(int depth) const {
  std::cout << std::string(depth, ' ') << "BinExpNode:\n";
    
  if (LHS)
    LHS->printinfo(depth + 2);
  else 
    std::cout << std::string(depth + 2, ' ') << "Nullptr" << '\n';
    
  std::cout << std::string(depth + 2, ' ') << "Op: "<< Op << '\n';
    
  if (RHS) 
    RHS->printinfo(depth + 2);
  else 
    std::cout << std::string(depth + 2, ' ') << "Nullptr" << '\n';
}


Value* BinExpNode::codegen() {
  // Special case '=' because we don't want to emit the LHS as an expression.
  if (Op == '=') {
    // This assume we're building without RTTI because LLVM builds that way by
    // default. If you build LLVM with RTTI this can be changed to a
    // dynamic_cast for automatic error checking.
    VarNode* LHSE = static_cast<VarNode*>(LHS.get());
    if (!LHSE)
      return Ast2IRError("destination of '=' must be a variable");
      // Codegen the RHS.
    Value* Val = RHS->codegen();
    if (!Val)
      return nullptr;

    // Look up the name.
    Value* Variable = NamedValues[LHSE->VarName];
    if (!Variable)
      return Ast2IRError("Unknown variable name");

    Builder->CreateStore(Val, Variable);
    return Val;
  }
  Value* L= LHS->codegen();
  Value* R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return Builder->CreateAdd(L, R, "addtmp");
  case '-':
    return Builder->CreateSub(L, R, "subtmp");
  case '*':
    return Builder->CreateMul(L, R, "multmp");
  case '/':
    return Builder->CreateSDiv(L, R, "divtmp");
  default:
    return Ast2IRError("invalid binary operator");
  }
}


CalleeExpNode::CalleeExpNode(const std::string& name,std::vector<std::unique_ptr<Node>> args)
  	: Callee(name), CalleeArgs{std::move(args)} {}
  	
void CalleeExpNode::printinfo(int depth) const{
    std::cout << std::string(depth, ' ') << "CalleeExpNode: " << Callee << '\n';
    std::cout << std::string(depth+2, ' ') << "CalleeArgs: " << '\n';
    for (const auto& arg : CalleeArgs) {
        arg->printinfo(depth + 4);
    }
}


Value* CalleeExpNode::codegen() {
  // Look up the name in the global module table.
  Function* CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return Ast2IRError("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != CalleeArgs.size())
    return Ast2IRError("Incorrect # arguments passed");

  std::vector<Value*> ArgsV;
  for (unsigned i = 0, e = CalleeArgs.size(); i != e; ++i) {
    ArgsV.push_back(CalleeArgs[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}


LetExpNode::LetExpNode(std::unique_ptr<Node> var,std::unique_ptr<Node> exp)
	: LetVar{std::move(var)}, LetBody{std::move(exp)} {}
	
void LetExpNode::printinfo(int depth) const {
    std::cout << std::string(depth, ' ') << "LetExpNode:\n" ;
    if (!LetVar) {
        std::cout << std::string(depth + 2, ' ') << "Nullptr" << '\n';
        return;  
    }
    std::cout << std::string(depth + 2, ' ') << "LetVar: \n";
    
    LetVar->printinfo(depth + 4);
    
    std::cout << std::string(depth + 2, ' ') << "LetBody: \n";
    
    if (!LetBody) {
        std::cout << std::string(depth + 2, ' ') << "Nullptr" << '\n';
        return;  
    }

    LetBody->printinfo(depth + 4);
}


Value* LetExpNode::codegen() {
    // 生成变量初始化的代码
    Value* VarValue = LetVar->codegen();
    if (!VarValue)
      return nullptr;
    
    std::unique_ptr<VarNode> newVarNodePtr = std::unique_ptr<VarNode>(static_cast<VarNode*>(LetVar.release()));
    
    // 假设 LetVar 是一个变量节点，并且它是整型（i32）
    AllocaInst* Alloca = Builder->CreateAlloca(Type::getInt32Ty(*TheContext), nullptr, newVarNodePtr->VarName.c_str());
    
    Value* BodyValue = LetBody->codegen();
    if (!BodyValue)
      return nullptr;
    
    // 存储初始化值到变量中
    Builder->CreateStore(BodyValue, Alloca);
    

    // 返回分配的地址作为变量值
    return Alloca;
}



  
FunDefNode::FunDefNode(const std::string& name,std::vector<std::string> args,std::unique_ptr<Node> body)
	: FunDefName(name), FunDefArgs{std::move(args)}, FunDefBody{std::move(body)} {}
  	
void FunDefNode::printinfo(int depth) const {
    std::cout << std::string(depth, ' ') << "FunctionNode: " << FunDefName << '\n';
    std::cout << std::string(depth + 2, ' ') << "Params: ";
    if (FunDefArgs.size() == 0)
        std::cout << "None";
    else {
        for (const auto& arg : FunDefArgs) {
            std::cout << arg << " ";
        }
    }
    std::cout << '\n';

    if (!FunDefBody) {
        std::cout << std::string(depth + 2, ' ') << "Nullptr" << '\n';
        return;  
    }

    FunDefBody->printinfo(depth + 2);
}


Function* FunDefNode::codegen() {
  // Make the function type:  int(int, int) etc.
  std::vector<Type*> Ints(FunDefArgs.size(), Type::getInt32Ty(*TheContext));
  FunctionType *FT = FunctionType::get(Type::getInt32Ty(*TheContext), Ints, false);

  Function* F =
    Function::Create(FT, Function::ExternalLinkage, FunDefName, TheModule.get());
  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(FunDefArgs[Idx++]);

  // Create a new basic block to start insertion into.
  BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", F);
  Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto& Arg : F->args()){
    // Create an alloca for this variable.
    AllocaInst *Alloca = CreateEntryBlockAlloca(F, Arg.getName().str());

    // Store the initial value into the alloca.
    Builder->CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[std::string(Arg.getName())] = Alloca;
  }
    
  if (Value* RetVal = FunDefBody->codegen()) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*F);

    return F;
  }
  
  // Error reading body, remove function.
  F->eraseFromParent();
  return nullptr;
}


IfExpNode::IfExpNode(std::unique_ptr<Node> cond,std::unique_ptr<Node> then,std::unique_ptr<Node> els) 
	: Cond{std::move(cond)},Then{std::move(then)},Else{std::move(els)} {}
  
void IfExpNode::printinfo(int depth) const {
  std::cout << std::string(depth, ' ') << "IfExpNode: " << '\n';
  std::cout << std::string(depth+2, ' ') << "Condition: " << '\n';
  Cond -> printinfo(depth+4);
  
  std::cout << std::string(depth+2, ' ') << "Then: " << '\n';
  Then -> printinfo(depth+4);

  std::cout << std::string(depth+2, ' ') << "Else: " << '\n';
  Else -> printinfo(depth+4);
}

Value* IfExpNode::codegen() {
  Value* CondV = Cond->codegen();
  if (!CondV)
    return Ast2IRError("condition codegen failed");
    
  CondV = Builder->CreateICmpNE(CondV, ConstantInt::get(*TheContext, APInt(32, 0)), "ifcond");
  Function *TheFunction = Builder->GetInsertBlock()->getParent();
  
  BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

  Builder->CreateCondBr(CondV, ThenBB, ElseBB);
  
  // Emit then block.
  Builder->SetInsertPoint(ThenBB);
  Value *ThenV = Then->codegen();
  if (!ThenV)
    return Ast2IRError("then branch codegen failed");
  Builder->CreateBr(MergeBB);
  
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder->GetInsertBlock();
  
  // Emit else block.
  //TheFunction->insert(TheFunction->end(), ElseBB);
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder->SetInsertPoint(ElseBB);

  Value *ElseV = Else->codegen();
  if (!ElseV)
    return Ast2IRError("else branch codegen failed");
  Builder->CreateBr(MergeBB);
  
  // codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = Builder->GetInsertBlock();
  
  // Emit merge block.
  //TheFunction->insert(TheFunction->end(), MergeBB);
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);
  PHINode *PN = Builder->CreatePHI(Type::getInt32Ty(*TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}



std::unique_ptr<ProgNode> InitAst() {
    return std::make_unique<ProgNode>(std::vector<std::unique_ptr<Node>>{});
}


void addNode(std::unique_ptr<ProgNode>& root, std::unique_ptr<Node> node) {
    if (!root) {
        std::cout << "root not ok" << '\n';
        root = InitAst();
    }
    root->defs.push_back(std::move(node));
    std::cout << "root ok" << '\n';
}

Value* Ast2IRError(const std::string& message){
    std::cout << "Ast2IRError: "<< message << '\n';
    return nullptr;
}

