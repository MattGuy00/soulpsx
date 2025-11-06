#pragma once

#include "Instruction.h"
#include <deque>
#include <string_view>

#include "Bus.h"
#include "../../../../Library/Frameworks/SDL3.xcframework/macos-arm64_x86_64/SDL3.framework/Versions/A/Headers/SDL_video.h"
#include "Dependencies/imgui/imgui.h"

class System;
struct ImGuiIO;
struct SDL_Window;
struct SDL_Renderer;
class Cpu;

class Gui {
public:
   Gui(std::shared_ptr<System>, int width, int height);
   ~Gui();

   [[nodiscard]] bool init_failed() const { return m_init_failed; }
   [[nodiscard]] SDL_Window* get_window() const { return m_window; }
   [[nodiscard]] float get_scale() const { return m_scale; }

   void render();
private:
   SDL_Window* m_window {};
   SDL_GLContext m_gl_context {};
   std::string_view m_title {};
   int m_width {};
   int m_height {};
   float m_scale {};
   bool m_init_failed { false };
   ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

   std::shared_ptr<System> m_system;

   std::vector<Instruction> m_disassembled_instructions {};
   std::unordered_map<uint32_t, std::pair<std::string, std::string>> m_executed_instructions {};
   std::deque<uint32_t> m_instructions_to_render {};
   uint32_t last_pc {};
   ImVec4 addr_colour = ImVec4{0, 80, 200, 1};

   void disassemble_memory(std::span<const std::byte> memory);
   void render_executed_instructions();
   void render_cpu_registers() const;
};
