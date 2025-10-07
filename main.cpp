#include "Bios.h"
#include "Bus.h"
#include "Ram.h"
#include "Cpu.h"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl3.h"
#include "Dependencies/imgui/imgui_impl_sdlrenderer3.h"

#include <SDL3/SDL.h>

#include "Gui.h"

int main() {

	Bios bios { "../scph1001.bin" };
	Ram memory {};
	Bus bus { bios, memory };
	Cpu cpu { bus };
	Gui gui { &cpu, 1280, 720 };
	if (gui.init_failed()) {
		return EXIT_FAILURE;
	}

	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				quit = true;
		}

		cpu.fetch_decode_execute();

		gui.render();
	}


	return 0;
}
