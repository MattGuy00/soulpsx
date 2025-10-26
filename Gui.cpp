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
                ImGui::TextColored({0, 80, 200, 1}, "%s:", m_system->get_cpu().register_name(reg).data());
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
                ImGui::TextColored({0, 80, 200, 1}, "%s:", m_system->get_cpu().cop0_register_name(reg).data());
                ImGui::SameLine( );
                ImGui::Text("0x%08x", value);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        ImGui::End();

        // Only disassemble memory if the memory region has changed since it last happened.
        if (m_disassembled_memory_region != m_system->get_current_memory_region()) {
            disassemble_memory(m_system->get_bus().read_memory(m_system->get_current_memory_region()));
        }
        ImGui::Begin("Disassembly", 0, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Memory Region: %s", Bus::region_name(m_system->get_current_memory_region()).data());
        ImGui::BeginTable("Disassembly", 3 , ImGuiTableFlags_SizingFixedFit);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        uint32_t offset { 0 };
        for (const auto& instruction : m_disassembled_instructions) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("0x%08x:", offset);
            offset += 4;

            ImGui::TableNextColumn();
            render_instruction(instruction);

            ImGui::TableNextColumn();
            ImGui::Text("%02hhX", instruction.data() >> 24);
            ImGui::SameLine();
            ImGui::Text("%02hhX", instruction.data() >> 16);
            ImGui::SameLine();
            ImGui::Text("%02hhX", instruction.data() >> 8);
            ImGui::SameLine();
            ImGui::Text("%02hhX", instruction.data());
        }
        ImGui::EndTable();
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

void Gui::disassemble_memory(std::span<const std::byte> memory) {
    m_disassembled_instructions.clear();
    for (uint32_t offset = 0; offset + 4 <= memory.size(); offset += 4) {
        Instruction instruction { memory.subspan(offset, 4) };
        m_disassembled_instructions.push_back(instruction);
    }

    m_disassembled_memory_region = m_system->get_current_memory_region();
}

void Gui::render_instruction(const Instruction& instruction) {
    std::string output {};
    switch (instruction.opcode()) {
        case Instruction::Opcode::add:
        case Instruction::Opcode::addu:
        case Instruction::Opcode::subu:
        case Instruction::Opcode::slt:
        case Instruction::Opcode::sltu:
        case Instruction::Opcode::and_b:
        case Instruction::Opcode::or_b:
        case Instruction::Opcode::Xor:
        case Instruction::Opcode::nor: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rd()),
                m_system->get_cpu().register_name(instruction.rs()),
                m_system->get_cpu().register_name(instruction.rt()),
            });
            break;
        }
        case Instruction::Opcode::sllv:
        case Instruction::Opcode::srlv:
        case Instruction::Opcode::srav: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rd()),
                m_system->get_cpu().register_name(instruction.rt()),
                m_system->get_cpu().register_name(instruction.rs()),
            });
            break;
        }
        case Instruction::Opcode::addi:
        case Instruction::Opcode::slti:
        case Instruction::Opcode::andi:
        case Instruction::Opcode::ori: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << instruction.imm16_se();
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rt()),
                m_system->get_cpu().register_name(instruction.rs()),
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::addiu:
        case Instruction::Opcode::sltiu: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << instruction.imm16_se();
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rt()),
                m_system->get_cpu().register_name(instruction.rs()),
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::sll:
        case Instruction::Opcode::srl:
        case Instruction::Opcode::sra: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << instruction.imm16_se();
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rd()),
                m_system->get_cpu().register_name(instruction.rt()),
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::lui: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << instruction.imm16_se();
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rt()),
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::multu:
        case Instruction::Opcode::div:
        case Instruction::Opcode::divu: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rs()),
                m_system->get_cpu().register_name(instruction.rt())
            });
            break;
        }
        case Instruction::Opcode::mfhi:
        case Instruction::Opcode::mflo: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rd()),
            });
            break;
        }
        case Instruction::Opcode::mthi:
        case Instruction::Opcode::mtlo: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rs()),
            });
            break;
        }
        case Instruction::Opcode::jump:
        case Instruction::Opcode::jal: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << m_system->get_cpu().get_next_pc();
            output = instruction_to_string(instruction.type_string(), {
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::jr: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rs()),
            });
            break;
        }
        case Instruction::Opcode::jalr: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rd()),
                m_system->get_cpu().register_name(instruction.rs()),
            });
            break;
        }
        case Instruction::Opcode::beq:
        case Instruction::Opcode::bne: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << m_system->get_cpu().get_next_pc();
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rs()),
                m_system->get_cpu().register_name(instruction.rt()),
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::bltz:
        case Instruction::Opcode::bgez:
        case Instruction::Opcode::bgtz:
        case Instruction::Opcode::blez: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << m_system->get_cpu().get_next_pc();
            output = instruction_to_string(instruction.type_string(), {
            m_system->get_cpu().register_name(instruction.rs()),
                value_as_hex.str()
            });
            break;
        }
        case Instruction::Opcode::syscall: {
            output = instruction_to_string(instruction.type_string(), {
            });
            break;
        }
        case Instruction::Opcode::lb:
        case Instruction::Opcode::lbu:
        case Instruction::Opcode::lh:
        case Instruction::Opcode::lhu:
        case Instruction::Opcode::lw:
        case Instruction::Opcode::sb:
        case Instruction::Opcode::sh:
        case Instruction::Opcode::sw:
        case Instruction::Opcode::lwr: {
            std::stringstream value_as_hex;
            value_as_hex << "0x" << std::hex << instruction.imm16_se();
            std::string load_address { value_as_hex.str() };
            load_address += "(";
            load_address += m_system->get_cpu().register_name(instruction.rs());
            load_address += ")";
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rt()),
                load_address
            });
            break;
        }
        case Instruction::Opcode::mfc0:
        case Instruction::Opcode::mtc0: {
            output = instruction_to_string(instruction.type_string(), {
                m_system->get_cpu().register_name(instruction.rt()),
                m_system->get_cpu().register_name(instruction.rd())
            });
            break;
        }
        case Instruction::Opcode::rfe:
        case Instruction::Opcode::unknown: {
            output = instruction_to_string(instruction.type_string(), {});
            break;
        }
    }

    ImGui::Text("%s", output.data());

}

// Helper function that takes an instruction name and the registers to modify, or immediate values as strings
// and prepares them for printing.
std::string Gui::instruction_to_string(std::string_view instruction_name, const std::vector<std::string_view>& values) {
    std::string output { instruction_name };
    output += " ";
    for (int i = 0; i < values.size(); i++) {
        output += values[i];
        if (i != values.size() - 1) {
            output += ", ";
        }
    }

    return output;
}
