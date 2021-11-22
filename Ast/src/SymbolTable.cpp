#include "SymbolTable.hpp"
#include <stdexcept>
#include <iostream>

namespace intermediate_rep
{
	SymbolTable::SymbolTable(SymbolTable* parent):
		parent{parent}
	{
	}

	SymbolTable::Symbol& SymbolTable::operator[](const std::string& key)
	{
		if(table.contains(key))
		{
			return table[key];
		}
		else if(parent != nullptr)
		{
			return (*parent)[key];
		}
		else
		{
			throw std::runtime_error("Symbol not found");
		}
	}

	SymbolTable::Symbol& SymbolTable::insert(const std::string& key, SymbolTable::Symbol v)
	{
		if(table.contains(key))
		{
			throw std::runtime_error("Symbol already defined");
		}
		else
		{
			return table[key] = v;
		}
	}

	SymbolTable& SymbolTable::addChild(std::unique_ptr<SymbolTable> child)
	{
		child->parent = this;
		children.push_back(std::move(child));
		return *children.back();
	}

	RecursiveSymTableIterator SymbolTable::begin()
	{
		return RecursiveSymTableIterator{this};
	}

	RecursiveSymTableIterator SymbolTable::end()
	{
		return RecursiveSymTableIterator{};
	}

	std::size_t SymbolTable::numChildren()
	{
		return table.size();
	}

	SymbolTable& SymbolTable::getChild(std::size_t indx)
	{
		if(indx >= children.size())
			throw std::runtime_error("Out of bounds");
		return *children[indx];
	}

	SymbolTableBuilder::SymbolTableBuilder(SymbolTable* root)
	{
		path.push(root);
	}

	void SymbolTableBuilder::push()
	{
		path.push(&path.top()->addChild(std::make_unique<SymbolTable>()));
	}

	void SymbolTableBuilder::pop()
	{
		if(path.size() <= 1)
		{
			throw std::runtime_error("Cannot remove root node");
		}
		path.pop();
	}

	SymbolTable* SymbolTableBuilder::top()
	{
		return path.top();
	}

	ScopeGuard::ScopeGuard(SymbolTableBuilder& builder):
		builder{builder}
	{
		builder.push();
	}

	ScopeGuard::~ScopeGuard()
	{
		builder.pop();
	}

	RecursiveSymTableIterator::RecursiveSymTableIterator(SymbolTable* sym_table):
		begin{sym_table->table.begin()}, end{sym_table->table.end()}
	{
		if(begin != end)
		{
			stack.push(sym_table);
		}
	}

	decltype(SymbolTable::table)::value_type& RecursiveSymTableIterator::operator*()
	{
		return *begin;
	}
	
	RecursiveSymTableIterator& RecursiveSymTableIterator::operator++()
	{
		++begin;
		if(begin == end && stack.size() > 0)
		{
			auto top = stack.top();
			stack.pop();
			for(auto& childPtr : top->children)
			{
				stack.push(childPtr.get());
			}
			if(stack.size() > 0)
			{
				begin = top->table.begin();
				end = top->table.end();
			}
		}
			
		return *this;
	}
	
	RecursiveSymTableIterator RecursiveSymTableIterator::operator++(int)
	{
		auto copy = *this;
		++(*this);
		return copy;
	}

	bool RecursiveSymTableIterator::operator!=(const RecursiveSymTableIterator& other)
	{
		return stack.size() > 0;
	}





}