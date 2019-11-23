//
// Top-Level parsing and JIT Driver
//


// Chapter8 use this function(diff from previous)
static void InitializeModuleAndPassManager()
{
	// Open a new module
	TheModule = llvm::make_unique<Module>("my cool jit", TheContext);
}

static void HandleDefinition()
{
	if (auto FnAST = ParseDefinition()) {
		if (auto *FnIR = FnAST->codegen()) {
			fprintf(stderr, "Read function definition:");
			FnIR->print(errs());
			fprintf(stderr, "\n");
		}
	} else {
		// Skip token for error recovery
		getNextToken();
	}
}

static void HandleExtern()
{
	if (auto ProtoAST = ParseExtern()) {
		if (auto *FnIR = ProtoAST->codegen()) {
			fprintf(stderr, "Read extern: ");
			FnIR->print(errs());
			fprintf(stderr, "\n");
			FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
		}
	} else {
		// Skip token for error recovery
		getNextToken();
	}
}

static void HandleTopLevelExpression()
{
	// Evaluate a top-level expression into an anonymous function.
	if (auto FnAST = ParseTopLevelExpr()) {
		FnAST->codegen();
	} else {
		// Skip token for error recovery
		getNextToken();
	}
}

// top ::= definition | external | expression | ';'
static void MainLoop()
{
	while (true) {
		switch (CurTok) {
			case tok_eof:
				return;
			case ';': // ignore top-level semicolons.
				getNextToken();
				break;
			case tok_def:
				HandleDefinition();
				break;
			case tok_extern:
				HandleExtern();
				break;
			default:
				HandleTopLevelExpression();
				break;
		}
		fprintf(stderr, "ready> ");
	}
}
