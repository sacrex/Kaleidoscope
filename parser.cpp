

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
	auto Result = std::make_unique<NumberExprAST>(Number);
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
		return LogError("expected ')'");
	}
	getNextToken(); // eat )
	return V;
}

// identifierexpr 
// 			::= identifer                          --- (1)
// 			::= identifer '(' expression ')'	   --- (2)
static std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
	std::string IdName = IdentifierStr;
	getNextToken(); // eat identifier

	if (CurTok != '(') {  // (1)
		return std::make_unique<VariableExprAST>(IdName);
	}

	//(2)
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
			getNextToken();
		}
	}
	
	getNextToken(); // eat ). update CurTok to next token
	
	return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

// primary
// 		::= identifierexpr
// 		::= numberexpr
// 		::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary()
{
	switch (CurTok) {
		default:
			return LogError("unknown token when expecting an expression");
		case tok_identifier:
			return ParseIdentifierExpr();
		case tok_number:
			return ParseNumberExpr();
		case '(':
			return ParseParenExpr();
	}
}

// binoprhs
// 		::= ('+' primary)*
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
		// binop, consume it, otherwise we are done.
		if (TokPrec < ExprPrec) {
			return LHS;
		}

		// Okay, we know this its a binop.
		int BinOp = CurTok;
		getNextToken(); // eat binop

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimary();
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
		LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
												std::move(RHS));
	}
}


