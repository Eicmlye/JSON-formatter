# change VERSION parameter to newest (3.23)
# avoid error in compacity
cmake_minimum_required (VERSION 3.23)
project (leptjson_test_parser CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(leptjson leptjson.cpp)
add_executable(leptjson_test_parser test_parser.cpp)
target_link_libraries(leptjson_test_parser leptjson)

# change start-up project from ALL_BUILD to leptjson_test_parser
# avoid error in Visual Studio, see Kevin's answer at https://stackoverflow.com/questions/59789453
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT leptjson_test_parser)
