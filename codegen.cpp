//
// Code Generation
//
static LLVMContext TheContext;
static std::unique_ptr<Module> TheModule;
static IRBuilder<> Builder(TheContext);
static std::map<std::string, Value *> NamedValues;
static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
static std::unique_ptr<KaleidoscopeJIT> TheJIT;
static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

//符号和符号所在的模块映射(解决了　后定义的重名函数能被调用，而先前的已定义的同名函数就会被覆盖(删除))
//见HandleDefinition的实现
static std::map<std::string, VModuleKey> FuncModuleMap;

Value *LogErrorV(const char *Str)
{
	LogError(Str);
	return nullptr;
}

Function *getFunction(std::string Name)
{
	// First, see if the function has already been added to the current
	// module.
	if (auto *F = TheModule->getFunction(Name)) {
		return F;
	}

	// If not, check whether we can codegen the declaration 
	// from some existing prototype.
	auto FI = FunctionProtos.find(Name);
	if (FI != FunctionProtos.end()) {
		return FI->second->codegen();
	}

	// If no existing prototype exists, return null.
	return nullptr;
}

Value *NumberExprAST::codegen()
{
	return ConstantFP::get(TheContext, APFloat(Val));
}

Value *VariableExprAST::codegen()
{
	//当前的变量只有函数参数
	//Look this variable up in the function.
	Value *V = NamedValues[Name];
	if (!V) {
		return LogErrorV("UnKnown variable name");
	}
	return V;
}

Value *BinaryExprAST::codegen()
{
	Value *L = LHS->codegen();
	Value *R = RHS->codegen();

	if (!L || !R) {
		return nullptr;
	}

	switch (Op) {
		case '+':
			return Builder.CreateFAdd(L, R, "addtmp");
		case '-':
			return Builder.CreateFSub(L, R, "subtmp");
		case '*':
			return Builder.CreateFMul(L, R, "multmp");
		case '<':
			L = Builder.CreateFCmpULT(L, R, "cmptmp");
			
			//Convert bool 0/1 to double 0.0/1.0
			return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), 
									"booltmp");
		default:
			return LogErrorV("invalid binary operator");
	}
}

Value *CallExprAST::codegen()
{
	// Look up the name in the global module table.
	Function *CalleeF = getFunction(Callee);
	if (!CalleeF) {
		return LogErrorV("Unknown function referenced");
	}

	// If argument mismatch error.
	if (CalleeF->arg_size() != Args.size()) {
		return LogErrorV("Incorrect # arguments passed");
	}

	std::vector<Value *> ArgsV;
	for (unsigned i = 0, e = Args.size(); i != e; ++i) {
		ArgsV.push_back(Args[i]->codegen());
		if (!ArgsV.back()) {
			return nullptr;
		}
	}

	return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}


Function *PrototypeAST::codegen()
{
	// Make the function type: double(double, double) etc.
	std::vector<Type *> Doubles(Args.size(), 
						Type::getDoubleTy(TheContext));
	
	FunctionType *FT = FunctionType::get(Type::getDoubleTy(TheContext), 
								Doubles, false);
	Function *F = Function::Create(FT, Function::ExternalLinkage, 
					Name, TheModule.get());
	
	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto &Arg : F->args()) {
		Arg.setName(Args[Idx++]);
	}
	
	return F;
}


Function *FunctionAST::codegen()
{
	auto &P = *Proto;
	FunctionProtos[Proto->getName()] = std::move(Proto);
	Function *TheFunction = getFunction(P.getName());

	if (!TheFunction) {
		return nullptr;
	}
	
	// Create a new basic block to start insertion into
	BasicBlock *BB = BasicBlock::Create(TheContext, "entry", 
									TheFunction);
	Builder.SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	// 这里做清除操作，因为现在只存函数参数
	NamedValues.clear();
	for (auto &Arg : TheFunction->args())
		NamedValues[Arg.getName()] = &Arg;

	if (Value *RetVal = Body->codegen()) {
		Builder.CreateRet(RetVal);

		// Validate the generated code, checking for consistency.
		verifyFunction(*TheFunction);
		
		TheFPM->run(*TheFunction);

		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();

	return nullptr;
}
