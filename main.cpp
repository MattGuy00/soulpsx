#include "Bios.h"
#include "Bus.h"
#include "Cpu.h"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl3.h"

#include <SDL3/SDL.h>

#include "Gui.h"
#include "System.h"

int main() {
	auto system { std::make_shared<System>() };
	Gui gui { system, 1280, 720 };
	if (gui.init_failed()) {
		return EXIT_FAILURE;
	}

	bool quit = false;
	bool pause = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT) {
				quit = true;
			} else if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.key == SDLK_P) {
					pause = !pause;
				}
				if (event.key.key == SDLK_RIGHT) {
					if (pause) {
						system->pause(false);
						system->run();
						system->pause(true);
					}
				}
			}
		}

		system->run();

		gui.render();
		system->pause(pause);
	}


	return 0;
}
