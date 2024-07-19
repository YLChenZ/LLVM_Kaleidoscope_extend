#include "parser.h"


int main() {
        
	Parser parser("example.txt");
	parser.ParseProgram();
	auto root = parser.getRoot();
	parser.PrintAst(std::move(root));
        return 0;
}
