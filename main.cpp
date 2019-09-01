//
// Main Driver code
//
int main() 
{
	// Install standard binary operators.
	// 1 is lowest precedence.
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; //highest.

	fprintf(stderr, "ready> ");

	getNextToken(); //get the first token

	// Make the module, which holds all the code.
	TheModule = std::make_unique<Module>("my cool jit", 
							getGlobalContext());

	// Run the main "interpreter loop" now.
	MainLoop();
	
	// Print out all of the generated code.
	TheModule->dump();

	return 0;
}
