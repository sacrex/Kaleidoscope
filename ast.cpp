//Abstract Syntax Tree (aka Parse Tree)

namespace {

// ExprAst - Base class for all expression nodes.
class ExprAST
{
public:
	virtual ~ExprAST() = default;
	virtual Value *codegen() = 0;

	virtual void dump() const
	{
		fprintf(stderr, "{\nExprAST: ");
		fprintf(stderr, "\n}\n");
	}
};

// NumberExprAst - Expression class for numeric literals like "1.0"
class NumberExprAST : public ExprAST
{
	double Val;
public:
	NumberExprAST(double Val) : Val(Val) {}
	Value *codegen() override;

	void dump() const override
	{
		fprintf(stderr, "{\nNumberExprAST Val: %f\n", Val);
		fprintf(stderr, "\n}\n");
	}
};

// VariableExprAST - Expression class for referencing a variable, like "a"
class VariableExprAST : public ExprAST
{
	std::string Name;
public:
	VariableExprAST(const std::string &Name) : Name(Name) {}
	Value *codegen() override;

	void dump() const override
	{
		fprintf(stderr, "{\nVariableExprAST Name: %s\n", Name.c_str());
		fprintf(stderr, "\n}\n");
	}
};

// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST
{
	char Op;
	std::unique_ptr<ExprAST> LHS, RHS;
public:
	BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
					std::unique_ptr<ExprAST> RHS)
			:Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
	Value *codegen() override;

	void dump() const override
	{
		fprintf(stderr, "{\nBinaryExprAST: OP %c", Op);
		LHS->dump();
		RHS->dump();
		fprintf(stderr, "\n}\n");
	}

};

// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST
{
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;
public:
	CallExprAST(const std::string &Callee,
					std::vector<std::unique_ptr<ExprAST>> Args)
			:Callee(Callee), Args(std::move(Args)) {}
	Value *codegen() override;
	
	void dump() const override
	{
		fprintf(stderr, "{\nCallExprAST: Callee %s\n", Callee.c_str());
		for (auto &i : Args) {
			i->dump();
		}
		fprintf(stderr, "\n}\n");
	}
};

// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST
{
	std::unique_ptr<ExprAST> Cond, Then, Else;	
public:
	IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
					std::unique_ptr<ExprAST> Else)
			:Cond(std::move(Cond)), Then(std::move(Then)),
			Else(std::move(Else)) {}

	Value *codegen() override;
};

// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST
{
	std::string VarName;
	std::unique_ptr<ExprAST> Start, End, Step, Body;
public:
	ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
					std::unique_ptr<ExprAST> End,
					std::unique_ptr<ExprAST> Step,
					std::unique_ptr<ExprAST> Body)
			:VarName(VarName), Start(std::move(Start)), End(std::move(End)),
				Step(std::move(Step)), Body(std::move(Body)){}

	Value *codegen() override;
};

// PrototypeAST - This class represents the "prototype" for a function,
// which captures its name, and its argument names (thus implicitly the
// number of arguments the function takes).
class PrototypeAST
{
	std::string Name;
	std::vector<std::string> Args;
public:
	PrototypeAST(const std::string &Name, std::vector<std::string> Args)
			:Name(Name), Args(std::move(Args)) {}

	const std::string &getName() const
	{
		return Name;
	}

	Function *codegen();

	void dump() const 
	{
		fprintf(stderr, "{\nPrototypeAST: Name %s\n", Name.c_str());
		fprintf(stderr, "Args: ");
		for (auto &a : Args) {
			fprintf(stderr, "%s ", a.c_str());
		}
		fprintf(stderr, "\n}\n");
	}
};

// FunctionAST - This class represents a function definition itself.
class FunctionAST
{
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<ExprAST> Body;
public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto,
					std::unique_ptr<ExprAST> Body)
			:Proto(std::move(Proto)), Body(std::move(Body)) {}

	Function *codegen();

	void dump() const
	{
		fprintf(stderr, "{\nFunctionAST: Proto: ");
		Proto->dump();
		
		fprintf(stderr, "Body: ");
		Body->dump();
		fprintf(stderr, "\n}\n");
	}
};
}
