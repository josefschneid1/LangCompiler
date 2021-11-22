#ifndef tacgenerator_hpp
#define tacgenerator_hpp
#include "Ast/Ast.hpp"
#include "Tac/Tac.hpp"
#include <vector>

namespace tac_gen
{
	class TacGenerator
	{
	public:
		TacGenerator(intermediate_rep::ast::Program* ast);

		auto gen() -> std::vector<intermediate_rep::tac::Function>;

	private:
		intermediate_rep::ast::Program* ast;
	};

}



#endif