#include "Bios.h"
#include "Bus.h"
#include "Ram.h"
#include "Cpu.h"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl3.h"
#include "Dependencies/imgui/imgui_impl_sdlrenderer3.h"

#include <SDL3/SDL.h>

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "Failed to initialize SDL.\n";
		return 1;
	}

	float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	SDL_Window *window = SDL_CreateWindow("PLEASE WORK", (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
	if (!window) {
		std::cerr << "Failed to create window.\n";
		return 1;
	}

	SDL_Renderer * renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer) {
		std::cerr << "Failed to create renderer.\n";
		return 1;
	}

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Bios bios { "../scph1001.bin" };
	Ram memory {};

	Bus bus { bios, memory };

	Cpu cpu { bus };

	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				quit = true;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
				quit = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
				{
					static float f = 0.0f;
					static int counter = 0;

					ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

					ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
					ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
					ImGui::Checkbox("Another Window", &show_another_window);

					ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
					ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

					if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
						counter++;
					ImGui::SameLine();
					ImGui::Text("counter = %d", counter);

					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::End();
				}
		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
	}
	//cpu.fetch_decode_execute();


	return 0;
}
