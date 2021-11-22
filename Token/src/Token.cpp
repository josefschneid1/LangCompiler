#include "Token.hpp"
#include <stdexcept>

namespace intermediate_rep
{
	std::string tokenTypeToStr(Token::Type type)
	{
		using enum Token::Type;

		switch(type)
		{
			case True:
				return "True";
			case False:
				return "False";
			case While:
				return "While";
			case If:
				return "If";
			case Else:
				return "Else";
			case Return:
				return "Return";
			case Id:
				return "Id";
			case IntLit:
				return "IntLit";
			case FloatLit:
				return "FloatLit";
			case StrLit:
				return "StrLit";
			case OParen:
				return "OParen";
			case CParen:
				return "CParen";
			case OSBracket:
				return "OSBracket";
			case CSBracket:
				return "CSBracket";
			case OCBracket:
				return "OCBracket";
			case CCBracket:
				return "CCBracket";
			case Comma:
				return "Comma";
			case Semicolon:
				return "Semicolon";
			case Plus:
				return "Plus";
			case Minus:
				return "Minus";
			case Star:
				return "Star";
			case Slash:
				return "Slash";
			case Less:
				return "Less";
			case LessEqual:
				return "LessEqual";
			case Greater:
				return "Greater";
			case GreaterEqual:
				return "GreaterEqual";
			case Equal:
				return "Equal";
			case NotEqual:
				return "NotEqual";
			case Assign:
				return "Assign";
			case And:
				return "And";
			case Or:
				return "Or";
			case Not:
				return "Not";
			case Eof:
				return "Eof";
			default:
				throw std::runtime_error("Missing case stmt");
		}

	}

	std::ostream& operator<<(std::ostream& os, const Token& t)
	{
		return os << "(" << tokenTypeToStr(t.type) << ", " << t.lexem + ")";
	}


}

