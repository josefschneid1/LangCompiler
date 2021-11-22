add_library(TacGenerator)

target_sources(TacGenerator
	PRIVATE
		src/TacGenerator.cpp
)

target_include_directories(TacGenerator
	PUBLIC
		include/
	PRIVATE
		include/TacGenerator
)

target_compile_features(TacGenerator
	PUBLIC
		cxx_std_20
)

target_link_libraries(TacGenerator
	Tac
	Ast
)