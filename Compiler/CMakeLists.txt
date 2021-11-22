add_executable(Compiler)

target_sources(Compiler
	PRIVATE
		src/main.cpp
)

target_include_directories(Compiler
	PUBLIC
		include/
	PRIVATE
		include/Compiler
)

target_compile_features(Compiler
	PUBLIC
	cxx_std_20
)

target_link_libraries(Compiler
	PUBLIC
		Ast
		Parser
		Lexer
		Token
		TacGenerator
		AsmGenerator
)