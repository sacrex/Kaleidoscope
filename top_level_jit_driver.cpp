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
	
	// Promote allocas to registers.
	TheFPM->add(createPromoteMemoryToRegisterPass());

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

			// 查找是否已经有该名字对应的函数，若存在，则从JIT中删除它
			if (TheJIT->findSymbol(FnIR->getName())) {
				if (FuncModuleMap.find(FnIR->getName()) != FuncModuleMap.end()) {
			  		TheJIT->removeModule(FuncModuleMap[FnIR->getName()]);
				}
			}
			// 由于下面的std::move(TheModule),会导致FnIR指向的内容变为空了
			// 故这里需要保存一下函数名
			std::string FuncName = FnIR->getName();

			auto N = TheJIT->addModule(std::move(TheModule));
			FuncModuleMap.insert(std::make_pair(FuncName, N));

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
		if (FnAST->codegen()) {

			// JIT the module containing the anonymous expression, keeping
			// a handle so we can free it later.
			auto H = TheJIT->addModule(std::move(TheModule));
			InitializeModuleAndPassManager();

			// Search the JIT for the __anno_expr symbol.
			auto ExprSymbol = TheJIT->findSymbol("__anno_expr");
			assert(ExprSymbol && "Function not found");


			// Get the symbol's address and cast it to the right type (takes no
			// arguments, return a double) so we can call it as a native function.

			//ExprSymol.getAddress()这个对于用户定义的函数(例如putchard, printd)会出现错误，
			//这个问题需要修改KaleidoscopeJIT.h中的符号查找方法，详细见
			//https://stackoverflow.com/questions/54048268/llvm-kaleidoscope-tutorial-failing-on-local-extern/56862433#56862433
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
