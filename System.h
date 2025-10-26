#pragma once

#include "Bios.h"

#include <string_view>

#include "Bus.h"
#include "Cpu.h"
#include "Ram.h"

class System {
public:
	const Cpu& get_cpu() const { return m_cpu; }
	const Bios& get_bios() const { return m_bios; }
	const Bus& get_bus() const { return m_bus; }
	void run();
	Region get_current_memory_region() const { return m_cpu.get_current_memory_region(); }
private:
    static constexpr std::string bios_file_path { "../scph1001.bin" };
    Bios m_bios { bios_file_path };
    Ram m_memory {};
    Bus m_bus { m_bios, m_memory };
	Cpu m_cpu { m_bus };
};
