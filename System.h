#pragma once

#include "Bios.h"

#include "Bus.h"
#include "Cpu.h"
#include "Gpu.h"
#include "Ram.h"

class System {
public:
	const Cpu& get_cpu() const { return m_cpu; }
	const Bios& get_bios() const { return m_bios; }
	const Bus& get_bus() const { return m_bus; }
	const Ram& get_ram() const { return m_memory; }

	void run();
	void pause(bool pause_state);
	void quit(bool quit_state);
private:
    static constexpr std::string bios_file_path { "../scph1001.bin" };
    Bios m_bios { bios_file_path };
    Ram m_memory {};
	Gpu m_gpu {};
    Bus m_bus { m_bios, m_memory, m_gpu };
	Cpu m_cpu { m_bus };
	bool m_pause_system { false };
};
