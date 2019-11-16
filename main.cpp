//
// Main Driver code
//
int main() 
{
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
		
	// Install standard binary operators.
	// 1 is lowest precedence.
	BinopPrecedence['='] = 2;
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; //highest.

	fprintf(stderr, "ready> ");

	getNextToken(); //get the first token

	TheJIT = llvm::make_unique<KaleidoscopeJIT>();

	InitializeModuleAndPassManager();

	// Run the main "interpreter loop" now.
	MainLoop();
	
	return 0;
}
