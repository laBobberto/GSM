cmake_minimum_required(VERSION 3.10)
project(csv_evaluator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

add_executable(csvreader 
    src/main.c 
    src/parser.c 
    src/evaluator.c 
    src/table.c
)

target_include_directories(csvreader PRIVATE include)
