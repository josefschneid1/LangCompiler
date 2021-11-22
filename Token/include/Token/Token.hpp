#ifndef token_hpp
#define token_hpp

#include <string>
#include <iostream>

namespace intermediate_rep
{
	struct Token
	{
		enum Type
		{
			True,
			False,
			While,
			If,
			Else,
			Return,

			Id,
			IntLit,
			FloatLit,
			StrLit,

			OParen,
			CParen,
			OSBracket,
			CSBracket,
			OCBracket,
			CCBracket,

			Comma,
			Semicolon,

			Plus,
			Minus,
			Star,
			Slash,

			Less,
			LessEqual,
			Greater,
			GreaterEqual,
			
			Equal,
			NotEqual,

			Assign,
			
			And,
			Or,
			Not,

			Eof,
		} type;

		std::string lexem;
	};

	std::string tokenTypeToStr(Token::Type type);

	std::ostream& operator<<(std::ostream& os, const Token& t);

	struct TokenProducer
	{
		virtual Token next() = 0;
		virtual ~TokenProducer(){}
	};
}

#endif