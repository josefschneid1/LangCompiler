#ifndef tac_hpp
#define tac_hpp
#include <variant>
#include "Ast/SymbolTable.hpp"
#include <string>
#include <ostream>

namespace intermediate_rep::tac
{
	/*
		Add ==> result = arg1 + arg2
		Sub ==> result = arg1 - arg2
		Mul ==> result = arg1 * arg2
		Div ==> result = arg1 / arg2

		Less 	  		==> result = arg1 <  arg2
		LessEqual 		==> result = arg1 <= arg2
		Greater   		==> result = arg1 >  arg2
		GreaterEqual 	==> result = arg1 >= arg2

		Equal 	 ==> result = arg1 == arg2
		NotEqual ==> result = arg1 != arg2
	
		IfJump 	    	==> If arg1 goto result
		IfFalseJump     ==> If not arg1 goto result

		Jump ==> goto result 

		Not 	==> result = not arg1
		Negate  ==> result = -   arg1

		Call ==> result = call(arg1), n(arg2)

		Assign ==> result = arg1

		Param ==> arg1

	*/

	enum class InstructionType
	{
		Add,
		Sub,
		Mul,
		Div,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		Equal,
		NotEqual,
		IfJump, 
		IfFalseJump, 
		Jump, 
		Not, 
		Negate, 
		Call, 
		Assign, 
		Param, 
		Return, 
		And,
		Or
	};

	bool isJump(InstructionType type);

	template<typename T>
	struct Constant
	{
		T value;
		explicit Constant(T value):
			value{value}
		{}

	};

	struct CallArgNum
	{
		std::size_t size;
		explicit CallArgNum(std::size_t size):
			size{size}
		{}
	};


	using Label = std::string;

	using Address = std::variant<std::monostate, intermediate_rep::SymbolTable::Variable*,intermediate_rep::SymbolTable::Function*,
	Constant<int>,Constant<double>, Constant<bool>, Label, CallArgNum>;

	struct Quadruple
	{
		Label label;
		InstructionType instr;
		Address result;
		Address arg1;
		Address arg2;
	};

	std::ostream& operator<<(std::ostream& os, const Quadruple& quad);

	struct Function
	{
		intermediate_rep::SymbolTable::Function* sym_entry;
		std::vector<Quadruple> tac;
	};

	std::ostream& operator<<(std::ostream& os, const Function& function);

}





#endif