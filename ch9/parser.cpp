//
// Parser
//

// CurTok/getNextToken - Provide a simple token buffer. CurTok is the
// current token the parser is looking at. getNextToken reads another
// token from the lexer and updates CurTok with its results;
static int CurTok;
static int getNextToken()
{
	return CurTok = gettok();
}

// BinopPrecedence - This holds the precedence for each binary 
// operator that is defined.
static std::map<char, int> BinopPrecedence;

// GetTokPrecedence - Get the precedence of the pending binary
// operator token.
static int GetTokPrecedence()
{
	if (!isascii(CurTok)) {
		return -1;
	}
	
	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[CurTok];
	if (TokPrec <= 0) {
		return -1;
	}
	
	return TokPrec;
}

// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str)
{
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
	LogError(Str);
	return nullptr;
}

//forward declaration
static std::unique_ptr<ExprAST> ParseExpression();

// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr()
{
	auto Result = llvm::make_unique<NumberExprAST>(Number);
	getNextToken(); //eat the number, update CurTok
	return std::move(Result);
}

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr()
{
	getNextToken(); // eat (
	auto V = ParseExpression();
	if (!V) {
		return nullptr;
	}

	if (CurTok != ')') {
		fprintf(stderr, "curTok : '(%d)-(%c)'\t", CurTok, CurTok);
		return LogError("expected ')'");
	}
	getNextToken(); // eat )
	return V;
}

// identifierexpr 
// 			::= identifer                          --- (1)
// 			::= identifer '(' expression* ')'	   --- (2)
static std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
	std::string IdName = IdentifierStr;

	SourceLocation LitLoc = CurLoc;

	getNextToken(); // eat identifier

	if (CurTok != '(') {  // (1) variable expr
		return llvm::make_unique<VariableExprAST>(LitLoc, IdName);
	}

	//(2) call expr 
	getNextToken(); // eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (true) {
			if (auto Arg = ParseExpression()) {
				Args.push_back(std::move(Arg));
			} else {
				return nullptr;
			}

			if (CurTok == ')') {
				break;
			}

			if (CurTok != ',') {
				return LogError("Expected ')' or ',' in argument list");
			}
			getNextToken(); // eat ','
		}
	}
	
	getNextToken(); // eat ). update CurTok to next token
	
	return llvm::make_unique<CallExprAST>(LitLoc, IdName, std::move(Args));
}

// ifexpr ::= 'if' expression 'then' expression 'else' expression
static std::unique_ptr<ExprAST> ParseIfExpr()
{
	SourceLocation  IfLoc = CurLoc;

	getNextToken(); //eat the if.
	
	// condition.
	auto Cond = ParseExpression();
	if (!Cond) {
		return nullptr;
	}

	if (CurTok != tok_then) {
		return LogError("expected then");
	}
	getNextToken(); // eat the then
	
	auto Then = ParseExpression();
	if (!Then) {
		return nullptr;
	}

	if (CurTok != tok_else) {
		return LogError("expected else");
	}
	getNextToken(); // eat the else

	auto Else = ParseExpression();
	if (!Else) {
		return nullptr;
	}
	
	return llvm::make_unique<IfExprAST>(IfLoc, std::move(Cond), std::move(Then),
									std::move(Else));
}

// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expr
static std::unique_ptr<ExprAST> ParseForExpr()
{
	getNextToken(); //eat the for.

	if (CurTok != tok_identifier) {
		return LogError("expected identifier after for");
	}

	std::string IdName = IdentifierStr;
	getNextToken(); //eat identifier.

	if (CurTok != '=') {
		return LogError("expected '=' after for");
	}
	getNextToken(); // eat '='.

	auto Start = ParseExpression();
	if (!Start) {
		return nullptr;
	}

	if (CurTok != ',') {
		return LogError("expected ',' after for start value");
	}
	getNextToken(); // eat ','
	
	auto End = ParseExpression();
	if (!End) {
		return nullptr;
	}
	
	std::unique_ptr<ExprAST> Step;
	if (CurTok == ',') {
		getNextToken(); //eat ','
		Step = ParseExpression();
		if (!Step) {
			return nullptr;
		}
	}

	if (CurTok != tok_in) {
		return LogError("expected 'in' after for");
	}
	getNextToken(); // eat 'in'.

	auto Body = ParseExpression();
	if (!Body) {
		return nullptr;
	}

	return llvm::make_unique<ForExprAST>(IdName, std::move(Start),
					std::move(End), std::move(Step), std::move(Body));
}

