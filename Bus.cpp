#include "Bus.h"
#include <cstdint>

std::span<const std::byte> Bus::read_memory(uint32_t offset, uint32_t bytes = 0) {
	uint32_t kseg { offset & 0xf0000000 };
	uint32_t physical_address { offset };
	if (kseg >= 0x80000000 && kseg <= 0x90000000) {
		// Shave off leftmost bit
		physical_address &= 0x7fffffff;
	} else if (kseg >= 0xa0000000 && kseg <= 0xb0000000) {
		// shave off leftmost 3 bits
		physical_address &= 0x1fffffff;
	}

	if (physical_address >= bios_memory_begin && physical_address < bios_memory_end) {
		return bios.read(physical_address - bios_memory_begin, bytes);
	} else if (physical_address >= ram_begin && physical_address < ram_end) {
		return memory.read_data(4, physical_address);
	}

	std::cout << "Out of bounds access\n";

	return std::span<const std::byte>();
}
