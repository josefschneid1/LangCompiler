#include "AsmGenerator.hpp"
#include <iostream>
#include <limits>
#include <concepts>
#include <iomanip>


namespace assembly
{
	AsmGenerator::AsmGenerator(std::vector<intermediate_rep::tac::Function>& functions, std::ostream& os):
		functions{functions}, os{os}
	{}

	std::vector<BasicBlock> AsmGenerator::getBasicBlocks(tac::Function& function)
	{
		std::vector<BasicBlock> basicBlocks;
		auto blockBegin = function.tac.begin();
		auto blockEnd = blockBegin;
		while(blockBegin != function.tac.end())
		{
			while(blockEnd != function.tac.end())
			{
				if(blockEnd != blockBegin && blockEnd->label.size() != 0)
					break;

				if(tac::isJump(blockEnd->instr))
				{
					++blockEnd;
					break;
				}
				++blockEnd;
			}
			basicBlocks.push_back({blockBegin, blockEnd});
			blockBegin = blockEnd;
		}
		return basicBlocks;
	}

	void AsmGenerator::gen()
	{	



	}

	std::vector<std::tuple<LiveUseInfo,LiveUseInfo,LiveUseInfo>> AsmGenerator::nextUseLive(BasicBlock& basicBlock, intermediate_rep::SymbolTable& sym_table)
	{
		using Variable = intermediate_rep::SymbolTable::Variable;

		resetSymbolTable(sym_table);

		std::vector<std::tuple<LiveUseInfo,LiveUseInfo,LiveUseInfo>> info(basicBlock.size());

		for(int i = basicBlock.size() - 1; i >= 0; --i)
		{
			auto& quad = basicBlock[i];

			if(std::holds_alternative<Variable*>(quad.result))
			{
				auto var =  std::get<Variable*>(quad.result);
				auto&[live,nextUse] = std::get<0>(info[i]);
				live  = var->live;
				nextUse = var->nextUse;
				var->live = false;
				var->nextUse = -1;
			}

			if(std::holds_alternative<Variable*>(quad.arg1))
			{
				auto var =  std::get<Variable*>(quad.arg1);
				auto&[live,nextUse] = std::get<1>(info[i]);
				live  = var->live;
				nextUse = var->nextUse;

				var->live = true;
				var->nextUse = i;
			}

			if(std::holds_alternative<Variable*>(quad.arg2))
			{
				auto var =  std::get<Variable*>(quad.arg2);
				auto&[live,nextUse] = std::get<2>(info[i]);
				live  = var->live;
				nextUse = var->nextUse;
				var->live = true;
				var->nextUse = i;
			}
		}

		return info;
	}

	void AsmGenerator::resetSymbolTable(intermediate_rep::SymbolTable& sym_table)
	{	
		for(auto& [name, entry] : sym_table)
		{
			if(auto varPtr = std::get_if<intermediate_rep::SymbolTable::Variable>(&entry))
			{
				if(varPtr->name.starts_with("__"))
				{
					varPtr->live = false;
					varPtr->nextUse = -1;
					std::cout << "Temp init " << name << '\n';
				}
				else
				{
					varPtr->live = true;
					varPtr->nextUse = 1000;
					std::cout << "Var init " << name << '\n';
				}
			}
		}
	}

	struct Visitor
	{
		auto operator()(intermediate_rep::SymbolTable::Variable*& var) -> std::string
		{
			return "Alive " + std::to_string(var->live) + " Next use " + std::to_string(var->nextUse);
		}
		auto operator()(auto&) -> std::string
		{
			return "Empty";
		}
	};

	void printLiveNessRanges(std::ostream& os, BasicBlock& basicBlock,std::vector<std::tuple<LiveUseInfo,LiveUseInfo,LiveUseInfo>>& info)
	{
		for(int i = 0; i < basicBlock.size(); ++i)
		{
			os << basicBlock[i] << '\t';		
			os << '(' << std::setw(20) << std::visit(Visitor{}, basicBlock[i].result) << ")\t";
			os << '(' << std::setw(20) << std::visit(Visitor{}, basicBlock[i].arg1)<< ")\t";
			os << '(' << std::setw(20) << std::visit(Visitor{}, basicBlock[i].arg2)<< ")\n";
		}

	}

}



std::ostream& operator<<(std::ostream& os, const assembly::BasicBlock& block)
{
	for(auto& quad : block)
 	{
 		os << quad << '\n';
 	}
 	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<assembly::BasicBlock>& blocks)
{
	int i = 0;
	for(auto& block : blocks)
	{
		os << "Block Nr. " << (i++) << '\n' << block << '\n';
	}

	return os;
}