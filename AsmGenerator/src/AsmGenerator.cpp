#include "AsmGenerator.hpp"
#include <iostream>
#include <limits>
#include <concepts>
#include <iomanip>
#include <map>
#include <variant>
#include <algorithm>
#include <string_view>
#include <sstream>

namespace assembly
{
	const std::map<intermediate_rep::SymbolTable::Variable::Type, int> sizeOf
	{
		{intermediate_rep::SymbolTable::Variable::Type::Int, 8},
		{intermediate_rep::SymbolTable::Variable::Type::Float, 8},
		{intermediate_rep::SymbolTable::Variable::Type::Bool, 1},
		{intermediate_rep::SymbolTable::Variable::Type::Str, 8},
	};

	const std::map<int, std::string_view> datatypes
	{
		{1, "BYTE"},
		{8, "QWORD"}
	};

	std::string sub_register(assembly::Register reg, int size)
	{
		std::stringstream ss;
		ss << reg;
		auto s = ss.str();
		auto center_char = s[1];
		switch (size)
		{
		case 1:
			return center_char + "L";
		case 2:
			return center_char + "H";
		case 4:
			return "E" + std::to_string(center_char) + "X";
		case 8:
			return "R" + std::to_string(center_char) + "X";
		default:
			throw std::runtime_error("");

		}
	}

	AsmGenerator::AsmGenerator(std::vector<intermediate_rep::tac::Function>& functions, std::ostream& os) :
		functions{ functions }, os{ os }
	{}

	std::vector<BasicBlock> AsmGenerator::getBasicBlocks(tac::Function& function)
	{
		std::vector<BasicBlock> basicBlocks;
		auto blockBegin = function.tac.begin();
		auto blockEnd = blockBegin;
		while (blockBegin != function.tac.end())
		{
			while (blockEnd != function.tac.end())
			{
				if (blockEnd != blockBegin && blockEnd->label.size() != 0)
					break;

				if (tac::isJump(blockEnd->instr))
				{
					++blockEnd;
					break;
				}
				++blockEnd;
			}
			basicBlocks.push_back({ blockBegin, blockEnd });
			blockBegin = blockEnd;
		}
		return basicBlocks;
	}

	void AsmGenerator::gen()
	{
		os << "section .text\nglobal main\n";
		for (auto& function : functions)
		{
			offset = 8;
			computeParameterOffsets(*function.sym_entry->parameter_scope);
			auto basicBlocks = getBasicBlocks(function);
			// Function Label
			os << function.sym_entry->name << ":\n";
			os << "push rbp\nmov rbp, rsp\n";
			for (auto block : basicBlocks)
			{
				resetSymbolTable(*function.sym_entry->parameter_scope);
				registerState.clear();
				auto use = nextUseLive(block, *function.sym_entry->parameter_scope);

				for (int i = 0; i < block.size(); ++i)
				{
					allocateRegisters(block[i], use[i]);
				}
			}
		}
	}

	std::vector<std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>> AsmGenerator::nextUseLive(BasicBlock& basicBlock, intermediate_rep::SymbolTable& sym_table)
	{
		using Variable = intermediate_rep::SymbolTable::Variable;

		resetSymbolTable(sym_table);

		std::vector<std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>> info(basicBlock.size());

		for (int i = basicBlock.size() - 1; i >= 0; --i)
		{
			auto& quad = basicBlock[i];

			if (std::holds_alternative<Variable*>(quad.result))
			{
				auto var = std::get<Variable*>(quad.result);
				auto& [live, nextUse] = std::get<0>(info[i]);
				live = var->live;
				nextUse = var->nextUse;
				var->live = false;
				var->nextUse = -1;
			}

			if (std::holds_alternative<Variable*>(quad.arg1))
			{
				auto var = std::get<Variable*>(quad.arg1);
				auto& [live, nextUse] = std::get<1>(info[i]);
				live = var->live;
				nextUse = var->nextUse;

				var->live = true;
				var->nextUse = i;
			}

			if (std::holds_alternative<Variable*>(quad.arg2))
			{
				auto var = std::get<Variable*>(quad.arg2);
				auto& [live, nextUse] = std::get<2>(info[i]);
				live = var->live;
				nextUse = var->nextUse;
				var->live = true;
				var->nextUse = i;
			}
		}

		return info;
	}

