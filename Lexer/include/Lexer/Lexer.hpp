#ifndef lexer_hpp
#define lexer_hpp
#include "Token/Token.hpp"
#include <string>

namespace Lexer
{
	class Lexer : public intermediate_rep::TokenProducer
	{
	public:
		Lexer(const std::string& program);

		intermediate_rep::Token next() override;

	private:
		std::string program;
		std::string::iterator lexemStart;
	};
}


#endif