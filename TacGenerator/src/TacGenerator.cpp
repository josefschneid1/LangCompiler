#include "TacGenerator.hpp"
#include <string>
#include <functional>
#include <stack>
#include <stdexcept>

namespace tac_gen
{
	namespace tac = intermediate_rep::tac;

	namespace ast = intermediate_rep::ast;

	tac::InstructionType binaryOpToInstruction(ast::BinaryOperator op)
	{
		using enum tac::InstructionType;

		switch(op)
		{
			case ast::BinaryOperator::Add:
				return Add;
			case ast::BinaryOperator::Sub:
				return Sub;
			case ast::BinaryOperator::Mul:
				return Mul;
			case ast::BinaryOperator::Div:
				return Div;
			case ast::BinaryOperator::Less:
				return Less;
			case ast::BinaryOperator::LessEqual:
				return LessEqual;
			case ast::BinaryOperator::Greater:
				return Greater;
			case ast::BinaryOperator::GreaterEqual:
				return GreaterEqual;
			case ast::BinaryOperator::Equal:
				return Equal;
			case ast::BinaryOperator::NotEqual:
				return NotEqual;
			case ast::BinaryOperator::Assign:
				return Assign;
			case ast::BinaryOperator::And:
				return And;
			case ast::BinaryOperator::Or:
				return Or;
			default:
				throw std::runtime_error("Missing Case statement");
		}
	}

	tac::InstructionType unaryOpToInstruction(ast::UnaryOperator op)
	{
		using enum tac::InstructionType;

		switch(op)
		{
			case ast::UnaryOperator::Not:
				return Not;
			case ast::UnaryOperator::Negate:
				return Negate;
			default:
				throw std::runtime_error("Missing Case statement");
		}

	}

	TacGenerator::TacGenerator(ast::Program* ast):
		ast{ast}
	{}

	auto TacGenerator::gen() -> std::vector<tac::Function>
	{

		class NameGenerator
		{
		public:

			NameGenerator(const std::string& prefix):
				prefix{prefix}
			{}

			std::string getUniqueLabel()
			{
				return prefix + std::to_string(n++);
			}
		private:
			int n = 0;
			std::string prefix;
		};

		struct Visitor : ast::StmtVisitor<std::string, std::string>, ast::ExprVisitor<std::string, std::string>
		{

			NameGenerator& labelGen;
			NameGenerator& varNameGen;
			intermediate_rep::SymbolTable* sym_table;
			std::vector<tac::Quadruple> tac;
			tac::Address address;

			Visitor(NameGenerator& labelGen,NameGenerator& varNameGen, intermediate_rep::SymbolTable* sym_table):
				labelGen{labelGen}, varNameGen{varNameGen}, sym_table{sym_table}
			{}

			std::string visit(ast::ExprStmt& stmt, std::string label)
			{
				label = stmt.expr->accept(*this, label);
				return label;
			}

			std::string visit(ast::IfStmt& stmt, std::string label)
			{	
				stmt.condition->accept(*this, label);
				auto afterLabel = labelGen.getUniqueLabel();

				if(stmt.falseStmt)
				{
					auto falseStmtLabel = labelGen.getUniqueLabel();
					tac.push_back(tac::Quadruple{"",tac::InstructionType::IfFalseJump, address, falseStmtLabel});
					stmt.trueStmt->accept(*this, "");
					tac.push_back(tac::Quadruple{"",tac::InstructionType::Jump, afterLabel});
					stmt.falseStmt->accept(*this, falseStmtLabel);
				}
				else
				{
					tac.push_back(tac::Quadruple{"", tac::InstructionType::IfFalseJump, address, afterLabel});
					stmt.trueStmt->accept(*this, "");
				}
				return afterLabel;
			}

			std::string visit(ast::WhileStmt& stmt, std::string label)
			{	
				label = label != "" ? label : labelGen.getUniqueLabel();
				auto afterLabel = labelGen.getUniqueLabel();
				stmt.condition->accept(*this, label);
				tac.push_back(tac::Quadruple{"", tac::InstructionType::IfFalseJump, address, afterLabel});
				stmt.stmt->accept(*this,"");
				tac.push_back(tac::Quadruple{"", tac::InstructionType::Jump, label});
				return afterLabel;
			}

			std::string visit(ast::ReturnStmt& stmt, std::string label) 
			{
				if(stmt.expr)
				{
					label = stmt.expr->accept(*this, label);
				}
				tac.push_back(tac::Quadruple{ label, tac::InstructionType::Return,std::monostate{},stmt.expr ?
					 address: tac::Address{std::monostate{}}});

				return "";
			}

			std::string visit(ast::Block& block, std::string label) 
			//Could be the block of a function definition block or an enclosing block
			{
				if(block.stmts.size() != 0)
				{
					for(auto& stmt : block.stmts)
					{
						label = stmt->accept(*this, label);
					}
				}
				return label;
			}

			intermediate_rep::SymbolTable::Variable* newTemp(intermediate_rep::SymbolTable::Variable::Type type)
			{
				auto name = varNameGen.getUniqueLabel();
				return &sym_table->insert(name, intermediate_rep::SymbolTable::Variable{name, type});
			}

			std::string visit(ast::BinaryExpression& expr, std::string label)
			{	
				label = expr.left->accept(*this, label);
				auto left = address;
				label = expr.right->accept(*this, label);
				auto right = address;

				if(expr.op == ast::BinaryOperator::Assign)
				{
					address = left;
					tac.push_back({label, binaryOpToInstruction(expr.op), left, right});
				}
				else
				{				
					auto varPtr = newTemp(expr.type);
					address = varPtr;
					tac.push_back({label, binaryOpToInstruction(expr.op), varPtr, left, right});
				}

				return {};
			}

			std::string visit(ast::UnaryExpression& expr, std::string label) 
			{
				label = expr.expr->accept(*this, label);
				auto operand = address;
				auto varPtr = newTemp(expr.type);
				address = varPtr;
				tac.push_back({label, unaryOpToInstruction(expr.op), varPtr, operand});
				return {};
			}

			std::string visit(ast::Call& call, std::string label) 
			{
				for(auto& arg : call.args)
				{
					label = arg->accept(*this, label);
					tac.push_back({ label, tac::InstructionType::Param, std::monostate{}, address });
				}
				auto varPtr = newTemp(call.sym_entry->returnType);
				address = varPtr;
				tac.push_back(tac::Quadruple{label, tac::InstructionType::Call, varPtr, call.sym_entry, tac::CallArgNum{call.args.size()}});
				return {};
			}

			std::string visit(ast::Variable& var, std::string label) 
			{
				address = var.sym_entry;
				return label;
			}

			std::string visit(ast::Constant<int>& c, std::string label) 
			{
				address = tac::Constant<int>{c.value};
				return label;
			}
			
			std::string visit(ast::Constant<double>& c, std::string label) 
			{
				address = tac::Constant<double>{c.value};
				return label;
			}

			std::string visit(ast::Constant<bool>& c, std::string label)
			{
				address = tac::Constant<bool>{c.value};
				return label;
			}

			std::string visit(ast::Constant<std::string>&, std::string label) 
			{
				throw std::runtime_error("");
			}	

		};

		NameGenerator labelGen{"__label"};
		NameGenerator varNameGen{"__temp"};

		std::vector<tac::Function> functions;

		for(auto& function : ast->functions)
		{
			Visitor visitor{labelGen, varNameGen, &function.sym_entry->parameter_scope->getChild(0)};
			function.block->accept(visitor, "");
			functions.push_back(tac::Function{function.sym_entry, std::move(visitor.tac)});
		}

		return functions;

	}

}