cmake_minimum_required(VERSION 3.20)
project(shell C)

set(CMAKE_C_STANDARD 99)

add_executable(shell main.c parser.h parser.c handler.h handler.c processHandler.h processHandler.c)