	assembly::RegisterDescriptor* AsmGenerator::load(intermediate_rep::SymbolTable::Variable* var)
	{
		// See if variable is already inside a register
		auto reg = std::find_if(var->variableDescriptor.begin(), var->variableDescriptor.end(), [](auto& addr) {
			if (auto regDesc = std::get_if<assembly::RegisterDescriptor*>(&addr))
				return true;
			return false;
			}
		);

		if (reg != var->variableDescriptor.end())
		{
			// the most recent version is already inside a register 
			return std::get<assembly::RegisterDescriptor*>(*reg);
		}

		// get a free register

		auto descrPtr = registerState.getEmptyRegister();

		// Found a free register
		if (descrPtr)
		{
			// Add Variable to Register
			descrPtr->content.push_back(var);

			// Variable now inside register and memory
			var->variableDescriptor.push_back(descrPtr);

			if (sizeOf.at(var->type) < 8)
			{
				os << "movsx ";
			}
			else
			{
				os << "mov ";
			}

			os << sub_register(descrPtr->reg, sizeOf.at(var->type)) << ", [rbp + " << var->basePointerOffset << "]\n";

			return descrPtr;
		}
		else
		{
			// There is no free register available
			// decide which variable to drop
			throw std::runtime_error("");
		}
	}

	void AsmGenerator::store(intermediate_rep::SymbolTable::Variable* var)
	{
		intermediate_rep::SymbolTable::Variable::Memory* mem = nullptr;
		assembly::RegisterDescriptor* descr = nullptr;

		for (auto& addr : var->variableDescriptor)
		{
			if (auto descrPtrPtr = std::get_if<assembly::RegisterDescriptor*>(&addr))
			{
				descr = *descrPtrPtr;
			}
			else
			{
				mem = &std::get<intermediate_rep::SymbolTable::Variable::Memory>(addr);
			}
		}

		if (!descr)
		{
			throw std::runtime_error("Trying to store variable that is not in a register");
		}

		if (mem)
		{
			// No need to store variable because the most recent version is also in memory
			return;
		}

		// Check if variable has a memory address
		if (var->basePointerOffset == 0)
		{
			// Compute memory address
			computeLocalOffset(var);
		}

		os << "mov [RBP + " << var->basePointerOffset << "], " << sub_register(descr->reg,sizeOf.at(var->type))<< '\n';

		// most recent version is now also in memory, but also in the register
		var->variableDescriptor.push_back(intermediate_rep::SymbolTable::Variable::Memory{});
	}

	void AsmGenerator::copyOrDrop(intermediate_rep::SymbolTable::Variable* var, LiveUseInfo& info)
	{
		// is the variable inside a register?
		auto iterReg = std::find_if(var->variableDescriptor.begin(), var->variableDescriptor.end(), [](auto& variant) {
			if (std::holds_alternative<assembly::RegisterDescriptor*>(variant))
				return true;
			return false;
			});

		// it's not in a register
		if (iterReg == var->variableDescriptor.end())
			throw std::runtime_error("Variable not inside register");


		// Is the most recent version in memory?
		auto iter = std::find_if(var->variableDescriptor.begin(), var->variableDescriptor.end(), [](auto& variant) {
			if (std::holds_alternative<intermediate_rep::SymbolTable::Variable::Memory>(variant))
			{
				return true;
			}
			return false;
			});

		if (iter != var->variableDescriptor.end())
		{
			// current value in memory and register

			if (info.nextUse != -1)
			{
				// variable is still needed later on
				auto descr = registerState.getEmptyRegister();

				var->variableDescriptor.push_back(descr);
				descr->content.emplace_back(var);
				// current value now in 2 registers and memory
			}
			// no need to write value back to mem
			return;
		}
		else
		{
			// current value in register

			if (info.nextUse != -1)
			{
				// value still used later on
				auto descr = registerState.getEmptyRegister();
				os << "mov " << descr->reg << ", " << (std::get<assembly::RegisterDescriptor*>(*iterReg))->reg << '\n';
				var->variableDescriptor.push_back(descr);
				descr->content.emplace_back(var);

			}
			else
			{
				if (!var->name.starts_with("__"))
				{
					store(var);
				}
				return;
			}
		}
	}

