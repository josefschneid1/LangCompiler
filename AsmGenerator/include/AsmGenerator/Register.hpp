#include <variant>
#include <map>
#include "Ast/SymbolTable.hpp"
#include "Tac/Tac.hpp"
#include <ostream>
#include <optional>

namespace assembly
{
	enum class Register
	{
		RAX,
		RBX,
		RCX,
		RDX,
		RSI,
		RDI,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
 
		RBP,	// Base Pointer
		RSP,	// Stack Pointer
		RIP,	// Instruction Pointer
	};

	struct RegisterDescriptor
	{
		// float int register etc. ....
		Register reg;
		std::vector<intermediate_rep::SymbolTable::Variable*> content;
	};

	struct RegisterStateMachine 
	{
		RegisterStateMachine();
		std::map<Register, RegisterDescriptor> registers;
		RegisterDescriptor* getEmptyRegister();
		void clear();
	};

	std::ostream& operator<<(std::ostream& os, Register reg);

	std::ostream& operator<<(std::ostream& os, const RegisterStateMachine& regm);

}