cmake_minimum_required(VERSION 3.20)
project(pyz80 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

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

Python_add_library(_pyz80 MODULE cpp/core/pyz80.cpp)
target_link_libraries(_pyz80 PRIVATE z80_core Python::Module)
set_target_properties(_pyz80 PROPERTIES PREFIX "")
install(TARGETS _pyz80 LIBRARY DESTINATION core)
