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

		friend class ArithmeticVisitor;
		friend class ReturnVisitor;
		friend class AssignVisitor;
		friend class ParamVisitor;
		friend class ComparisonVisitor;

	private:

		assembly::RegisterDescriptor* load(intermediate_rep::SymbolTable::Variable*);

		void store(intermediate_rep::SymbolTable::Variable*);

		void copyOrDrop(intermediate_rep::SymbolTable::Variable*, LiveUseInfo& info);

		void overwriteWithResult(intermediate_rep::SymbolTable::Variable*, assembly::RegisterDescriptor*);

		void allocateRegisters(tac::Quadruple& quad, std::tuple<LiveUseInfo, LiveUseInfo, LiveUseInfo>& info);

		void computeParameterOffsets(intermediate_rep::SymbolTable& para_scope);

		void computeLocalOffset(intermediate_rep::SymbolTable::Variable*);

		void resetSymbolTable(intermediate_rep::SymbolTable& sym_table);

		std::vector<tac::Function>& functions;

		RegisterStateMachine registerState;		
		
		std::ostream& os;

		int offset = 8;
 	
 	};
 	void printLiveNessRanges(std::ostream& os, BasicBlock& basicBlock,std::vector<std::tuple<LiveUseInfo,LiveUseInfo,LiveUseInfo>>& info);

}


std::ostream& operator<<(std::ostream& os, const assembly::BasicBlock& block);
std::ostream& operator<<(std::ostream& os, const std::vector<assembly::BasicBlock>& block);

#endif