add_library(Lexer)

target_sources(Lexer
	PRIVATE
		src/Lexer.cpp
		include/Lexer/Lexer.hpp
)

target_include_directories(Lexer
	PUBLIC
		include/
	PRIVATE
		include/Lexer
)

target_compile_features(Lexer
	PUBLIC
		cxx_std_20
)

target_link_libraries(Lexer
	Token
)