// Main driver code

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage %s filename\n", argv[0]);
		exit(-1);
	}

	// redirection from argv[1]
	freopen(argv[1], "r", stdin);
	
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

	// Install stanard binary operators.
	// 1 is lowest precedence.
	BinopPrecedence['='] = 2;
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.

	// Prime the first token.
	getNextToken();

	TheJIT = llvm::make_unique<KaleidoscopeJIT>();


	InitializeModule();

	// Add the current debug info version into the module.
	TheModule->addModuleFlag(Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);
	
	// Construct the DIBuilder, we do this here because we need the module.
	DBuilder = llvm::make_unique<DIBuilder>(*TheModule);

	// Create the compile unit for the module.
	KSDbgInfo.TheCU = DBuilder->createCompileUnit(dwarf::DW_LANG_C, DBuilder->createFile(argv[1], "."),
					"Kaleidoscope Compiler", 0, "", 0);

	// Run the main "interpreter loop" now.
	MainLoop();

	// Finalize the debug info.
	DBuilder->finalize();

	// Print out all of the generated code.
	TheModule->print(errs(), nullptr);

	return 0;
}
