#include "Bus.h"

std::span<const std::byte> Bus::read_memory(uint32_t offset, uint32_t bytes = 0) {
	if (offset >= bios.memory_region() && offset < bios.memory_region() + bios.rom_size()) {
		return bios.read(offset - bios.memory_region(), bytes);
	}

}
