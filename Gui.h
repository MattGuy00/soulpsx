#pragma once

#include "Instruction.h"
#include <deque>
#include <string_view>

#include "Bus.h"
#include "Dependencies/imgui/imgui.h"

class System;
struct ImGuiIO;
struct SDL_Window;
struct SDL_Renderer;
class Cpu;

class Gui {
public:
   Gui(std::string_view title, int width, int height);
   Gui(std::shared_ptr<System>, int width, int height);
   ~Gui();

   [[nodiscard]] bool init_failed() const { return m_init_failed; }
   [[nodiscard]] SDL_Window* get_window() const { return m_window; }
   [[nodiscard]] SDL_Renderer* get_renderer() const { return m_renderer; }
   [[nodiscard]] float get_scale() const { return m_scale; }

   void render();
private:
   SDL_Window* m_window {};
   SDL_Renderer* m_renderer {};
   std::string_view m_title {};
   int m_width {};
   int m_height {};
   float m_scale {};
   bool m_init_failed { false };
   ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

   std::shared_ptr<System> m_system;

   std::vector<Instruction> m_disassembled_instructions {};
   Region m_disassembled_memory_region { Region::unknown };

   std::deque<Instruction> m_executed_instructions;
   void disassemble_memory(std::span<const std::byte> memory);
   void render_disassembler();
   std::string instruction_as_string(const Instruction& instruction) const;
   std::string instruction_to_string(std::string_view instruction_name, const std::vector<std::string_view>& values) const;
};
