cmake_minimum_required(VERSION 3.22)

project(
    alloha
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(
    ALLOHA_FLAGS
    "-std=c17"
    "-Wall"
    "-Wextra"
    "-pedantic"
)

set(
    ALLOHA_SAN_FLAGS
    "-fsanitize=address"
    "-fsanitize=pointer-compare"
    "-fsanitize=pointer-subtract"
    "-fsanitize=shadow-call-stack"
    "-fsanitize=leak"
    "-fsanitize=undefined"
    "-fstack-protector-strong"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ---- alloha lib ----

add_library(
    alloha STATIC
    "${PROJECT_SOURCE_DIR}/src/core.c"
    "${PROJECT_SOURCE_DIR}/src/arena.c"
    "${PROJECT_SOURCE_DIR}/src/stack.c"
)
target_compile_options(alloha PRIVATE ${ALLOHA_FLAGS} ${ALLOHA_SAN_FLAGS})
target_link_libraries(alloha PRIVATE ${ALLOHA_SAN_FLAGS})
target_include_directories(alloha PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_executable(test_arena "${PROJECT_SOURCE_DIR}/tests/test_arena.c")
target_compile_options(test_arena PRIVATE ${ALLOHA_FLAGS} ${ALLOHA_SAN_FLAGS})
target_link_libraries(test_arena PRIVATE ${ALLOHA_SAN_FLAGS} alloha)

add_executable(test_stack "${PROJECT_SOURCE_DIR}/tests/test_stack.c")
target_compile_options(test_stack PRIVATE ${ALLOHA_FLAGS} ${ALLOHA_SAN_FLAGS})
target_link_libraries(test_stack PRIVATE ${ALLOHA_SAN_FLAGS} alloha)
