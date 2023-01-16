cmake_minimum_required (VERSION 3.8)

project ("sokket")

add_executable (sokket "sokket.cpp" "sokket.hpp"  "client.cpp" "server.cpp" "commandlineparser.cpp" "main.cpp")

set_property(TARGET sokket PROPERTY CXX_STANDARD 20)
