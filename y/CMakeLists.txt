cmake_minimum_required(VERSION 3.7)
project(y)

set(EXTRA_WARNINGS -Woverloaded-virtual -Wnon-virtual-dtor -Wodr -Wdisabled-optimization -Wlogical-op -Wredundant-decls -Wsign-promo -Wundef -Wzero-as-null-pointer-constant)
set(Y_COMPILE_OPTIONS -pedantic -Wall -Wextra ${EXTRA_WARNINGS})

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wodr")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)


file(GLOB_RECURSE SOURCE_FILES
		"y/*.h"
		"y/*.cpp"
	)

file(GLOB_RECURSE TEST_FILES
		"tests/*.cpp"
	)

add_library(y STATIC ${SOURCE_FILES})
#target_link_libraries(y pthread)
target_compile_options(y PUBLIC ${Y_COMPILE_OPTIONS})


if(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	target_compile_options(y PUBLIC "-DY_DEBUG")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Release)
	target_compile_options(y PUBLIC "-DY_NO_DEBUG")
endif()


option(Y_BUILD_TESTS "Build tests" ON)
if(Y_BUILD_TESTS)
	#enable_testing()
	add_executable(tests ${TEST_FILES} "tests.cpp")
	target_compile_definitions(tests PRIVATE "-DY_BUILD_TESTS")
	target_link_libraries(tests y)
	#add_test(Test tests)
endif()
