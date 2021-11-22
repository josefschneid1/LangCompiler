add_library(Tac)

target_sources(Tac
	PRIVATE
		src/Tac.cpp
		include/Tac/Tac.hpp
)

target_include_directories(Tac
	PUBLIC
		include/
	PRIVATE
		include/Tac
)

target_compile_features(Tac
	PUBLIC
		cxx_std_20
)

target_link_libraries(Tac
	Ast
)