add_library(Parser)

target_sources(Parser
	PRIVATE
		src/Parser.cpp
		include/Parser/Parser.hpp
)

target_include_directories(Parser
	PUBLIC
		include/
	PRIVATE
		include/Parser
)

target_compile_features(Parser
	PUBLIC
		cxx_std_20
)

target_link_libraries(Parser
	Token
	Ast
)