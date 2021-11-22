#include "Ast.hpp"
#include <stdexcept>
#include <iostream>
#include <ios>

#define ACCEPT(Class, Visitor) void Class::accept(Visitor<void> & v){v.visit(*this);}

namespace intermediate_rep::ast
{
	ACCEPT(BinaryExpression, ExprVisitor)
	ACCEPT(UnaryExpression, ExprVisitor)
	ACCEPT(Variable, ExprVisitor)
	ACCEPT(Call, ExprVisitor)

	ACCEPT(ExprStmt, StmtVisitor)
	ACCEPT(IfStmt, StmtVisitor)
	ACCEPT(WhileStmt, StmtVisitor)
	ACCEPT(ReturnStmt, StmtVisitor)
	ACCEPT(Block, StmtVisitor)

	std::string ExprStmt::accept(StmtVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string IfStmt::accept(StmtVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string WhileStmt::accept(StmtVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string ReturnStmt::accept(StmtVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string Block::accept(StmtVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}

	std::string BinaryExpression::accept(ExprVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string UnaryExpression::accept(ExprVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string Variable::accept(ExprVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}
	std::string Call::accept(ExprVisitor<std::string,std::string> & v, std::string arg){return v.visit(*this,arg);}

	Expression::Expression(SymbolTable::Variable::Type type):
		type{type}
	{}

	BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, BinaryOperator op):
		Expression([op,&left](){
			using enum intermediate_rep::ast::BinaryOperator;
			switch(op)
			{
			case Add:
			case Sub:
			case Mul:
			case Div:
			case Assign:
				return left->type;
			case Less:
			case LessEqual:
			case Greater:
			case GreaterEqual:
			case Equal:
			case NotEqual:
			case Or:
			case And:
				return intermediate_rep::SymbolTable::Variable::Type::Bool;
			default:
				throw std::runtime_error("Unkown Binary Operator Missing Case Stmt");
			}

		}()),left{std::move(left)}, right{std::move(right)}, op{op}
	{}

	UnaryExpression::UnaryExpression(std::unique_ptr<Expression> expr, UnaryOperator op):
		Expression(expr->type),
		expr{std::move(expr)}, op{op}
	{}

	Variable::Variable(SymbolTable::Variable* sym_entry):
		Expression(sym_entry->type),
		sym_entry{sym_entry}
	{}

	Call::Call(SymbolTable::Function* sym_entry, std::vector<std::unique_ptr<Expression>> args):
		Expression(sym_entry->returnType),
		sym_entry{sym_entry}, args{std::move(args)}
	{}

	WhileStmt::WhileStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> stmt):
		condition{std::move(condition)}, stmt{std::move(stmt)}
	{}

	IfStmt::IfStmt(std::unique_ptr<Expression> condition,std::unique_ptr<Statement> trueStmt,std::unique_ptr<Statement> falseStmt):
		condition{std::move(condition)}, trueStmt{std::move(trueStmt)}, falseStmt{std::move(falseStmt)}
	{}

	ExprStmt::ExprStmt(std::unique_ptr<Expression> expr):
		expr{std::move(expr)}
		{}

	ReturnStmt::ReturnStmt(std::unique_ptr<Expression> expr):
		expr{std::move(expr)}
	{}

	std::string_view binOpToStr(BinaryOperator op)
	{
		switch(op)
		{
			case BinaryOperator::Add:
				return "Add";

			case BinaryOperator::Sub:
				return "Sub";

			case BinaryOperator::Mul:
				return "Mul";

			case BinaryOperator::Div:
				return "Div";

			case BinaryOperator::Less:
				return "Less";

			case BinaryOperator::LessEqual:
				return "LessEqual";

			case BinaryOperator::Greater:
				return "Greater";

			case BinaryOperator::GreaterEqual:
				return "GreaterEqual";

			case BinaryOperator::Equal:
				return "Equal";

			case BinaryOperator::NotEqual:
				return "NotEqual";

			case BinaryOperator::Or:
				return "Or";

			case BinaryOperator::And:
				return "And";

			case BinaryOperator::Assign:
				return "Assign";
			default:
				throw std::runtime_error("Missing Case Stmt");
		}
	}

	std::string_view unOpToStr(UnaryOperator op)
	{
		switch(op)
		{
			case UnaryOperator::Not:
				return "Not";
			case UnaryOperator::Negate:
				return "Negate";
			default:
				throw std::runtime_error("Missing Case Stmt");
		}
	}


	struct Visitor : ExprVisitor<void>, StmtVisitor<void>
	{
		std::ostream& os;
		int indentation_level;
		int offset;
		Visitor(std::ostream& os, int indentation_level = 0, int offset = 15):
			os{os}, indentation_level{indentation_level}, offset{offset}
		{}

		void drawIndent(int n)
		{

			for(int i = 0; i < n - offset; ++i)
			{
				os << ' ';
			}
			os << "|";
			for(int i = 1; i < offset; ++i)
			{
				os << "_";
			}

		}

		void visit(ExprStmt& stmt) override
		{
			stmt.expr->accept(*this);
		}

		void visit(IfStmt& stmt) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << "If\n";	
			indentation_level = indent + offset;
			stmt.condition->accept(*this);
			drawIndent(indent + offset);
			os << "Then\n";
			indentation_level = indent + 2 * offset;
			stmt.trueStmt->accept(*this);
			if(stmt.falseStmt)
			{
				drawIndent(indent + offset);
				os << "Else\n";
				indentation_level = indent + 2 * offset;
				stmt.falseStmt->accept(*this);
			}

		}
		void visit(WhileStmt& stmt) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << "While\n";
			indentation_level = indent + offset;
			stmt.condition->accept(*this);
			drawIndent(indent);
			os << "Then\n";
			indentation_level = indent + offset;
			stmt.stmt->accept(*this);
		}

		void visit(Block& block) override
		{
			auto indent = indentation_level;
			for(auto& stmt : block.stmts)
			{
				stmt->accept(*this);
				indentation_level = indent;
			}
		}

		void visit(ReturnStmt& stmt) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << "Return\n";
			if(stmt.expr)
			{
				indentation_level = indent + 15;
				stmt.expr->accept(*this);
			}

		}

		void visit(BinaryExpression& expr) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << binOpToStr(expr.op) << '\n';
			indentation_level = indent + offset;
			expr.left->accept(*this);
			indentation_level = indent + offset;
			expr.right->accept(*this);
		}

		void visit(UnaryExpression& expr) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << unOpToStr(expr.op) << '\n';
			indentation_level = indent + offset;
			expr.expr->accept(*this);
		}

		void visit(Variable& v) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << v.sym_entry->name << '\n';
		}

		void visit(Call& call) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << "Call " <<call.sym_entry->name << '\n';
			for(auto& expr : call.args)
			{
				indentation_level = indent + offset;
				expr->accept(*this);
			}
		}

		void visit(Constant<int>& c) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << c.value << '\n';
		}

		void visit(Constant<double>& c) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << c.value << '\n';
		}

		void visit(Constant<bool>& c) override
		{
			std::cout << std::boolalpha;
			auto indent = indentation_level;
			drawIndent(indent);
			os << c.value << '\n';
		}

		void visit(Constant<std::string>& c) override
		{
			auto indent = indentation_level;
			drawIndent(indent);
			os << c.value << '\n';
		}
	};

	std::ostream& operator<<(std::ostream& os, const Program& program)
	{
		Visitor visitor{os, 15};
		for(auto& function : program.functions)
		{
			os << "Function " << function << '\n';
			function.block->accept(visitor);
			visitor.indentation_level = 15;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Function& function)
	{
		os << function.sym_entry->name << '\n';

		return os;
	}
}

#undef ACCEPT