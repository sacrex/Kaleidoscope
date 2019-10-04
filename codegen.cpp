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

// IfExprAST codegen
Value *IfExprAST::codegen()
{
	Value *CondV = Cond->codegen();
	if (!CondV) {
		return nullptr;
	}
	
	// Convert condition to a bool by comparing non-equal to 0.0
	CondV = Builder.CreateFCmpONE(
			CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
	Function *TheFunction = Builder.GetInsertBlock()->getParent();
	
	// Create blocks for the then and else cases. 
	// Insert the 'then' block at the end of the function.
	BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
	BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
	BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

	Builder.CreateCondBr(CondV, ThenBB, ElseBB);
	
	// Emit then value
	Builder.SetInsertPoint(ThenBB);

	Value *ThenV = Then->codegen();
	if (!ThenV) {
		return nullptr;
	}

	Builder.CreateBr(MergeBB);
	
	// Codegen of 'Then' can change the current block, update
	// ThenBB for the PHI.
	ThenBB = Builder.GetInsertBlock();
	
	// Emit else block.	
	TheFunction->getBasicBlockList().push_back(ElseBB);
	Builder.SetInsertPoint(ElseBB);

	Value *ElseV = Else->codegen();
	if (!ElseV) {
		return nullptr;
	}
	Builder.CreateBr(MergeBB);
	// Codegen of 'Else' can change the current block,
	// update ElseBB for the PHI.
	ElseBB = Builder.GetInsertBlock();
	
	// Emit merge block.
	TheFunction->getBasicBlockList().push_back(MergeBB);
	Builder.SetInsertPoint(MergeBB);
	PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");

	PN->addIncoming(ThenV, ThenBB);
	PN->addIncoming(ElseV, ElseBB);

	return PN;
}

// 原版的for语义
// Output for-loop as:
//	loopheader: 
// 		...
// 		start = startexpr
// 		goto loop
//	loop:
//		variable = phi [start, loopheader], [nextvariable, loopend]
//		...
//		bodyexpr
//		...
//	loopend:
//		step = stepexpr
//		nextvariable = variable + step
//		endcond = endexpr
//		br endcond, loop, outloop
//	outloop:
//
/*
Value *ForExprAST::codegen()
{
	// Emit the start code first, without 'variable' in scope
	Value *StartVal = Start->codegen();
	if (!StartVal) {
		return nullptr;
	}
	
	// Make the new basic block for the loop header,
	// inserting after current block.
	Function *TheFunction = Builder.GetInsertBlock()->getParent();
	BasicBlock *PreheaderBB = Builder.GetInsertBlock();
	BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);

	// Insert an explicit fall through from the current block to the LoopBB.
	Builder.CreateBr(LoopBB);

	// Start insertion in LoopBB.
	Builder.SetInsertPoint(LoopBB);

	// Start the PHI node with an entry for Start.
	PHINode *Variable = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, 
						VarName);
	Variable->addIncoming(StartVal, PreheaderBB);
	
	// Within the loop, the variable is defined equal to the PHI node.
	// If it shadows an existing variable, we have to restore it, so save it now.
	Value *OldVal = NamedValues[VarName];
	NamedValues[VarName] = Variable;
	
	// Emit the body of the loop. This, like any other expr, can change the
	// current BB. Note that we ignore the value computed by the body, but
	// don't allow an error.
	if (!Body->codegen()) {
		return nullptr;
	}
	
	// Emit the step value.
	Value *StepVal = nullptr;
	if (Step) {
		StepVal = Step->codegen();
		if (!StepVal) {
			return nullptr;
		}
	} else {
		// If not specified, use 1.0
		StepVal = ConstantFP::get(TheContext, APFloat(1.0));
	}
	
	Value *NextVar = Builder.CreateFAdd(Variable, StepVal, "nextvar");
	
	// Compute the end condition.
	Value *EndCond = End->codegen();
	if (!EndCond) {
		return nullptr;
	}
	
	// Convert condition to a bool by comparing non-equal to 0.0.	
	EndCond = Builder.CreateFCmpONE(
					EndCond, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

	// Create the "after loop" block and insert it.
	BasicBlock *LoopEndBB = Builder.GetInsertBlock();
	BasicBlock *AfterBB = BasicBlock::Create(TheContext, "afterloop", TheFunction);
	
	// Insert the conditional branch into the end of LoopEndBB.
	// TODO LoopBB or LoopEndBB ??
	Builder.CreateCondBr(EndCond, LoopBB, AfterBB);
	
	// Any new code will be inserted in AfterBB.
	Builder.SetInsertPoint(AfterBB);

	// Add a new entry to the PHI node for the backedge.
	Variable->addIncoming(NextVar, LoopEndBB);

	// Restore the unshadowed variable.	
	if (OldVal) {
		NamedValues[VarName] = OldVal;
	} else {
		NamedValues.erase(VarName);
	}
	
	// for expr always returns 0.0.	
	return ConstantFP::getNullValue(Type::getDoubleTy(TheContext));
}
*/

// 修改后的for表达式的语义(和Ｃ语言一样)
// Output for-loop as:
// 	entry:
// 		start = startexpr
// 		goto loopend
//	loopend:
//		variable = phi [start, entry], [nextvariable, loop]
//		...
//		endcond = endexpr
//		...
//		br endcond, loop, afterloop
//	loop:
//		...
//		body = bodyexpr
//		step = stepexpr
//		nextvariable = variable + step
//		br loopend
//	afterloop:
//
Value *ForExprAST::codegen()
{
	// Emit the start code first, without 'variable' in scope
	Value *StartVal = Start->codegen();
	if (!StartVal) {
		return nullptr;
	}

	// Make the new basic block for the loop end,
	// inserting after current block.
	Function *TheFunction = Builder.GetInsertBlock()->getParent();
	BasicBlock *EntryBB = Builder.GetInsertBlock();
	BasicBlock *LoopEndBB = BasicBlock::Create(TheContext, "loopend", TheFunction);

	// Insert an explicit fall through from the current block to the LoopEndBB.
	Builder.CreateBr(LoopEndBB);

	// Start insertion in LoopEndBB.
	Builder.SetInsertPoint(LoopEndBB);

	// Start the PHI node with an entry for Start.
	PHINode *Variable = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, 
						VarName);
	Variable->addIncoming(StartVal, EntryBB);
	
	// Within the loop, the variable is defined equal to the PHI node.
	// If it shadows an existing variable, we have to restore it, so save it now.
	Value *OldVal = NamedValues[VarName];
	NamedValues[VarName] = Variable;

	// Compute the end condition.
	Value *EndCond = End->codegen();
	if (!EndCond) {
		return nullptr;
	}
	
	// Convert condition to a bool by comparing non-equal to 0.0.	
	EndCond = Builder.CreateFCmpONE(EndCond,
					ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");
	
	// Create the 'loop' block
	BasicBlock *LoopBB = BasicBlock::Create(TheContext, 
					"loop");

	// Create the 'afterloop' block	
	BasicBlock *AfterBB = BasicBlock::Create(TheContext,
					"afterloop");
	
	// Insert the conditional branch into the end of LoopEndBB.	
	Builder.CreateCondBr(EndCond, LoopBB, AfterBB);
	
	// add LoopBB to Function and set insert point at LoopBB.
	TheFunction->getBasicBlockList().push_back(LoopBB);
	Builder.SetInsertPoint(LoopBB);

	// Emit the body of the loop. This, like any other expr, can change the
	// current BB. Note that we ignore the value computed by the body, but
	// don't allow an error.
	if (!Body->codegen()) {
		return nullptr;
	}
	
	// Emit the step value.
	Value *StepVal = nullptr;
	if (Step) {
		StepVal = Step->codegen();
		if (!StepVal) {
			return nullptr;
		}
	} else {
		// If not specified, use 1.0
		StepVal = ConstantFP::get(TheContext, APFloat(1.0));
	}
	
	Value *NextVar = Builder.CreateFAdd(Variable, StepVal, "nextvar");

	Builder.CreateBr(LoopEndBB);
	
	// 重新赋值LoopBB，获取更新后的值(e.g. 在二重循环中, 更新后LoopBB
	// 会指向内循环中的AfterBB这个基本快(因为上述的Body->codegen()返回之后，设
	// 置了AfterBB为插入点, 导致我们需要更新LoopBB指针指向AfterBB(内循环), 
	// 此时LoopBB指向的快中就有NextVar这个指令了))
	//
	// 二重循环例子
	//	def foo(x y) for i = 1, i < x in for i = 1, i < y in putchard(97);
	LoopBB = Builder.GetInsertBlock();

	Variable->addIncoming(NextVar, LoopBB);

	// Any new code will be inserted in AfterBB.
	TheFunction->getBasicBlockList().push_back(AfterBB);
	Builder.SetInsertPoint(AfterBB);

	// Restore the unshadowed variable.	
	if (OldVal) {
		NamedValues[VarName] = OldVal;
	} else {
		NamedValues.erase(VarName);
	}
	
	// for expr always returns 0.0.	
	return ConstantFP::getNullValue(Type::getDoubleTy(TheContext));
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
