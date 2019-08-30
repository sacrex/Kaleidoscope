#include<stdio.h>
#include<stdlib.h>

#include <string>

enum {
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
	static int LastToken = ' ';
	while (isspace(LastToken)) {
		LastToken = getchar();
	}

	//identifer [a-zA-Z][a-zA-Z0-9]+
	if (isalpha(LastToken)) {
		IdentifierStr = LastToken;
		while(isalnum((LastToken = getchar()))) {
			IdentifierStr += LastToken;
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
	if (isnum(LastToken) || LastToken == '.') {
		std::string NumStr;
		NumStr = LastToken;
		while (isnum(LastToken = getchar()) || LastToken == '.') {
			NumStr += LastToken;
		}
		Number = strtod(NumStr.c_str(), 0);
		return tok_number;
	}

	//comment #
	if (LastToken == '#') {
		do {
			LastToken = getchar();
		}while(LastToken != EOF && LastToekn != '\n' && LastToken != '\r');

		if (LastkToken != EOF) {
			return gettok();  //recursive to get next token
		}
	}

	//EOF
	if (LastToken == EOF) {
		return tok_eof;
	}

	//otherwise, we return the ascii
	int ThisToken = LastToken;
	LastToken = getchar();
	return ThisToken;
}
