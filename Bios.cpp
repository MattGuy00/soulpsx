#include "Bios.h"
#include <cstdint>
#include <span>

const std::span<const std::byte> Bios::read(uint32_t offset, uint32_t bytes = 0) const {
	if (bytes == 0) {
		return std::span { m_rom };
	}

	return std::span{ m_rom }.subspan(offset, bytes);
}
