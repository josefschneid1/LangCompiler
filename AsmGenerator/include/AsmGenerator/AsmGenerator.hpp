#ifndef asmgenerator_hpp
#define asmgenerator_hpp
#include "Tac/Tac.hpp"
#include "Register.hpp"
#include <vector>
#include <ostream>
#include <array>
#include <tuple>
#include <span>
#include <utility>
namespace assembly
{
	namespace tac = intermediate_rep::tac;

	using BasicBlock = std::span<tac::Quadruple>;

	struct LiveUseInfo
	{
		bool live = false;
		int nextUse = -1;
	};

	class AsmGenerator
	{
	public:
		AsmGenerator(std::vector<tac::Function>& functions, std::ostream& os);

		void gen();

		std::vector<BasicBlock> getBasicBlocks(tac::Function& function);

		std::vector<std::tuple<LiveUseInfo,LiveUseInfo,LiveUseInfo>> nextUseLive(BasicBlock& basicBlock, intermediate_rep::SymbolTable& sym_table);

	private:

		void resetSymbolTable(intermediate_rep::SymbolTable& sym_table);

		std::vector<tac::Function>& functions;

		RegisterStateMachine registerState;		
		
		std::ostream& os;
 	
 	};
 	void printLiveNessRanges(std::ostream& os, BasicBlock& basicBlock,std::vector<std::tuple<LiveUseInfo,LiveUseInfo,LiveUseInfo>>& info);

}


std::ostream& operator<<(std::ostream& os, const assembly::BasicBlock& block);
std::ostream& operator<<(std::ostream& os, const std::vector<assembly::BasicBlock>& block);

#endif