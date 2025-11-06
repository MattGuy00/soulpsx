#include "Gui.h"

#include <sstream>
#include <utility>

#include "Cpu.h"
#include "Bus.h"
#include "Instruction.h"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl3.h"
#include "Dependencies/imgui/imgui_impl_opengl3.h"

#include <SDL3/SDL.h>

#include "Logger.h"
#include "Memory.h"
#include "System.h"
#include "Dependencies/imgui/imgui_impl_opengl3_loader.h"

Gui::Gui(std::shared_ptr<System> system, int width, int height)
    : m_width { width }, m_height { height }, m_system { std::move(system) }
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        m_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL;
        if ((m_window = SDL_CreateWindow("Soulpsx", width * m_scale, height * m_scale, window_flags))) {
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(m_window);

            m_gl_context = SDL_GL_CreateContext(m_window);
            SDL_GL_MakeCurrent(m_window, m_gl_context);
            // SDL_GL_SetSwapInterval(1); // Enable vsync

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

            ImGui_ImplSDL3_InitForOpenGL(m_window, m_gl_context);
            ImGui_ImplOpenGL3_Init("#version 410 core");

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
    ImGui_ImplOpenGL3_Shutdown();
    SDL_GL_DestroyContext(m_gl_context);

    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(m_window);
    SDL_Quit();
}


void Gui::render() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (m_system) {
        render_cpu_registers();
        render_executed_instructions();
    }

    ImGui::Render();
    SDL_GL_MakeCurrent(m_window, m_gl_context);
    const ImGuiIO& io = ImGui::GetIO(); (void)io;
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);
}

void Gui::render_executed_instructions() {
    // ImGui::ShowMetricsWindow();
    ImGui::Begin("Executed Instructions", 0, ImGuiWindowFlags_NoCollapse);

    if (!m_executed_instructions.contains(m_system->get_cpu().get_pc())) {
        auto instruction { m_system->get_cpu().get_current_instruction() };
        m_executed_instructions[m_system->get_cpu().get_pc()] = std::pair{ instruction.to_string(), instruction.as_hex() };
    }
    m_instructions_to_render.push_back(m_system->get_cpu().get_pc());
    if (m_instructions_to_render.size() > 20) {
        m_instructions_to_render.pop_front();
    }

    if (ImGui::BeginTable("Disassembly", 3 , ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (const auto& address : m_instructions_to_render) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const auto text = m_executed_instructions[address];
            ImGui::TextColored(addr_colour, "0x%08x:", address);

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(text.first.c_str());

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(text.second.c_str());

        }

        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImU32(0xAA50bc1b));
        ImGui::SetScrollHereY();

        ImGui::EndTable();
    }
    ImGui::End();
}

void Gui::render_cpu_registers() const {
    ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::BeginTabBar("Registers");
    if (ImGui::BeginTabItem("CPU Registers")) {
        for (int i = 0; i < 32; i++) {
            if (i != 0 && i % 4 != 0) {
                ImGui::SameLine();
            }
            Register reg { static_cast<Register>(i) };
            uint32_t value { m_system->get_cpu().get_register_data(reg) };
            ImGui::TextColored({0, 80, 200, 1}, "%s:", Instruction::register_name(reg).data());
            ImGui::SameLine( );
            ImGui::Text("0x%08x", value);
        }

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("COP0 Registers")) {
        for (int i = 0; i < 16; i++) {
            if (i != 0 && i % 4 != 0) {
                ImGui::SameLine();
            }
            Cop0_Register reg { static_cast<Cop0_Register>(i) };
            uint32_t value { m_system->get_cpu().cop0_get_register_data(reg) };
            ImGui::TextColored({0, 80, 200, 1}, "%s:", Instruction::cop0_register_name(reg).data());
            ImGui::SameLine( );
            ImGui::Text("0x%08x", value);
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}
