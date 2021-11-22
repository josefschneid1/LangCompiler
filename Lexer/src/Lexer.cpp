#include "Lexer.hpp"
#include <map>
#include <string_view>
#include <stdexcept>

namespace Lexer
{
	const std::map<char, intermediate_rep::Token::Type> singleCharTokens
	{
		{'(', intermediate_rep::Token::Type::OParen},
		{')', intermediate_rep::Token::Type::CParen},
		{'[', intermediate_rep::Token::Type::OSBracket},
		{']', intermediate_rep::Token::Type::CSBracket},
		{'{', intermediate_rep::Token::Type::OCBracket},
		{'}', intermediate_rep::Token::Type::CCBracket},
		{'+', intermediate_rep::Token::Type::Plus},
		{'-', intermediate_rep::Token::Type::Minus},
		{'/', intermediate_rep::Token::Type::Slash},
		{'*', intermediate_rep::Token::Type::Star},
		{',', intermediate_rep::Token::Type::Comma},
		{';', intermediate_rep::Token::Type::Semicolon},
	};

	const std::map<std::string_view, intermediate_rep::Token::Type> keywords
	{
		{"and", intermediate_rep::Token::Type::And},
		{"or", intermediate_rep::Token::Type::Or},
		{"not", intermediate_rep::Token::Type::Not},
		{"return", intermediate_rep::Token::Type::Return},
		{"if", intermediate_rep::Token::Type::If},
		{"else", intermediate_rep::Token::Type::Else},
		{"while", intermediate_rep::Token::Type::While},
		{"true", intermediate_rep::Token::Type::True},
		{"false", intermediate_rep::Token::Type::False},
	};

	Lexer::Lexer(const std::string& program):
		program{program}, lexemStart{this->program.begin()}
	{}

	intermediate_rep::Token Lexer::next()
	{
		while(lexemStart != program.end() && std::isspace(*lexemStart))
		{
			++lexemStart;
		}

		if(lexemStart != program.end())
		{
			if(std::isalpha(*lexemStart))
			{
				auto start = lexemStart;
				while(std::isalnum(*lexemStart))
				{
					++lexemStart;
				}

				if(keywords.contains({start, lexemStart}))
				{
					return {keywords.at({start, lexemStart})};
				}
				else
				{
					return {intermediate_rep::Token::Id, {start, lexemStart}};

				}
			}
			else if(std::isdigit(*lexemStart))
			{
				auto start = lexemStart;
				while(std::isdigit(*lexemStart))
				{
					++lexemStart;
				}
				if(*lexemStart == '.')
				{
					++lexemStart;
					while(std::isdigit(*lexemStart))
					{
						++lexemStart;
					}
					return {intermediate_rep::Token::FloatLit, {start, lexemStart}};
				}
				else
				{
					return {intermediate_rep::Token::IntLit, {start, lexemStart}};
				}
			}
			else if(singleCharTokens.contains(*lexemStart))
			{
				return {singleCharTokens.at(*(lexemStart++))};
			}
			else if(*lexemStart == '\"')
			{
				auto start = lexemStart;
				++lexemStart;
				while(*lexemStart != '\n' || *lexemStart != '\"')
				{
					++lexemStart;
				}
				if(*lexemStart == '\n')
				{
					throw std::runtime_error("New line in Str Literal");
				}
				++lexemStart;
				return {intermediate_rep::Token::StrLit, {start, lexemStart}};
			}
			else
			{
				switch(*lexemStart)
				{
					case '=':
					{
						++lexemStart;
						if(*lexemStart == '=')
						{
							++lexemStart;
							return {intermediate_rep::Token::Equal};
						}
						else
						{
							return {intermediate_rep::Token::Assign};
						}
					}
					case '<':
					{
						++lexemStart;
						if(*lexemStart == '=')
						{
							++lexemStart;
							return {intermediate_rep::Token::LessEqual};
						}
						else
						{
							return {intermediate_rep::Token::Less};
						}
					}
					case '>':
					{
						++lexemStart;
						if(*lexemStart == '=')
						{
							++lexemStart;
							return {intermediate_rep::Token::GreaterEqual};
						}
						else
						{
							return {intermediate_rep::Token::Greater};
						}
					}
					default:
						throw std::runtime_error("Unexpected Token");
				}
			}

		}
		else
		{
			return {intermediate_rep::Token::Eof};
		}
	}
}

