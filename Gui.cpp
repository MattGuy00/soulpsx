#include "Gui.h"

#include <sstream>
#include <utility>

#include "Cpu.h"
#include "Instruction.h"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl3.h"
#include "Dependencies/imgui/imgui_impl_sdlrenderer3.h"

#include <SDL3/SDL.h>

#include "System.h"

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

Gui::Gui(std::shared_ptr<System> system, int width, int height)
    : m_width { width }, m_height { height }, m_system { std::move(system) }
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

    if (m_system) {
        ImGui::Begin("Registers", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
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

        render_disassembler();
    }

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SDL_SetRenderScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(m_renderer, m_clear_color.x, m_clear_color.y, m_clear_color.z, m_clear_color.w);
    SDL_RenderClear(m_renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
}

void Gui::disassemble_memory(std::span<const std::byte> memory) {
    m_disassembled_instructions.clear();
    for (uint32_t offset = 0; offset + 4 <= memory.size(); offset += 4) {
        Instruction instruction { memory.subspan(offset, 4), m_system->get_cpu().get_pc() };
        m_disassembled_instructions.emplace_back(instruction);
    }

    m_disassembled_memory_region = m_system->get_current_memory_region();
}

void Gui::render_disassembler() {
    ImGui::Begin("Disassembly", 0, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Memory Region: %s", Bus::region_name(m_system->get_current_memory_region()).data());

    // Only disassemble memory if the memory region has changed since it last happened.
    if (m_disassembled_memory_region != m_system->get_current_memory_region()) {
        disassemble_memory(m_system->get_bus().read_memory(m_system->get_current_memory_region()));
    }

    if (ImGui::BeginTable("Disassembly", 3 , ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        uint32_t pc_relative_offset { m_system->get_bus().get_relative_offset(m_system->get_cpu().get_pc()) };
        int64_t start_offset { pc_relative_offset - 4 * 20 };
        for (uint32_t offset = (start_offset < 0 ? 0 : start_offset); offset < pc_relative_offset; offset += 4) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored({0, 80, 200, 1}, "0x%08x:", offset);

            ImGui::TableNextColumn();

            ImGui::Text(m_disassembled_instructions[offset / 4].as_string().data());

            // Render instruction as hex
            ImGui::TableNextColumn();
            ImGui::Text(m_disassembled_instructions[offset / 4].as_hex().data());
        }

        for (uint32_t offset = pc_relative_offset; offset < pc_relative_offset + 4 * 20; offset += 4) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored({0, 80, 200, 1}, "0x%08x:", offset);

            ImGui::TableNextColumn();
            ImGui::Text(m_disassembled_instructions[offset / 4].as_string().data());

            // Jump to and highlight the most recently executed instruction
            if (offset == m_system->get_bus().get_relative_offset(m_system->get_cpu().get_pc())) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImU32(0xAA50bc1b));
                ImGui::SetScrollHereY();
            }
            // Render instruction as hex
            ImGui::TableNextColumn();
            ImGui::Text(m_disassembled_instructions[offset / 4].as_hex().data());
        }

        // uint32_t offset { 0 };
        // for (const auto& instruction : m_disassembled_instructions) {
        //     ImGui::TableNextRow();
        //
        //     ImGui::TableNextColumn();
        //     ImGui::TextColored({0, 80, 200, 1}, "0x%08x:", offset);
        //     offset += 4;
        //
        //     ImGui::TableNextColumn();
        //     ImGui::Text(instruction.as_string().data());
        //
        //     // Jump to and highlight the most recently executed instruction
        //     if (offset == m_system->get_bus().get_relative_offset(m_system->get_cpu().get_pc())) {
        //         ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImU32(0xAA50bc1b));
        //         ImGui::SetScrollHereY();
        //     }
        //     // Render instruction as hex
        //     ImGui::TableNextColumn();
        //     ImGui::Text(instruction.as_hex().data());
        // }
        ImGui::EndTable();
    }
    ImGui::End();
}


