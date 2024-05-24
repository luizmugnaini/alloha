# ------------------------------------------------------------------------------
# Alloha library CMake build file.
# 
# Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.22)

project(
    alloha
    VERSION 0.0.2
    LANGUAGES C
)

# ------------------------------------------------------------------------------
# Compiler info
# ------------------------------------------------------------------------------

message(STATUS "C Compiler executable: ${CMAKE_C_COMPILER}")
message(STATUS "Linker executable: ${CMAKE_LINKER}")

# Enable LSP integration with the project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(status "Enabling tests")
    enable_testing()
endif()

# ------------------------------------------------------------------------------
# GCC configuration
# ------------------------------------------------------------------------------

if (CMAKE_CXX_COMPILER_ID STREQUAL "GCC")
    set(
        ALLOHA_FLAGS
        "-std=c17"
        "-Wall"
        "-Wextra"
        "-pedantic"
        "-Wuninitialized"
        "-Wswitch"
        "-Wcovered-switch-default"
        "-Wshadow" 
                              
        "-Wcast-align"
        "-Wunused"
        "-Woverloaded-virtual"
                              
        "-Wpedantic"
        "-Wconversion"
        "-Wsign-conversion"
        "-Wnull-dereference"
        "-Wdouble-promotion"
        "-Wmisleading-indentation"
        "-fcolor-diagnostics"
    )
    set(
        ALLOHA_SAN_FLAGS
        "-fsanitize=address"
        "-fsanitize=pointer-compare"
        "-fsanitize=pointer-subtract"
        "-fsanitize=shadow-call-stack"
        "-fsanitize=undefined"
        "-fstack-protector-strong"
        "-fsanitize=leak"
    )
endif()

# ------------------------------------------------------------------------------
# Clang configuration
# ------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # ---- Native Clang ----
    if(UNIX)
        # ---- Compilation flags ----

        set(
            ALLOHA_FLAGS
            "-std=c17"
            "-Wall"
            "-Wextra"
            "-pedantic"
            "-Wuninitialized"
            "-Wswitch"
            "-Wcovered-switch-default"
            "-Wshadow"
            "-Wcast-align"
            "-Wunused"
            "-Woverloaded-virtual"
                                  
            "-Wpedantic"
            "-Wconversion"
            "-Wsign-conversion"
            "-Wnull-dereference"
            "-Wdouble-promotion"
            "-Wmisleading-indentation"

            "-fcolor-diagnostics"
        )

        # ---- Sanitizer flags ----

        set(
            ALLOHA_SAN_FLAGS
            "-fsanitize=address"
            "-fsanitize=pointer-compare"
            "-fsanitize=pointer-subtract"
            "-fsanitize=shadow-call-stack"
            "-fsanitize=undefined"
            "-fstack-protector-strong"
            "-fsanitize=leak"
        )
    endif()

    # ---- Clang with MSVC backend (Clang-CL) ----

    if(MSVC)
        set(
            ALLOHA_FLAGS
            "/std:c17"

            "/Ob2"          # Inline functions as deemed beneficial by the compiler.
            "-nobuiltininc" # Disable builtin #include directories.

            "-fcolor-diagnostics"

            "/W3"
            "-Wuninitialized"
            "-Wswitch"
            "-Wcovered-switch-default"
            "-Wshadow"
            "-Wunused"
            "-Wpedantic"
            "-Wconversion"
            "-Wsign-conversion"
            "-Wnull-dereference"
            "-Wdouble-promotion"
            "-Wmisleading-indentation"
            "-Wcast-align"
            "-Wno-cast-align"
            "-Wno-language-extension-token"

            "/D_CRT_SECURE_NO_WARNINGS"
        )

        # ---- Debug flags ----

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "Compiling with debug flags")
            set(
                MINA_CXX_FLAGS ${MINA_CXX_FLAGS}
                "/Zi"   # Debug info.
                "/Ob0"  # Disable inlining.
                "/Oy-"  # Disable frame pointer optimization.
            )
        else()
            set(
                MINA_CXX_FLAGS ${MINA_CXX_FLAGS}
                "/Ob2"  # Inline functions as deemed beneficial by the compiler.
            )
        endif()

        # ---- Sanitizer flags ----

        set(MINA_CXX_SAN_FLAGS "")  # TODO: Enable ASan and UBSan

    endif() # MSVC Clang
endif() # Clang

# ------------------------------------------------------------------------------
# MSVC configuration
# ------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(
        ALLOHA_FLAGS
        "/std:c17"
        "/W3"
        "/GA"   # Optimize for Windows applications.
        "/D_CRT_SECURE_NO_WARNINGS"
    )
set(ALLOHA_SAN_FLAGS "")
endif()

# ------------------------------------------------------------------------------
# Inform the resulting compiler and sanitizer flags
# ------------------------------------------------------------------------------

message(STATUS "Mina compiler flags: ${MINA_CXX_FLAGS}")
message(STATUS "Mina sanitizer flags: ${MINA_CXX_SAN_FLAGS}")

# ------------------------------------------------------------------------------
# alloha library
# ------------------------------------------------------------------------------

# Output directory for the binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(
    alloha STATIC
    "${PROJECT_SOURCE_DIR}/src/core.c"
    "${PROJECT_SOURCE_DIR}/src/arena.c"
    "${PROJECT_SOURCE_DIR}/src/stack.c"
)
target_compile_options(alloha PRIVATE ${ALLOHA_FLAGS} ${ALLOHA_SAN_FLAGS})
target_link_libraries(alloha PRIVATE ${ALLOHA_SAN_FLAGS})
target_include_directories(alloha PUBLIC "${PROJECT_SOURCE_DIR}/include")

# ------------------------------------------------------------------------------
# alloha library tests
# ------------------------------------------------------------------------------

list(APPEND TESTS "test_arena" "test_stack")

foreach(t IN LISTS TESTS)
    add_executable(${t} "${PROJECT_SOURCE_DIR}/tests/${t}.c")
    target_compile_options(${t} PRIVATE ${ALLOHA_FLAGS} ${ALLOHA_SAN_FLAGS})
    target_link_libraries(${t} PRIVATE ${ALLOHA_SAN_FLAGS} alloha)
    add_test(NAME ${t} COMMAND ${t})
endforeach()