	void AsmGenerator::overwriteWithResult(intermediate_rep::SymbolTable::Variable* var, assembly::RegisterDescriptor* descr)
	{
		for (auto& addr : descr->content)
		{
			addr->variableDescriptor.erase(std::remove_if(addr->variableDescriptor.begin(), addr->variableDescriptor.end(), [descr](auto& variant) {
				if (std::holds_alternative<assembly::RegisterDescriptor*>(variant) && std::get<assembly::RegisterDescriptor*>(variant) == descr)
				{
					return true;
				}
				return false;
				}));
		}
		descr->content.clear();
		var->variableDescriptor.clear();
		var->variableDescriptor.emplace_back(descr);
		descr->content.push_back(var);
	}


	struct ArithmeticVisitor
	{

		ArithmeticVisitor(const std::string& opcode, AsmGenerator& gen, std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>& info) :
			gen{ gen }, info{ info }, opcode{opcode}
		{}

		auto operator()(intermediate_rep::SymbolTable::Variable*& result, intermediate_rep::SymbolTable::Variable*& arg1, intermediate_rep::SymbolTable::Variable*& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descrArg1 = gen.load(arg1);
			auto descrArg2 = gen.load(arg2);
			gen.copyOrDrop(arg1, arg1_usage);
			gen.os << opcode << " " << descrArg1->reg << ", " << descrArg2->reg << '\n';
			gen.overwriteWithResult(result, descrArg1);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, intermediate_rep::SymbolTable::Variable*& arg1, tac::Constant<T>& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descrArg1 = gen.load(arg1);
			gen.copyOrDrop(arg1, arg1_usage);
			if constexpr (std::same_as<T,bool>)
			{
				gen.os << opcode << " " << descrArg1->reg << ", " << static_cast<int>(arg2.value) << '\n';
			}
			else
			{
				gen.os << opcode << " " << descrArg1->reg << ", " << arg2.value << '\n';
			}
			gen.overwriteWithResult(result, descrArg1);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, tac::Constant<T>& arg1, intermediate_rep::SymbolTable::Variable*& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descrArg2 = gen.load(arg2);
			auto descrArg1 = gen.registerState.getEmptyRegister();


			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "mov " << descrArg1->reg << ", " << static_cast<int>(arg1.value) << '\n';
			}
			else
			{
				gen.os << "mov " << descrArg1->reg << ", " << arg1.value << '\n';
			}

			gen.os << opcode << " " << descrArg1->reg << ", " << descrArg2->reg << '\n';
			gen.overwriteWithResult(result, descrArg1);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, tac::Constant<T>& arg1, tac::Constant<T>& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descr = gen.registerState.getEmptyRegister();


			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "mov " << descr->reg << ", " << static_cast<int>(arg1.value) << '\n';
				gen.os << opcode << " " << descr->reg << ", " << static_cast<int>(arg2.value) << '\n';
			}
			else
			{
				gen.os << "mov " << descr->reg << ", " << arg1.value << '\n';
				gen.os << opcode << " " << descr->reg << ", " << arg2.value << '\n';
			}

