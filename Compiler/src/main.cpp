#include "Ast/Ast.hpp"
#include "Parser/Parser.hpp"
#include "Lexer/Lexer.hpp"
#include <string>
#include <vector>
#include <iostream>
#include "Token/Token.hpp"
#include "TacGenerator/TacGenerator.hpp"
#include "AsmGenerator/AsmGenerator.hpp"





int main()
{
	std::string program = R"(
	
	bool main()
	{
		if(5 < 3 and true)
		{
			int a = 5;
		}
		else
		{
			int b = 3;
		}
		return true;
	}
)";


	std::cout << program << '\n';

	{
		Lexer::Lexer lexer{program};
		auto t = lexer.next();
		while( t.type != intermediate_rep::Token::Type::Eof)
		{
			std::cout << t << ", ";
			t = lexer.next();
		}
		std::cout << '\n';
	}

	Lexer::Lexer lexer{program};

	Parser::Parser parser{&lexer};

	auto ast = parser.program();

	std::cout << ast << '\n';

	tac_gen::TacGenerator tacGen{&ast};

	auto tac = tacGen.gen();

	for(auto& function : tac)
	{
		std::cout << function << '\n';
	}

	std::cout << "BasicBlock Code\n";

	assembly::AsmGenerator assemblyGen{tac, std::cout};

	auto basicBlocks = assemblyGen.getBasicBlocks(tac[0]);

	std::cout << basicBlocks << '\n';


	auto live = assemblyGen.nextUseLive(basicBlocks[0], tac[0].sym_entry->parameter_scope->getChild(0));

	printLiveNessRanges(std::cout, basicBlocks[0], live);

	std::cout << "Assembly Code\n";

	assemblyGen.gen();







}