cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (Differentiator)

set (DEBUG_FLAGS   -ggdb3 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wno-format-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wno-unused-parameter -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fPIC -fno-omit-frame-pointer -pie -fPIE -Werror=vla -Wno-write-strings -fsanitize=address,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr)
set (RELEASE_FLAGS -std=c++17)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/headers)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/headers/Differentiator)

add_executable (${PROJECT_NAME})

add_subdirectory (src)
add_subdirectory (libs)

target_compile_definitions (${PROJECT_NAME} PRIVATE _SHOW_STACK_TRACE)

target_compile_options (${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:${DEBUG_FLAGS}>)
target_compile_options (${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:${RELEASE_FLAGS}>)

target_link_options (${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:${DEBUG_FLAGS}>)
target_link_options (${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:${RELEASE_FLAGS}>)

target_link_libraries (${PROJECT_NAME} PRIVATE ColorConsole)
target_link_libraries (${PROJECT_NAME} PRIVATE CustomAssert)
target_link_libraries (${PROJECT_NAME} PRIVATE Buffer)
target_link_libraries (${PROJECT_NAME} PRIVATE FileIO)
target_link_libraries (${PROJECT_NAME} PRIVATE Tree)
