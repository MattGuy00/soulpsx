cmake_minimum_required(VERSION 3.22)
project(soulpsx CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(soulpsx 
	main.cpp 
	Bios.cpp 
	Instruction.cpp
	Memory.cpp
)
