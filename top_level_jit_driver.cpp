//
// Top-Level parsing and JIT Driver
//

static void InitializeModuleAndPassManager()
{
	// open a new module.
	TheModule = llvm::make_unique<Module>("my cool jit", TheContext);
	TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());
	
	// Create a new pass manager attached to it.
	TheFPM = llvm::make_unique<legacy::FunctionPassManager>(TheModule.get());

	// Do simple "peephole" optimizations and bit-twiddling optzns.
	TheFPM->add(createInstructionCombiningPass());

	// Reassociate expressions.
	TheFPM->add(createReassociatePass());

	// Eliminate Common SubExpressions.
	TheFPM->add(createGVNPass());

	// Simplify the control flow graph (deleting unreachable blocks, etc).
	TheFPM->add(createCFGSimplificationPass());

	TheFPM->doInitialization();
}

static void HandleDefinition()
{
	if (auto FnAST = ParseDefinition()) {
		if (auto *FnIR = FnAST->codegen()) {
			fprintf(stderr, "Read function definition:");
			FnIR->print(errs());
			fprintf(stderr, "\n");
			TheJIT->addModule(std::move(TheModule));
			InitializeModuleAndPassManager();
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
		if (auto *FnIR = FnAST->codegen()) {
			// JIT the module containing the anonymous expression, keeping
			// a handle so we can free it later.
			auto H = TheJIT->addModule(std::move(TheModule));
			InitializeModuleAndPassManager();

			// Search the JIT for the __anon_expr symbol.
			auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
			assert(ExprSymbol && "Function not found");

			// Get th symbol's address and cast it to the right type (takes no
			// arguments, return a double) so we can call it as a native function.
			double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());

			fprintf(stderr, "Evaluated to %f\n", FP());
			
			// Delete the anonymous expression module from the JIT.
			TheJIT->removeModule(H);
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
		fprintf(stderr, "ready> ");
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