// varexpr ::= 'var' identifier ('=' expression)? 
// 				(',' identifier ('=' expression)?)* 'in' expression
static std::unique_ptr<ExprAST> ParseVarExpr() {
	getNextToken(); // eat the var.
	
	std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;

	// At least one variable name is required.
	if (CurTok != tok_identifier) {
		return LogError("expected identifier after var");
	}
	
	while (true) {
		std::string Name = IdentifierStr;
		getNextToken(); // eat identifier.
	
		// Read the optional initializer.
		std::unique_ptr<ExprAST> Init = nullptr;
		if (CurTok == '=') {
			getNextToken(); // eat the '='.
			
			Init = ParseExpression();
			if (!Init) {
				return nullptr;
			}
		}
		
		VarNames.push_back(std::make_pair(Name, std::move(Init)));

		// End of var list, exit loop.
		if (CurTok != ',') {
			break;
		}
		getNextToken(); // eat the ','

		if (CurTok != tok_identifier) {
			return LogError("expected identifier list after var");
		}
	}
	
	// At this point, we have to have 'in'.
	if (CurTok != tok_in) {
		return LogError("expect 'in' keyword after 'var'");
	}
	getNextToken(); // eat 'in'.

	auto Body = ParseExpression();
	if (!Body) {
		return nullptr;
	}
	
	return llvm::make_unique<VarExprAST>(std::move(VarNames), 
										std::move(Body));
}


// primary
// 		::= identifierexpr
// 		::= numberexpr
// 		::= parenexpr
// 		::= ifexpr
// 		::= forexpr
// 		::= varexpr
static std::unique_ptr<ExprAST> ParsePrimary()
{
	switch (CurTok) {
		default:
			fprintf(stderr, "curTok : '(%d)-(%c)'\t", CurTok, CurTok);
			return LogError("unknown token when expecting an expression");
		case tok_identifier:
			return ParseIdentifierExpr();
		case tok_number:
			return ParseNumberExpr();
		case '(':
			return ParseParenExpr();
		case tok_if:
			return ParseIfExpr();
		case tok_for:
			return ParseForExpr();
		case tok_var:
			return ParseVarExpr();
	}
}

// unary ::= primary
// 		 ::= '!' unary 
static std::unique_ptr<ExprAST> ParseUnary()
{
	// If the current token is not an operator, it must be a primary expr.
	if (!isascii(CurTok) || CurTok == '(' || CurTok == ',') {
		return ParsePrimary();
	}
	
	// If this is a unary operator, read it.	
	int Opc = CurTok;
	getNextToken();
	if (auto Operand = ParseUnary()) {
		return llvm::make_unique<UnaryExprAST>(Opc, std::move(Operand));
	}
	return nullptr;
}


