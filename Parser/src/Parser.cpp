#include "Parser.hpp"
#include <vector>
#include <map>
#include <string_view>
#include <optional>
#include <utility>

namespace Parser
{

	using namespace intermediate_rep;


	const std::map<std::string_view, SymbolTable::Variable::Type> strToVarType
	{
		{"int", SymbolTable::Variable::Type::Int},
		{"float", SymbolTable::Variable::Type::Float},
		{"str", SymbolTable::Variable::Type::Str},
		{"bool", SymbolTable::Variable::Type::Bool},
	};


	ast::BinaryOperator tokenToBinOp(Token::Type type)
	{
		using enum ast::BinaryOperator;
		switch(type)
		{
			case Token::Type::Plus:
				return Add;
			case Token::Type::Minus:
				return Sub;
			case Token::Type::Star:
				return Mul;
			case Token::Type::Slash:
				return Div;
			case Token::Type::Less:
				return Less;
			case Token::Type::Greater:
				return Greater;
			case Token::Type::LessEqual:
				return LessEqual;
			case Token::Type::GreaterEqual:
				return GreaterEqual;
			case Token::Type::Equal:
				return Equal;
			case Token::Type::NotEqual:
				return NotEqual;
			case Token::Type::And:
				return And;
			case Token::Type::Or:
				return Or;
			case Token::Type::Assign:
				return Assign;
			default:
				throw std::runtime_error("Token is not a binary Operator");
		}
	}

	bool isBinaryOperator(Token::Type type)
	{
		using enum Token::Type;
		switch(type)
		{
			case Plus:
			case Minus:
			case Star:
			case Slash:
			case Less:
			case LessEqual:
			case Greater:
			case GreaterEqual:
			case Equal:
			case NotEqual:
			case Assign:
			case And:
			case Or:
				return true;
			default:
				return false;
		}
	}

	Parser::Parser(TokenProducer* tokenProducer):
		tokenProducer{tokenProducer}, globals{std::make_unique<SymbolTable>()}, builder{globals.get()}
	{
		next = tokenProducer->next();
	}

	void Parser::advance()
	{
		next = tokenProducer->next();
	}

	auto Parser::program() -> ast::Program
	{
		std::vector<ast::Function> functions;

		while(!match(Token::Type::Eof))
		{
			functions.push_back(function());
		}

		// Parser left in invalid state; Maybe fix it later?
		return {std::move(globals), std::move(functions)};
	}


	// ReturnType Name (Type name, ....){}
	auto Parser::function() -> ast::Function
	{
		// Create Parameter Scope
		auto returnType = consume(Token::Type::Id);
		auto name = consume(Token::Type::Id);

		auto& func = builder.top()->insert(name.lexem, SymbolTable::Function{name.lexem, strToVarType.at(returnType.lexem)});
		ScopeGuard guard{builder};
		func.parameter_scope = builder.top();
		consume(Token::Type::OParen);
		while(match(Token::Type::Id))
		{
			auto type = consume(Token::Type::Id);
			auto name = consume(Token::Type::Id);

			func.parameters.push_back(&builder.top()->insert(name.lexem, SymbolTable::Variable{
				name.lexem,
				strToVarType.at(type.lexem),
			}));

			if(match(Token::Type::Comma))
			{
				advance();
			}
		}
		consume(Token::Type::CParen);
		return {&func, block()};
	}

	auto Parser::stmt() -> std::unique_ptr<ast::Statement>
	{
		switch(next.type)
		{
			case Token::Type::While:
				return whileStmt();
			case Token::Type::If:
				return ifStmt();
			case Token::Type::Return:
				return returnStmt();
			case Token::Type::OCBracket:
				return block();
			case Token::Type::Id:
			{
				if(strToVarType.contains(next.lexem))
				{
					return varDecl();
				}
			}
			default:
				return exprStmt();
		}
	} 

	auto Parser::whileStmt() -> std::unique_ptr<ast::WhileStmt>
	{
		consume(Token::Type::While, Token::Type::OParen);
		auto condition = expr();
		consume(Token::Type::CParen);
		auto statement = stmt();
		return std::make_unique<ast::WhileStmt>(std::move(condition), std::move(statement));
	}

	auto Parser::ifStmt() -> std::unique_ptr<ast::IfStmt>
	{
		consume(Token::Type::If, Token::Type::OParen);
		auto condition = expr();
		consume(Token::Type::CParen);
		auto trueStmt = stmt();
		decltype(trueStmt) falseStmt;
		if(match(Token::Type::Else))
		{
			advance();
			falseStmt = stmt();
		}
		return std::make_unique<ast::IfStmt>(std::move(condition), std::move(trueStmt), std::move(falseStmt));
	}

