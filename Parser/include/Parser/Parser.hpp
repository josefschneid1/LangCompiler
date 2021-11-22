#ifndef parser_hpp
#define parser_hpp

#include "Token/Token.hpp"
#include "Ast/Ast.hpp"
#include "Ast/SymbolTable.hpp"
#include <memory>
#include <stdexcept>
#include <utility>
#include <map>

namespace Parser
{

	namespace ast = intermediate_rep::ast;

	enum class Associativity
	{
		Left,Right
	};

	const std::map<ast::BinaryOperator, std::pair<int, Associativity>> operatorPrecedence
	{
		{ast::BinaryOperator::Assign, {0, Associativity::Right}},
		{ast::BinaryOperator::Or, {10, Associativity::Left}},
		{ast::BinaryOperator::And, {20, Associativity::Left}},
		{ast::BinaryOperator::Equal, {30, Associativity::Left}},
		{ast::BinaryOperator::NotEqual, {30, Associativity::Left}},
		{ast::BinaryOperator::Less, {40, Associativity::Left}},
		{ast::BinaryOperator::LessEqual, {40, Associativity::Left}},
		{ast::BinaryOperator::Greater, {40, Associativity::Left}},
		{ast::BinaryOperator::GreaterEqual, {40, Associativity::Left}},
		{ast::BinaryOperator::Add, {50, Associativity::Left}},
		{ast::BinaryOperator::Sub, {50, Associativity::Left}},
		{ast::BinaryOperator::Mul, {60, Associativity::Left}},
		{ast::BinaryOperator::Div, {60, Associativity::Left}},
	};

	class Parser
	{
	public:

		Parser(intermediate_rep::TokenProducer* tokenProducer);

		auto program() -> ast::Program;

	private:

		auto function() -> ast::Function;

		// Statements

		auto stmt() -> std::unique_ptr<ast::Statement>;

		auto exprStmt() -> std::unique_ptr<ast::ExprStmt>;

		auto ifStmt() -> std::unique_ptr<ast::IfStmt> ;

		auto whileStmt() -> std::unique_ptr<ast::WhileStmt>;

		auto returnStmt() -> std::unique_ptr<ast::ReturnStmt>;

		auto block() -> std::unique_ptr<ast::Block>;

		auto varDecl() -> std::unique_ptr<ast::ExprStmt>;

		// Expressions

		auto expr() -> std::unique_ptr<ast::Expression>;

		auto binaryExpr() -> std::unique_ptr<ast::Expression>;

		auto binaryExpr_h(std::unique_ptr<ast::Expression> left, int min_precedence) -> std::unique_ptr<ast::Expression>;

		auto unaryExpr() -> std::unique_ptr<ast::Expression>;

		auto primaryExpr() -> std::unique_ptr<ast::Expression>;


		// Check if the next token has any of the following types
		template<std::same_as<intermediate_rep::Token::Type> ... TArgs>
		bool match(intermediate_rep::Token::Type type, TArgs ... types);

		// Check if the token has a specific type, advance the stream, repeat, return last token
		template<std::same_as<intermediate_rep::Token::Type> ... TArgs>
		intermediate_rep::Token consume(intermediate_rep::Token::Type type, TArgs ... types);

		// advance the stream unconditionally
		void advance();

		intermediate_rep::TokenProducer* tokenProducer;
		std::unique_ptr<intermediate_rep::SymbolTable> globals;
		intermediate_rep::SymbolTableBuilder builder;
		intermediate_rep::Token next;
	};



	template<std::same_as<intermediate_rep::Token::Type> ... TArgs>
	intermediate_rep::Token Parser::consume(intermediate_rep::Token::Type type, TArgs ... types)
	{
		if(next.type != type)
			throw std::runtime_error("Expected " + intermediate_rep::tokenTypeToStr(type) + " but got (" + intermediate_rep::tokenTypeToStr(next.type) + ", " +next.lexem  + ")");

		if constexpr(sizeof...(types) > 0)
		{
			advance();
			return consume(types...);
		}
		else
		{
			return std::exchange(next, tokenProducer->next());
		}
	}	

	template<std::same_as<intermediate_rep::Token::Type> ... TArgs>
	bool Parser::match(intermediate_rep::Token::Type type, TArgs ... types)
	{
		if(next.type == type)
			return true;

		if constexpr(sizeof...(types) > 0)
		{
			return match(types...);
		}
		else
		{
			return false;
		}
	}

}

#endif