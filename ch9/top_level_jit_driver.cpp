//
// Top-Level parsing and JIT Driver
//


static void InitializeModule()
{
	TheModule = llvm::make_unique<Module>("my cool jit", TheContext);
	TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());
}

static void HandleDefinition()
{
	if (auto FnAST = ParseDefinition()) {
		if (!FnAST->codegen()) {
			fprintf(stderr, "Error reading function definition:");
		}
	} else {
		// Skip token for error recovery
		getNextToken();
	}
}

static void HandleExtern()
{
	if (auto ProtoAST = ParseExtern()) {
		if (!ProtoAST->codegen()) {
			fprintf(stderr, "Error reading extern");
		} else {
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
		if(!FnAST->codegen()) {
			fprintf(stderr, "Error generating code for top level expr");
		}
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
	}
}