	auto Parser::returnStmt() -> std::unique_ptr<ast::ReturnStmt>
	{
		consume(Token::Type::Return);
		if(match(Token::Type::Semicolon))
		{
			advance();
			return std::make_unique<ast::ReturnStmt>();
		}
		else
		{
			auto expression = expr();
			consume(Token::Type::Semicolon);
			return std::make_unique<ast::ReturnStmt>(std::move(expression));
		}
	}

	auto Parser::exprStmt() -> std::unique_ptr<ast::ExprStmt>
	{
		auto expression = expr();
		consume(Token::Type::Semicolon);
		return std::make_unique<ast::ExprStmt>(std::move(expression));
	}

	auto Parser::varDecl() -> std::unique_ptr<ast::ExprStmt>
	{
		auto type = consume(Token::Type::Id);
		auto name = consume(Token::Type::Id);

		auto& var = builder.top()->insert(name.lexem, SymbolTable::Variable{
			name.lexem,
			strToVarType.at(type.lexem)
		});

		consume(Token::Type::Assign);
		auto expression = expr();
		consume(Token::Type::Semicolon);
		return std::make_unique<ast::ExprStmt>(
			std::make_unique<ast::BinaryExpression>(
				std::make_unique<ast::Variable>(&var),
				std::move(expression),
				ast::BinaryOperator::Assign
			)
		);
	}

	auto Parser::block() -> std::unique_ptr<ast::Block>
	{
		ScopeGuard guard{builder};
		consume(Token::Type::OCBracket);
		ast::Block blck;
		
		while(!match(Token::Type::CCBracket))
		{
			blck.stmts.push_back(stmt());
		}
		consume(Token::Type::CCBracket);
		return std::make_unique<ast::Block>(std::move(blck));
	}

	auto Parser::unaryExpr() -> std::unique_ptr<ast::Expression>
	{
		if(match(Token::Type::Not))
		{
			advance();
			return std::make_unique<ast::UnaryExpression>(primaryExpr(), ast::UnaryOperator::Not);
		}
		else
		{
			return primaryExpr();
		}
	}

	auto Parser::primaryExpr() -> std::unique_ptr<ast::Expression>
	{
		switch(next.type)
		{
			case Token::Type::OParen:
			{
				advance();
				auto expression = expr();
				consume(Token::Type::CParen);
				return std::move(expression);
			}
			case Token::Type::IntLit:
			{
				int n = std::stoi(next.lexem);
				advance();
				return std::make_unique<ast::Constant<int>>(n);
			}
			case Token::Type::FloatLit:
			{
				int n = std::stof(next.lexem);
				advance();
				return std::make_unique<ast::Constant<double>>(n);
			}
			case Token::Type::StrLit:
			{
				auto str = std::move(next.lexem);
				advance();
				return std::make_unique<ast::Constant<std::string>>(std::move(str));
			}
			case Token::Type::Id:
			{
				//Check if function call or variable
				auto name = consume(Token::Type::Id).lexem;

				if(match(Token::Type::OParen))
				{
					advance();
					std::vector<std::unique_ptr<ast::Expression>> arguments;
					while(!match(Token::Type::CParen))
					{
						arguments.push_back(expr());
						if(match(Token::Type::Comma))
						{
							advance();
						}
					}	
					consume(Token::Type::CParen);

					return std::make_unique<ast::Call>(&builder.top()->get<SymbolTable::Function>(name), std::move(arguments));
				}
				else
				{
					return std::make_unique<ast::Variable>(&builder.top()->get<SymbolTable::Variable>(name));
				}
			}

			case Token::Type::True:
			{
				advance();
				return std::make_unique<ast::Constant<bool>>(true);
			}

			case Token::Type::False:
			{
				advance();
				return std::make_unique<ast::Constant<bool>>(false);
			}
			default:
				throw std::runtime_error("Unexpected Token");
		}
	}

	auto Parser::expr() -> std::unique_ptr<ast::Expression>
	{
		return binaryExpr();
	}

	auto Parser::binaryExpr() -> std::unique_ptr<ast::Expression>
	{
		return binaryExpr_h(primaryExpr(), 0);
	}

	auto Parser::binaryExpr_h(std::unique_ptr<ast::Expression> left, int min_precedence) -> std::unique_ptr<ast::Expression>
	{
		while(isBinaryOperator(next.type))
		{
			auto op_left = tokenToBinOp(next.type);
			auto prec_left = operatorPrecedence.at(op_left).first;
			if(prec_left >= min_precedence)
			{
				advance();
				auto right = primaryExpr();
				while(isBinaryOperator(next.type))
				{
					auto op_right = tokenToBinOp(next.type);
					auto [prec_right, assoc] = operatorPrecedence.at(op_right);
					if( prec_right > prec_left || assoc == Associativity::Right)
					{
						right = binaryExpr_h(std::move(right), prec_left + 1);
					}
					else
					{
						break;
					}
				}
				left = std::make_unique<ast::BinaryExpression>(std::move(left), std::move(right), op_left);
			}
			else
			{
				break;
			}
		}
		return std::move(left);
	}
}