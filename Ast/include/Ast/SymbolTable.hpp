#ifndef symboltable_hpp
#define symboltable_hpp
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <variant>
#include <concepts>
#include <stdexcept>
#include <stack>
#include <optional>

namespace assembly
{
	enum class Register;
}

namespace intermediate_rep
{

	// Store Information about Identifiers

	// Variables:
	// where they are located in Memory or in which register they reside
	// Function:
	// the retrurn type, parameters

	class RecursiveSymTableIterator;


	class SymbolTable
	{
	public:
		
		SymbolTable(SymbolTable* parent = nullptr);

		// All Compiler generated Variables start with two underscores (__)
		struct Variable
		{
			std::string name;
			enum Type
			{
				Int,	// 8 Byte
				Float,	// 8 Byte
				Str, 	// 8 Byte
				Bool	// 1 Byte
			} type;

			int basePointerOffset = 0;

			// -1 means no next use
			int nextUse = -1;

			bool live = false;

			std::optional<assembly::Register> reg;
		};

		struct Function
		{
			std::string name;
			Variable::Type returnType;
			SymbolTable* parameter_scope = nullptr;
			std::vector<Variable*> parameters;
		};

		using Symbol = std::variant<Variable, Function>;
 
		Symbol& operator[](const std::string& key);


		Symbol& insert(const std::string& key, Symbol v);

		template<typename T>
			requires std::same_as<T, SymbolTable::Variable> || std::same_as<T, SymbolTable::Function>
		T & get(const std::string & key)
		{
			auto ptr = std::get_if<T>(&(*this)[key]);
			if (ptr == nullptr)
			{
				throw std::runtime_error("");
			}
			return *ptr;
		}

		template<typename T>
			requires std::same_as<T, SymbolTable::Variable> || std::same_as<T, SymbolTable::Function>
		T & insert(const std::string & key, T v)
		{
			return std::get<T>(insert(key, Symbol{ std::move(v) }));
		}


		SymbolTable& addChild(std::unique_ptr<SymbolTable> child);

		std::size_t numChildren();

		SymbolTable& getChild(std::size_t indx);

		friend class RecursiveSymTableIterator;

		RecursiveSymTableIterator begin();
		RecursiveSymTableIterator end();

	private:
		std::map<std::string, Symbol> table;
		SymbolTable* parent;
		std::vector<std::unique_ptr<SymbolTable>> children;
	};

	class RecursiveSymTableIterator
	{
	public:
		RecursiveSymTableIterator() = default;
		RecursiveSymTableIterator(SymbolTable* sym_table);
		decltype(SymbolTable::table)::value_type& operator*();
		RecursiveSymTableIterator& operator++();
		RecursiveSymTableIterator operator++(int);
		bool operator!=(const RecursiveSymTableIterator& other);

	private:
		std::stack<SymbolTable*> stack;
		decltype(SymbolTable::table)::iterator begin = {};
		decltype(SymbolTable::table)::iterator end = {};
	};

	// Class lets me build the symbol table incrementally
	class SymbolTableBuilder
	{
	public:
		SymbolTableBuilder(SymbolTable* root);
		void push();
		void pop();
		SymbolTable* top();
	private:
		std::stack<SymbolTable*> path;
	};

	//Creates new Scope on construction and pops Scope on destruction
	class ScopeGuard
	{
	public:
		ScopeGuard(SymbolTableBuilder& builder);
		~ScopeGuard();
	private:
		SymbolTableBuilder& builder;
	};

	
}
#endif