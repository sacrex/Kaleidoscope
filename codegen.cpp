//
// Code Generation
//

static std::unique_ptr<Module> TheModule;
static IRBuilder<> Builder(getGlobalContext());
static std::map<std::string, Value *> NamedValues;

Value *ErrorV(const char *Str)
{
	LogError(Str);
	return nullptr;
}

Value *NumberExprAST::codegen()
{
	return ConstantFP::get(getGlobalContext(), APFloat(Val));
}

Value *VariableExprAST::codegen()
{
	//当前的变量只有函数参数
	//Look this variable up in the function.
	Value *V = NamedValues[Name];
	if (!V) {
		return ErrorV("UnKnown variable name");
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
			return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()), 
									"booltmp");
		default:
			return ErrorV("invalid binary operator");
	}
}

Value *CallExprAST::codegen()
{
	// Look up the name in the global module table.
	Function *CalleeF = TheModule->getFunction(Callee);
	if (!CalleeF) {
		return ErrorV("Unknown function referenced");
	}

	// If argument mismatch error.
	if (CalleeF->arg_size() != Args.size()) {
		return ErrorV("Incorrect # arguments passed");
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
						Type::getDoubleTy(getGlobalContext()));
	
	FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobalContext()), 
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
	Function *TheFunction = TheModule->getFunction(Proto->getName());

	if (!TheFunction) {
		TheFunction = Proto->codegen();
	}

	if (!TheFunction) {
		return nullptr;
	}
	
	// Create a new basic block to start insertion into
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", 
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
		
		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();

	return nullptr;
}
