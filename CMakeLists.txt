cmake_minimum_required(VERSION 3.16)
project(Z80Core VERSION 0.1.0 LANGUAGES CXX)

# ============================================================
# C++ Standard
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Options
# ============================================================
option(BUILD_SHARED_LIBS "Build as shared library" ON)
option(BUILD_TESTS "Build test suite" OFF)
option(ENABLE_PYBIND11 "Enable pybind11 bindings" OFF)

# Build type: Debug, Release, Trace, Test
set(BUILD_MODE "Release" CACHE STRING "Build mode: Debug, Release, Trace, Test")

# ============================================================
# Build mode configurations
# ============================================================
set_property(CACHE BUILD_MODE PROPERTY STRINGS "Debug" "Release" "Trace" "Test")

if(BUILD_MODE STREQUAL "Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    add_compile_definitions(Z80_DEBUG=1)
    message(STATUS "Debug mode: Extra validation enabled")
elseif(BUILD_MODE STREQUAL "Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    add_compile_definitions(Z80_RELEASE=1)
    message(STATUS "Release mode: Optimizations enabled")
elseif(BUILD_MODE STREQUAL "Trace")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    add_compile_definitions(Z80_TRACE=1)
    message(STATUS "Trace mode: Instruction logging enabled")
elseif(BUILD_MODE STREQUAL "Test")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    add_compile_definitions(Z80_TEST=1)
    message(STATUS "Test mode: Additional sanity checks enabled")
endif()

# ============================================================
# Warning flags
# ============================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
    # Treat warnings as errors in CI
    if(CI)
        add_compile_options(-Werror)
    endif()
endif()

# ============================================================
# Source files
# ============================================================
set(Z80_SOURCES
    src/z80.cpp
    src/opcode_table.cpp
    src/flags.cpp
    src/handlers/load.cpp
    src/handlers/block.cpp
    src/handlers/alu.cpp
    src/handlers/jump.cpp
    src/handlers/io.cpp
    # Add more handler files as they are implemented
)

set(Z80_HEADERS
    include/z80/z80.h
    include/z80/bus.h
    include/z80/registers.h
    include/z80/flags.h
    include/z80/opcode_table.h
    include/z80/timing.h
    include/z80/handlers.h
)

# ============================================================
# Z80 Library
# ============================================================
add_library(z80_core ${Z80_SOURCES} ${Z80_HEADERS})

target_include_directories(z80_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(z80_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${Z80_HEADERS}"
)

# ============================================================
# Optional: pybind11 bindings
# ============================================================
if(ENABLE_PYBIND11)
    find_package(pybind11 CONFIG REQUIRED)
    
    # Python binding sources (add as needed)
    set(PYBIND11_SOURCES
        src/pybind11/z80_bindings.cpp
    )
    
    pybind11_add_module(z80_py ${PYBIND11_SOURCES})
    target_link_libraries(z80_py PRIVATE z80_core)
    target_include_directories(z80_py PRIVATE ${PYTHON_INCLUDE_DIRS})
    
    install(TARGETS z80_py DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

# ============================================================
# Tests
# ============================================================
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        add_executable(z80_tests
            tests/test_z80.cpp
            tests/test_runner.cpp
        )
        target_link_libraries(z80_tests PRIVATE z80_core GTest::GTest GTest::Main)
        add_test(NAME z80_tests COMMAND z80_tests)
    else()
        message(WARNING "GTest not found, skipping tests")
    endif()
endif()

# ============================================================
# Installation
# ============================================================
include(GNUInstallDirs)

install(TARGETS z80_core
    EXPORT Z80CoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT Z80CoreTargets
    FILE Z80CoreTargets.cmake
    NAMESPACE Z80Core::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Z80Core
)

# ============================================================
# Package config for find_package()
# ============================================================
configure_file(Z80CoreConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Z80CoreConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Z80CoreConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Z80Core
)

message(STATUS "Z80 Core build configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
