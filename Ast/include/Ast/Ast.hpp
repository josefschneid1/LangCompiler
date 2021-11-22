#ifndef ast_hpp
#define ast_hpp

#include <memory>
#include <vector>
#include <string>
#include <ostream>
#include <string_view>
#include "SymbolTable.hpp"
#include <concepts>
#include <stdexcept>

namespace intermediate_rep::ast
{

	enum class BinaryOperator
	{
		Add,
		Sub,
		Mul,
		Div,

		Less,
		LessEqual,
		Greater,
		GreaterEqual,

		Equal,
		NotEqual,

		Or,
		And,

		Assign,
	};

	std::string_view binOpToStr(BinaryOperator op);

	enum class UnaryOperator
	{
		Not,
		Negate,
	};

	std::string_view unOpToStr(UnaryOperator op);

	template<typename T, typename ... TArgs>
	struct ExprVisitor;

	struct Expression
	{
		SymbolTable::Variable::Type type;
		Expression(SymbolTable::Variable::Type type);
		virtual void accept(ExprVisitor<void>&) = 0;
		virtual std::string accept(ExprVisitor<std::string,std::string>&, std::string) = 0;
		virtual ~Expression(){}
	};

	struct BinaryExpression : Expression
	{
		std::unique_ptr<Expression> left,right;
		BinaryOperator op;

		BinaryExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, BinaryOperator op);

		void accept(ExprVisitor<void>&) override;
		std::string accept(ExprVisitor<std::string,std::string>&, std::string) override;
	};

	struct UnaryExpression : Expression
	{
		std::unique_ptr<Expression> expr;
		UnaryOperator op;
		UnaryExpression(std::unique_ptr<Expression> expr, UnaryOperator op);
		void accept(ExprVisitor<void>&) override;
		std::string accept(ExprVisitor<std::string,std::string>&, std::string) override;
	};

	struct Variable : Expression
	{
		SymbolTable::Variable* sym_entry;
		Variable(SymbolTable::Variable* sym_entry);
		void accept(ExprVisitor<void>&) override;
		std::string accept(ExprVisitor<std::string,std::string>&, std::string) override;
	};

	struct Call : Expression
	{
		SymbolTable::Function* sym_entry;
		std::vector<std::unique_ptr<Expression>> args;
		Call(SymbolTable::Function* sym_entry, std::vector<std::unique_ptr<Expression>> args);
		void accept(ExprVisitor<void>&) override;
		std::string accept(ExprVisitor<std::string,std::string>&, std::string) override;
	};

	template<typename T>
	struct Constant;

	template<typename T, typename ... TArgs>
	struct ExprVisitor
	{
		virtual T visit(BinaryExpression&, TArgs ...) = 0;
		virtual T visit(UnaryExpression&, TArgs ...) = 0;
		virtual T visit(Variable&,TArgs ...) = 0;
		virtual T visit(Constant<int>&,TArgs ...) = 0;
		virtual T visit(Constant<double>&,TArgs ...) = 0;
		virtual T visit(Constant<bool>&,TArgs ...) = 0;
		virtual T visit(Constant<std::string>&,TArgs ...) = 0;
		virtual T visit(Call&, TArgs ...) = 0;

		virtual ~ExprVisitor(){}
	};
		
	template<typename T>
	struct Constant : Expression
	{
		T value;

		Constant(T value):
			Expression([](){
				if constexpr(std::same_as<int, T>)
				{
					return SymbolTable::Variable::Type::Int;
				} 
				else if constexpr(std::same_as<double,T>)
				{
					return SymbolTable::Variable::Type::Float;
				}
				else if constexpr(std::same_as<bool,T>)
				{
					return SymbolTable::Variable::Type::Bool;
				}
				else if constexpr(std::same_as<std::string,T>)
				{
					return SymbolTable::Variable::Type::Str;
				}
				else
				{
					throw std::runtime_error("Unkown Constant Type");
				}
								
			}()), value{std::move(value)}
		{}
 
		void accept(ExprVisitor<void>& v) override
		{
			v.visit(*this);
		}
		std::string accept(ExprVisitor<std::string,std::string>& v, std::string arg) override
		{
			return v.visit(*this, arg);
		}

	};

	template<typename T, typename ... TArgs>
	struct StmtVisitor;

	struct Statement
	{
		virtual void accept(StmtVisitor<void>&) = 0;
		virtual std::string accept(StmtVisitor<std::string, std::string>&, std::string) = 0;
		virtual ~Statement(){}
	};

	struct ExprStmt : Statement
	{
		std::unique_ptr<Expression> expr;
		ExprStmt(std::unique_ptr<Expression> expr);
		void accept(StmtVisitor<void>&) override;
		std::string accept(StmtVisitor<std::string, std::string>&, std::string) override;
	};

	struct IfStmt : Statement
	{
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Statement> trueStmt;
		std::unique_ptr<Statement> falseStmt;
		IfStmt(std::unique_ptr<Expression> condition,std::unique_ptr<Statement> trueStmt,std::unique_ptr<Statement> falseStmt = nullptr);
		void accept(StmtVisitor<void>&) override;
		std::string accept(StmtVisitor<std::string, std::string>&, std::string) override;
	};

	struct WhileStmt : Statement
	{
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Statement> stmt;
		WhileStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> stmt);
		void accept(StmtVisitor<void>&) override;
		std::string accept(StmtVisitor<std::string, std::string>&, std::string) override;
	};

	struct ReturnStmt : Statement
	{
		std::unique_ptr<Expression> expr;
		ReturnStmt(std::unique_ptr<Expression> expr = nullptr);
		void accept(StmtVisitor<void>&) override;
		std::string accept(StmtVisitor<std::string, std::string>&, std::string) override;
	};

	struct Block : Statement
	{
		std::vector<std::unique_ptr<Statement>> stmts;
		void accept(StmtVisitor<void>&) override;
		std::string accept(StmtVisitor<std::string, std::string>&, std::string) override;
	};

	template<typename T, typename ... TArgs>
	struct StmtVisitor
	{
		virtual T visit(ExprStmt&, TArgs ...) = 0;
		virtual T visit(IfStmt&, TArgs ...) = 0;
		virtual T visit(WhileStmt&, TArgs ...) = 0;
		virtual T visit(ReturnStmt&, TArgs ...) = 0;
		virtual T visit(Block&, TArgs ...) = 0;
		virtual ~StmtVisitor(){} 
	};

	struct Function
	{
		SymbolTable::Function* sym_entry;
		std::unique_ptr<Block> block;
	};
	
	struct Program
	{
		std::unique_ptr<SymbolTable> root;
		std::vector<Function> functions;
	};


	std::ostream& operator<<(std::ostream& os, const Program& program);
	std::ostream& operator<<(std::ostream& os, const Function& function);

}

#endif