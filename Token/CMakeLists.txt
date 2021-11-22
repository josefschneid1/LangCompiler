add_library(Token)

target_sources(Token
	PRIVATE
		src/Token.cpp
		include/Token/Token.hpp
)

target_include_directories(Token
	PUBLIC
		include/
	PRIVATE
		include/Token
)

target_compile_features(Token
	PUBLIC
		cxx_std_20
)