cmake_minimum_required(VERSION 3.22)
project(soulpsx CXX)

set(CMAKE_CXX_STANDARD 20)
find_package(SDL3 REQUIRED)


add_executable(soulpsx
	main.cpp 
	Bios.cpp 
	Instruction.cpp
	Ram.cpp
	Cpu.cpp
	Bus.cpp
		Gui.cpp
		Gui.h

	# Quick way to add imgui to the project. Need to come and clean up later
	Dependencies/imgui/imconfig.h
	Dependencies/imgui/imgui.cpp
	Dependencies/imgui/imgui.h
	Dependencies/imgui/imgui_demo.cpp
	Dependencies/imgui/imgui_draw.cpp
	Dependencies/imgui/imgui_internal.h
	Dependencies/imgui/imgui_tables.cpp
	Dependencies/imgui/imgui_widgets.cpp
	Dependencies/imgui/imstb_rectpack.h
	Dependencies/imgui/imstb_textedit.h
	Dependencies/imgui/imstb_truetype.h

	Dependencies/imgui/imgui_impl_sdl3.cpp
	Dependencies/imgui/imgui_impl_sdl3.h
	Dependencies/imgui/imgui_impl_sdlrenderer3.cpp
	Dependencies/imgui/imgui_impl_sdlrenderer3.h
)

target_link_libraries(soulpsx SDL3::SDL3)
target_include_directories(soulpsx PRIVATE ${SDL3_INCLUDE_DIRECTORIES})


target_compile_options(soulpsx PRIVATE -Wall -Wextra)
