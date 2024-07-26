#include "parser.h"


int main() {
        
	Parser parser("../example.txt");
	parser.ParseProgram();
	auto root = parser.getRoot();
	InitializeModule();

	//std::cout << root->defs.size()<<'\n';
	parser.PrintAst(root);
	parser.PrintIR(root);
	TheModule->print(errs(), nullptr);
        return 0;
}