// binoprhs
// 		::= ('+' unary)*
// ExprPrec 可以看作一个PrePrec.
// e.g   0     a  +  b  +  c
// 	 	 |	      |      |
// 		 |	      |      |
// 		PrePrec TokPrec NextPrec
// 		PrePrec 这个指向的token并不存在，我们只是假想了一个优先级为0
// 		的二元运算符token(binop)
//
// !!！这里的TokPrec(一个binop)不仅要和其左边的binop比较优先级(在这里就是
// 	TokPrec和ExprPrec比较), 而且还要和右边的binop比较优先级(在这里是
// 	TokPrec和NextPrec比较),从而确定是TokPrec还是NextPrec(若是NextPrec,
// 	那么再次递归)
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
											   std::unique_ptr<ExprAST> LHS)
{
	// If this is a binop, find its precedence.
	while (true) {
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least at tightly as the current
		// binop(e.g. TokPrec >= ExprPrec), consume it, otherwise we are done.
		if (TokPrec < ExprPrec) {
			return LHS;
		}

		// Okay, we know this its a binop.
		int BinOp = CurTok;
		SourceLocation BinLoc = CurLoc;
		getNextToken(); // eat binop

		// Parse the unary expression after the binary operator.
		auto RHS = ParseUnary();
		if (!RHS) {
			return nullptr;
		}

		// If BinOp binds less tightly with RHS than the operator after
		// RHS, let the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
			if (!RHS) {
				return nullptr;
			}
		}

		// Merge LHS/RHS.
		LHS = llvm::make_unique<BinaryExprAST>(BinLoc, BinOp, std::move(LHS),
												std::move(RHS));
	}
}


// expression
// 		::= unary binoprhs
static std::unique_ptr<ExprAST> ParseExpression()
{
	auto LHS = ParseUnary();
	if (!LHS) {
		return nullptr;
	}
	
	return ParseBinOpRHS(0, std::move(LHS));
}

// prototype
// 		::= id '(' id* ')'
// 		::= binary LETTER number? (id, id)
// 		::= unary LETTER (id)
// there are not comma in prototype argument list
static std::unique_ptr<PrototypeAST> ParsePrototype()
{
	std::string FnName;

	SourceLocation FnLoc = CurLoc;

	unsigned Kind = 0; // 0 = identifier, 1 = unary, 2 = binary.
	unsigned BinaryPrecedence = 30;
	
	switch (CurTok) {
		default:
			return LogErrorP("Expected function name in prototype");
		case tok_identifier:
			FnName = IdentifierStr;
			Kind = 0;
			getNextToken();
			break;
		case tok_unary:
			getNextToken();
			if (!isascii(CurTok)) {
				return LogErrorP("Expected unary operator");
			}
			FnName = "unary";
			FnName += (char)CurTok;
			Kind = 1;
			getNextToken();
			break;
		case tok_binary:
			getNextToken();
			if (!isascii(CurTok)) {
				return LogErrorP("Expected binary operator");
			}
			FnName = "binary";
			FnName += (char)CurTok;
			Kind = 2;
			getNextToken();

			if (CurTok == tok_number) {
				if (Number < 1 || Number > 100) {
					return LogErrorP("Invalid precedence: must be 1..100");
				}
				BinaryPrecedence = (unsigned)Number;
				getNextToken();
			}
			break;
	}

	if (CurTok != '(') {
		return LogErrorP("Expected '(' in prototype");
	}

	std::vector<std::string> ArgNames;
	while(getNextToken() == tok_identifier) {
		ArgNames.push_back(IdentifierStr);
	}

	if (CurTok != ')') {
		return LogErrorP("Expected ')' in prototype");
	}
	
	// success.	
	getNextToken(); // eat ).
	
	// Verify right number of names for operator.
	if (Kind && ArgNames.size() != Kind) {
		return LogErrorP("Invalid number of operands for operator");
	}

	return llvm::make_unique<PrototypeAST>(FnLoc, FnName, std::move(ArgNames), 
					Kind != 0, BinaryPrecedence);
}

// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition()
{
	getNextToken(); //eat def

	auto Proto = ParsePrototype();
	if (!Proto) {
		return nullptr;
	}

	if (auto E = ParseExpression()) {
		return llvm::make_unique<FunctionAST>(std::move(Proto), 
											std::move(E));
	}
	return nullptr;
}

// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
	SourceLocation FnLoc = CurLoc;
	if (auto E = ParseExpression()) {
		auto Proto = llvm::make_unique<PrototypeAST>(FnLoc, "main", 
						std::vector<std::string>());
		return llvm::make_unique<FunctionAST>(std::move(Proto),
						std::move(E));
	}
	return nullptr;
}

// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern()
{
	getNextToken(); // eat extern
	return ParsePrototype();
}

