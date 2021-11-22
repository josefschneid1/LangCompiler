add_library(Ast)

target_sources(Ast
	PRIVATE
		src/Ast.cpp
		src/SymbolTable.cpp
		include/Ast/Ast.hpp
		include/Ast/SymbolTable.hpp
)

target_include_directories(Ast
	PUBLIC
		include/
	PRIVATE
		include/Ast
)

target_compile_features(Ast
	PUBLIC
		cxx_std_20
)