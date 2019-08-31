#include<stdio.h>
#include<stdlib.h>

#include <string>

enum Token{
	tok_eof = -1,

	//keywords
	tok_def = -2,
	tok_extern = -3,

	//primary
	tok_identifier = -4,
	tok_number = -5
};

static string IdentifierStr;
static double Number;

static int gettok()
{
	static int LastChar = ' ';

	while (isspace(LastChar)) {
		LastChar = getchar();
	}

	//identifer [a-zA-Z][a-zA-Z0-9]+
	if (isalpha(LastChar)) {
		IdentifierStr = LastChar;

		while(isalnum((LastChar = getchar()))) {
			IdentifierStr += LastChar;
		}

		if (IdentifierStr == "def") {
			return tok_def;
		}
		if (Identifier == "extern") {
			return tok_extern;

		}
		return tok_identifier;
	}

	//number [0-9.]+
	if (isnum(LastChar) || LastChar == '.') {
		std::string numStr;
		numStr = LastChar;
		while (isdigit(LastChar = getchar()) || LastChar == '.') {
			numStr += LastChar;
		}

		Number = strtod(numStr.c_str(), 0);
		return tok_number;
	}

	//comment #
	if (LastChar == '#') {
		do {
			LastChar = getchar();
		}while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF) {
			return gettok();  //recursive to get next token
		}
	}

	//EOF
	if (LastChar == EOF) {
		return tok_eof;
	}

	//otherwise, we return the ascii
	int ThisToken = LastChar;
	LastChar = getchar();
	return ThisToken;
}
