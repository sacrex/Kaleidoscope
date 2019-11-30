//
// Lexer
//

// The lexer return tokens [0-255] if it is an unknower character,
// otherwise one of these for known things.
enum Token{
	tok_eof = -1,

	//commands
	tok_def = -2,
	tok_extern = -3,

	//primary
	tok_identifier = -4,
	tok_number = -5,

	// control
	tok_if = -6,
	tok_then = -7,
	tok_else = -8,
	tok_for = -9,
	tok_in = -10,
	
	// operators
	tok_binary = -11,
	tok_unary = -12,
	
	// var definition
	tok_var = -13
};

std::string getTokName(int Tok)
{
	switch(Tok) {
		case tok_eof:
			return "eof";
		case tok_def:
			return "def";
		case tok_extern:
			return "extern";
		case tok_identifier:
			return "identifier";
		case tok_number:
			return "number";
		case tok_if:
			return "if";
		case tok_then:
			return "then";
		case tok_else:
			return "else";
		case tok_for:
			return "for";
		case tok_in:
			return "in";
		case tok_binary:
			return "binary";
		case tok_unary:
			return "unary";
		case tok_var:
			return "var";
	}
	return std::string(1, (char)Tok);
}

namespace {
	class PrototypeAST;
	class ExprAST;
}

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);

struct DebugInfo {
	DICompileUnit *TheCU;
	DIType 		  *DblTy;
	std::vector<DIScope *> LexicalBlocks;

	void emitLocation(ExprAST *AST);
	DIType *getDoubleTy();
} KSDbgInfo;

struct SourceLocation {
	int Line;
	int Col;
};

static SourceLocation CurLoc;
static SourceLocation LexLoc = {1, 0};

static int advance() {
	int LastChar = getchar();
	
	if (LastChar == '\n' || LastChar == '\r') {
		LexLoc.Line++;
		LexLoc.Col = 0;
	} else {
		LexLoc.Col++;
	}

	return LastChar;
}

static std::string IdentifierStr; // Filled in if tok_identifier
static double Number; // Filled in if tok_number

// gettok - Return the next token from standard input.
static int gettok()
{
	static int LastChar = ' ';
	
	//Skip any whitespace
	while (isspace(LastChar)) {
		LastChar = advance();
	}
	
	CurLoc = LexLoc;

	//identifer [a-zA-Z][a-zA-Z0-9]+
	if (isalpha(LastChar)) {
		IdentifierStr = LastChar;

		while(isalnum((LastChar = advance()))) {
			IdentifierStr += LastChar;
		}

		if (IdentifierStr == "def") {
			return tok_def;
		}
		if (IdentifierStr == "extern") {
			return tok_extern;
		}
		if (IdentifierStr == "if") {
			return tok_if;
		}
		if (IdentifierStr == "then") {
			return tok_then;
		}
		if (IdentifierStr == "else") {
			return tok_else;
		}
		if (IdentifierStr == "for") {
			return tok_for;
		}
		if (IdentifierStr == "in") {
			return tok_in;
		}
		if (IdentifierStr == "binary") {
			return tok_binary;
		}
		if (IdentifierStr == "unary") {
			return tok_unary;
		}
		if (IdentifierStr == "var") {
			return tok_var;
		}
		return tok_identifier;
	}

	//number [0-9.]+
	if (isdigit(LastChar) || LastChar == '.') {
		std::string numStr;
		numStr = LastChar;
		while (isdigit(LastChar = advance()) || LastChar == '.') {
			numStr += LastChar;
		}

		Number = strtod(numStr.c_str(), 0);
		return tok_number;
	}

	//comment until end of line.
	if (LastChar == '#') {
		do {
			LastChar = advance();
		}while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF) {
			return gettok();  //recursive to get next token
		}
	}

	// Check for end of file. Don't eat the EOF.
	if (LastChar == EOF) {
		return tok_eof;
	}

	//otherwise, just return the character as its ascii value.
	int ThisToken = LastChar;
	LastChar = advance();
	return ThisToken;
}
