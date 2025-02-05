#include "Bus.h"

#include <cstdint>
#include <cstdlib>

std::span<const std::byte> Bus::read_memory(uint32_t address, uint32_t bytes = 0) {
	uint32_t physical_address { to_physical_address(address) };

	if (physical_address >= bios_memory_begin && physical_address < bios_memory_end) {
		return bios.read(physical_address - bios_memory_begin, bytes);
	} else if (physical_address >= ram_begin && physical_address < ram_end) {
		return ram.read(physical_address - ram_begin, bytes);
	}

	std::cout << "Out of bounds access\n";

	return std::span<const std::byte>();
}

void Bus::write_memory(uint32_t address, std::span<const std::byte> data) {
	uint32_t physical_address { to_physical_address(address) };

	if (physical_address >= bios_memory_begin && physical_address < bios_memory_end) {
		std::cout << "Illegal write to Read Only Memory (Bios)\n";
	} else if (physical_address >= ram_begin && physical_address < ram_end) {
		ram.write(physical_address - ram_begin, data);
	}
}

// Translates the virtual memory address into its physical memory address
uint32_t Bus::to_physical_address(uint32_t virtual_address) {
	uint32_t kseg { virtual_address & 0xf0000000 };
	uint32_t physical_address { virtual_address };
	if (kseg >= 0x80000000 && kseg <= 0x90000000) {
		// Shave off leftmost bit
		physical_address &= 0x7fffffff;
	} else if (kseg >= 0xa0000000 && kseg <= 0xb0000000) {
		// shave off leftmost 3 bits
		physical_address &= 0x1fffffff;
	}

	return physical_address;
}
