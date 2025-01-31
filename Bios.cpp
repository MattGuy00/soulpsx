#include "Bios.h"
#include <cstdint>
#include <span>

std::span<const std::byte> Bios::read(uint32_t offset, uint32_t bytes = 0) const {
	return std::span{ m_rom }.subspan(offset, bytes);
}
