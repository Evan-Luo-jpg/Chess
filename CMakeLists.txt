cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")

# Add source files
set(SOURCES
    src/main.cpp
    src/bitboard.cpp
    src/board.cpp
    src/movegen.cpp
    src/search.cpp
    src/eval.cpp
)

# Create executable
add_executable(chess_engine ${SOURCES})

# Include directories
target_include_directories(chess_engine PRIVATE src)

# Set output directory
set_target_properties(chess_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 