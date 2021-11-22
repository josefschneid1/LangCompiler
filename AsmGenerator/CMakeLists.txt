add_library(AsmGenerator)

target_sources(AsmGenerator
	PRIVATE
		src/AsmGenerator.cpp
		src/Register.cpp
		include/AsmGenerator/AsmGenerator.hpp
		include/AsmGenerator/Register.hpp
)

target_include_directories(AsmGenerator
	PUBLIC
		include/
	PRIVATE
		include/AsmGenerator
)

target_compile_features(AsmGenerator
	PUBLIC
		cxx_std_20
)

target_link_libraries(AsmGenerator
	Tac
)