#pragma once

#include "Instruction.h"
#include <deque>
#include <string_view>

#include "Dependencies/imgui/imgui.h"

struct ImGuiIO;
struct SDL_Window;
struct SDL_Renderer;
class Cpu;

class Gui {
public:
   Gui(std::string_view title, int width, int height);
   Gui(Cpu* cpu, int width, int height);
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

   Cpu* m_cpu {};

   std::deque<Instruction> m_executed_instructions;
   void render_instruction();
   std::string instruction_to_string(std::string_view instruction_name, const std::vector<std::string_view>& values);
};
