cmake_minimum_required(VERSION 3.20)
project(pyz80 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(nanobind CONFIG REQUIRED)

add_library(z80_core STATIC
    cpp/core/flags.cpp
    cpp/core/registers.cpp
    cpp/core/bus.cpp
    cpp/core/handlers.cpp
    cpp/core/decoder.cpp
    cpp/core/cpu.cpp
)
target_include_directories(z80_core PUBLIC cpp/core)
set_target_properties(z80_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

nanobind_add_module(_pyz80 cpp/core/pyz80.cpp)
target_link_libraries(_pyz80 PRIVATE z80_core)
install(TARGETS _pyz80 LIBRARY DESTINATION core)
