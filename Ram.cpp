#include "Ram.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

std::span<const std::byte> Ram::read(uint32_t address, uint32_t bytes = 0) {
	return std::span{ m_ram }.subspan(address, bytes);
}

void Ram::write(uint32_t address, std::span<const std::byte> data) {
	std::copy(data.begin(), data.end(), m_ram.begin() + address);
}

