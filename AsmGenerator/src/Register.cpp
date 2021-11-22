#include "Register.hpp"
#include <ostream>
#include <stdexcept>
#include <variant>
#include <concepts>

namespace assembly
{
	RegisterStateMachine::RegisterStateMachine()
	{
		for(int i = 0; i < 14; ++i)
		{
			registers[static_cast<Register>(i)] = RegisterDescriptor{static_cast<Register>(i)};
		}
	}

	void RegisterStateMachine::clear()
	{
		for(auto&[reg,descr] : registers)
		{
			descr.content = decltype(descr.content){std::monostate{}};
		}
	}

	const std::map<Register, std::string_view> registerToStr
	{
		{Register::RAX, "RAX"},
		{Register::RBX, "RBX"},
		{Register::RCX, "RCX"},
		{Register::RDX, "RDX"},
		{Register::RSI, "RSI"},
		{Register::RDI, "RDI"},
		{Register::R8,  "R8"},
		{Register::R9,  "R9"},
		{Register::R10, "R10"},
		{Register::R11, "R11"},
		{Register::R12, "R12"},
		{Register::R13, "R13"},
		{Register::R14, "R14"},
		{Register::R15, "R15"},
		{Register::RBP, "RBP"},
		{Register::RSP, "RSP"},
		{Register::RIP, "RIP"},
	};

	std::ostream& operator<<(std::ostream& os, Register reg)
	{
		return os << registerToStr.at(reg);
	}
}
