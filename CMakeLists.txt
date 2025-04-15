cmake_minimum_required(VERSION 3.10)
project(DBApp)

set(CMAKE_CXX_STANDARD 17)

# Add SQLite source
add_library(sqlite3 sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC sqlite)

# Main app
add_executable(DBApp main.cpp)
target_link_libraries(DBApp PRIVATE sqlite3)