			gen.overwriteWithResult(result, descr);
		}

		auto operator()(auto&, auto&, auto&)
		{
			throw std::runtime_error("Unsupported Operands");
		}

		AsmGenerator& gen;
		std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>& info;
		std::string opcode;
	};

	struct ComparisonVisitor
	{

		ComparisonVisitor(const std::string& opcode, AsmGenerator& gen, std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>& info) :
			gen{ gen }, info{ info }, opcode{ opcode }
		{}

		auto operator()(intermediate_rep::SymbolTable::Variable*& result, intermediate_rep::SymbolTable::Variable*& arg1, intermediate_rep::SymbolTable::Variable*& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descrArg1 = gen.load(arg1);
			auto descrArg2 = gen.load(arg2);
			auto descr = gen.registerState.getEmptyRegister();
			gen.os << "cmp " << descrArg1->reg << ", " << descrArg2->reg << '\n';
			gen.os << opcode << " " << descr->reg << '\n';
			gen.overwriteWithResult(result, descr);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, intermediate_rep::SymbolTable::Variable*& arg1, tac::Constant<T>& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descrArg1 = gen.load(arg1);
			auto descr = gen.registerState.getEmptyRegister();

			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "cmp " << descrArg1->reg << ", " << static_cast<int>(arg2.value) << '\n';
			}
			else
			{
				gen.os << "cmp " << descrArg1->reg << ", " << arg2.value << '\n';
			}

			gen.os << opcode << " " << descr->reg << '\n';
			gen.overwriteWithResult(result, descr);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, tac::Constant<T>& arg1, intermediate_rep::SymbolTable::Variable*& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descrArg2 = gen.load(arg2);
			auto descrArg1 = gen.registerState.getEmptyRegister();
		 	
			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "mov " << descrArg1->reg << ", " << static_cast<int>(arg1.value) << '\n';

			}
			else
			{
				gen.os << "mov " << descrArg1->reg << ", " << arg1.value << '\n';
			}

			gen.os << "cmp " << descrArg1->reg << ", " << descrArg2->reg << '\n';
			gen.os << opcode << " " << descrArg1->reg << '\n';
			gen.overwriteWithResult(result, descrArg1);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, tac::Constant<T>& arg1, tac::Constant<T>& arg2)
		{
			auto& [result_usage, arg1_usage, arg2_usage] = info;
			auto descr = gen.registerState.getEmptyRegister();

			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "mov " << descr->reg << ", " << static_cast<int>(arg1.value) << '\n';
				gen.os << "cmp " << descr->reg << ", " << static_cast<int>(arg2.value) << '\n';
			}
			else
			{
				gen.os << "mov " << descr->reg << ", " << arg1.value << '\n';
				gen.os << "cmp " << descr->reg << ", " << arg2.value << '\n';
			}

			gen.os << opcode << " " << descr->reg << '\n';
			gen.overwriteWithResult(result, descr);
		}

		auto operator()(auto&, auto&, auto&)
		{
			throw std::runtime_error("Unsupported Operands");
		}

		AsmGenerator& gen;
		std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>& info;
		std::string opcode;
	};

	struct ReturnVisitor
	{

		ReturnVisitor(AsmGenerator& gen) :
			gen{ gen }
		{}

		auto operator()(intermediate_rep::SymbolTable::Variable*& arg1)
		{
			auto descr = gen.load(arg1);
			if (descr->reg != assembly::Register::RAX)
				gen.os << "mov RAX, " << descr->reg << '\n';
			gen.os << "ret\n";
		}

		template<typename T>
		auto operator()(tac::Constant<T>& arg1)
		{
			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "mov RAX, " << static_cast<int>(arg1.value) << '\n';

			}
			else
			{
				gen.os << "mov RAX, " << arg1.value << '\n';
			}
			gen.os << "ret\n";
		}

		auto operator()(std::monostate& arg1)
		{
			gen.os << "ret\n";
		}

		auto operator()(auto&)
		{
			throw std::runtime_error("Unsupported Operands");
		}

		AsmGenerator& gen;
	};

	struct ParamVisitor
	{

		ParamVisitor(AsmGenerator& gen) :
			gen{ gen }
		{}

		auto operator()(intermediate_rep::SymbolTable::Variable*& arg1)
		{
			auto descr = gen.load(arg1);
			gen.os << "push " << sub_register(descr->reg, sizeOf.at(arg1->type)) << '\n';
		}

		template<typename T>
		auto operator()(tac::Constant<T>& arg1)
		{
			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "push " << static_cast<int>(arg1.value) << '\n';
			}
			else
			{
				gen.os << "push " << arg1.value << '\n';
			}	
		}

		auto operator()(auto&)
		{
			throw std::runtime_error("Unsupported Operands");
		}

		AsmGenerator& gen;
	};

	struct AssignVisitor
	{

		AssignVisitor(AsmGenerator& gen) :
			gen{ gen }
		{}

		auto operator()(intermediate_rep::SymbolTable::Variable*& result, intermediate_rep::SymbolTable::Variable*& arg1)
		{
			auto descrArg2 = gen.load(arg1);
			descrArg2->content.push_back(result);
			result->variableDescriptor.clear();
			result->variableDescriptor.emplace_back(descrArg2);
		}

		template<typename T>
		auto operator()(intermediate_rep::SymbolTable::Variable*& result, tac::Constant<T>& arg2)
		{
			auto descr = gen.registerState.getEmptyRegister();
			descr->content.push_back(result);
			result->variableDescriptor.clear();
			result->variableDescriptor.emplace_back(descr);

			if constexpr (std::same_as<T, bool>)
			{
				gen.os << "mov " << descr->reg << ", " << static_cast<int>(arg2.value) << '\n';
			}
			else
			{
				gen.os << "mov " << descr->reg << ", " << arg2.value << '\n';
			}
		}

		auto operator()(auto&, auto&)
		{
			throw std::runtime_error("Unsupported Operands");
		}

		AsmGenerator& gen;
	};

	struct IfFalseJumpVisitor
	{

		IfFalseJumpVisitor(AsmGenerator& gen) :
			gen{ gen }
		{}

		auto operator()(std::string& label, intermediate_rep::SymbolTable::Variable*& arg1)
		{
			auto descr = gen.load(arg1);
			gen.os << "cmp " << descr->reg << ", 0\n";
			gen.os << "jz " << label << '\n';
		}

		auto operator()(std::string& label, tac::Constant<bool>& arg1)
		{
			if (!arg1.value)
			{
				gen.os << "jmp " << label << '\n';
			}
		}

		auto operator()(auto&, auto&)
		{
			throw std::runtime_error("Unsupported Operands");
		}

		AsmGenerator& gen;
	};
	

	void AsmGenerator::allocateRegisters(tac::Quadruple& quad, std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>& info)
	{
		using enum tac::InstructionType;

		//std::cout << registerState << '\n';

		if(quad.label.size() > 0)
			os << quad.label << ": ";

		switch (quad.instr)
		{
		case Add:
		{
			std::visit(ArithmeticVisitor{ "add",  *this, info}, quad.result, quad.arg1, quad.arg2);
			break;
		}
		case Sub:
			std::visit(ArithmeticVisitor{ "sub",  *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case Mul:
			std::visit(ArithmeticVisitor{ "mul",  *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case Div:
			std::visit(ArithmeticVisitor{ "div",  *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case Less:
			std::visit(ComparisonVisitor{ "setl", *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case LessEqual:
			std::visit(ComparisonVisitor{ "setle", *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case Greater:
			std::visit(ComparisonVisitor{ "setg", *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case GreaterEqual:
			std::visit(ComparisonVisitor{ "setge", *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case Equal:
			std::visit(ComparisonVisitor{ "sete", *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case NotEqual:
			std::visit(ComparisonVisitor{ "setne", *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case IfJump:
			throw std::runtime_error("Not implemented\n");
		case IfFalseJump:
			std::visit(IfFalseJumpVisitor{ *this }, quad.result, quad.arg1);
			break;
		case Jump:
		{
			os << "jmp " << std::get<std::string>(quad.result) << '\n';
			break;
		}
		case Not:
			break;
		case Negate:
			break;
		case Call:
		{
			//tac.push_back(tac::Quadruple{ label, tac::InstructionType::Call, varPtr, call.sym_entry, tac::CallArgNum{call.args.size()} });





			break;
		}
		case Assign:
			std::visit(AssignVisitor{ *this }, quad.result, quad.arg1);
			break;
		case Param:
			std::visit(ParamVisitor{ *this }, quad.arg1);
			break;
		case Return:
			std::visit(ReturnVisitor{ *this }, quad.arg1);
			break;
		case And:
			std::visit(ArithmeticVisitor{ "and",  *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		case Or:
			std::visit(ArithmeticVisitor{ "or",  *this, info }, quad.result, quad.arg1, quad.arg2);
			break;
		default:
			throw std::runtime_error("Unsupported Three Address Code Operation");
			break;
		}
	}

	void AsmGenerator::computeParameterOffsets(intermediate_rep::SymbolTable& para_scope)
	{
		int offset = -8;
		for (auto& [name, entry] : para_scope.non_rec_view())
		{
			auto& var = std::get<intermediate_rep::SymbolTable::Variable>(entry);
			offset -= sizeOf.at(var.type);
			var.basePointerOffset = offset;
			var.variableDescriptor.push_back(intermediate_rep::SymbolTable::Variable::Memory{});
		}
	}

	void AsmGenerator::computeLocalOffset(intermediate_rep::SymbolTable::Variable* var)
	{
		var->basePointerOffset = offset;
		offset += sizeOf.at(var->type);
	}

	void AsmGenerator::resetSymbolTable(intermediate_rep::SymbolTable& sym_table)
	{
		for (auto& [name, entry] : sym_table)
		{
			if (auto varPtr = std::get_if<intermediate_rep::SymbolTable::Variable>(&entry))
			{
				if (varPtr->name.starts_with("__"))
				{
					varPtr->live = false;
					varPtr->nextUse = -1;
					varPtr->variableDescriptor.clear();
				}
				else
				{
					varPtr->live = true;
					varPtr->nextUse = 1000;
					varPtr->variableDescriptor.clear();
					varPtr->variableDescriptor.emplace_back(intermediate_rep::SymbolTable::Variable::Memory{});
				}
			}
		}
	}

	struct Visitor
	{

		Visitor(LiveUseInfo& info) :
			info{ info }
		{}

		auto operator()(intermediate_rep::SymbolTable::Variable*& var) -> std::string
		{
			return "Alive: " + std::to_string(info.live) + ", Next use: " + (info.nextUse == -1 ? std::string("No next use") : std::to_string(info.nextUse));
		}
		auto operator()(auto&) -> std::string
		{
			return "Empty";
		}

		LiveUseInfo& info;
	};

	void printLiveNessRanges(std::ostream& os, BasicBlock& basicBlock, std::vector<std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>>& info)
	{
		using Variable = intermediate_rep::SymbolTable::Variable;

		for (int i = 0; i < basicBlock.size(); ++i)
		{
			os << std::boolalpha;
			os << std::left;
			os << std::setw(10) << "(" + std::to_string(i) + ")";
			os << basicBlock[i] << '\t';

			os << '(' << std::setw(35) << std::visit(Visitor{ std::get<0>(info[i]) }, basicBlock[i].result) << ")\t";
			os << '(' << std::setw(35) << std::visit(Visitor{ std::get<1>(info[i]) }, basicBlock[i].arg1) << ")\t";
			os << '(' << std::setw(35) << std::visit(Visitor{ std::get<2>(info[i]) }, basicBlock[i].arg2) << ")\n";
		}

	}

}

std::ostream& operator<<(std::ostream& os, const assembly::BasicBlock& block)
{
	for (auto& quad : block)
	{
		os << quad << '\n';
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<assembly::BasicBlock>& blocks)
{
	int i = 0;
	for (auto& block : blocks)
	{
		os << "Block Nr. " << (i++) << '\n' << block << '\n';
	}

	return os;
}