#include "Tac.hpp"
#include <map>
#include <iomanip>
#include <string_view>

namespace intermediate_rep::tac
{
	const std::map<InstructionType, std::string_view> instructionToStr
	{
		{InstructionType::Add, "Add"},
		{InstructionType::Sub, "Sub"},
		{InstructionType::Mul, "Mul"},
		{InstructionType::Div, "Div"},
		{InstructionType::Less, "Less"},
		{InstructionType::LessEqual, "LessEqual"},
		{InstructionType::Greater, "Greater"},
		{InstructionType::GreaterEqual, "GreaterEqual"},
		{InstructionType::Equal, "Equal"},
		{InstructionType::NotEqual, "NotEqual"},
		{InstructionType::IfJump, "IfJump"},
		{InstructionType::IfFalseJump, "IfFalseJump"},
		{InstructionType::Jump, "Jump"},
		{InstructionType::Not, "Not"},
		{InstructionType::Negate, "Negate"},
		{InstructionType::Call, "Call"},
		{InstructionType::Assign, "Assign"},
		{InstructionType::Param, "Param"},
		{InstructionType::Return, "Return"},
		{InstructionType::And, "And"},
		{InstructionType::Or, "Or"},
	};

	struct Visitor
	{
		auto operator()(const std::monostate&) -> std::string
		{
			return "Empty";
		}

		auto operator()(intermediate_rep::SymbolTable::Variable* const & var) -> std::string
		{
			return var->name;
		}

		auto operator()(intermediate_rep::SymbolTable::Function* const& fun) -> std::string
		{
			return fun->name;
		}

		template<typename T>
		auto operator()(const Constant<T> & c) -> std::string
		{
			return std::to_string(c.value);
		}

		auto operator()(const Label & label) -> std::string
		{
			return label;
		}

		auto operator()(const CallArgNum& num) -> std::string
		{
			return std::to_string(num.size);
		}
	};

	std::ostream& operator<<(std::ostream& os, const Quadruple& quad)
	{
		os << std::left;
		os << "[ ";
		os << std::setw(15) << quad.label;
		os << std::setw(15) << instructionToStr.at(quad.instr);
		os << std::setw(15) << std::visit(Visitor{}, quad.result);
		os << std::setw(15) << std::visit(Visitor{}, quad.arg1);
		os << std::setw(15) << std::visit(Visitor{}, quad.arg2);

		return os << " ]";;
	}

	std::ostream& operator<<(std::ostream& os, const Function& function)
	{
		os << std::left;
		os << "Function " << function.sym_entry->name << ":\n";
		os << "[ " << std::setw(15)  << "Index" <<std::setw(15) << "Label"  << std::setw(15) <<
		 "Instruction" << std::setw(15) << "Result" << std::setw(15) << "Arg1" << std::setw(15) << "Arg2" << " ]\n";
		for(int i = 0; i < 15 * 6 + 4; ++i)
			os << '#';
		os << '\n';

		for(int i = 0; i < function.tac.size(); ++i)
		{
			os << std::setw(15) << ("(" + std::to_string(i) + ")") <<function.tac[i] << '\n';
		}
		return os;
	}

	bool isJump(InstructionType type)
	{
		using enum InstructionType;
		switch(type)
		{
			case IfJump:
			case IfFalseJump:
			case Jump:
			case Call:
			case Return:
				return true;
			default:
				return false;
		}
	}
}


