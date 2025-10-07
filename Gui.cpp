#include "Gui.h"
#include "Cpu.h"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl3.h"
#include "Dependencies/imgui/imgui_impl_sdlrenderer3.h"

#include <SDL3/SDL.h>

Gui::Gui(std::string_view title, int width, int height)
    : m_title {title}, m_width { width }, m_height { height }
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        m_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (SDL_CreateWindowAndRenderer(title.data(), width * m_scale, height * m_scale, window_flags, &m_window, &m_renderer)) {
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(m_window);

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
            ImGui_ImplSDLRenderer3_Init(m_renderer);

            ImGui::StyleColorsDark();

            // Setup scaling
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(m_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
            style.FontScaleDpi = m_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window and renderer: %s", SDL_GetError());
            m_init_failed = true;
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Initialise SDL: %s", SDL_GetError());
        m_init_failed = true;
    }
}

Gui::Gui(Cpu* cpu, int width, int height)
    : m_cpu { cpu }, m_width { width }, m_height { height }
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        m_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (SDL_CreateWindowAndRenderer("Soulpsx", width * m_scale, height * m_scale, window_flags, &m_window, &m_renderer)) {
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(m_window);

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
            ImGui_ImplSDLRenderer3_Init(m_renderer);

            ImGui::StyleColorsDark();

            // Setup scaling
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(m_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
            style.FontScaleDpi = m_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window and renderer: %s", SDL_GetError());
            m_init_failed = true;
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Initialise SDL: %s", SDL_GetError());
        m_init_failed = true;
    }
}

Gui::~Gui() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Gui::render() {
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    if (m_cpu) {
        ImGui::Begin("Registers", 0, ImGuiWindowFlags_AlwaysAutoResize);
        for (int i = 0; i < 32; i++) {
            if (i != 0 && i % 4 != 0) {
                ImGui::SameLine();
            }
            Register reg { static_cast<Register>(i) };
            uint32_t value { m_cpu->get_register_data(reg) };
            ImGui::TextColored({0, 80, 200, 1}, "%s:", m_cpu->register_name(reg).data());
            ImGui::SameLine( );
            ImGui::Text("0x%08x", value);
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SDL_SetRenderScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(m_renderer, m_clear_color.x, m_clear_color.y, m_clear_color.z, m_clear_color.w);
    SDL_RenderClear(m_renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
}
