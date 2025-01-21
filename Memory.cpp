#include "Memory.h"
#include <algorithm>
#include <cstddef>
#include <span>

void Memory::write_data(std::span<const std::byte> data, int offset) {
	std::copy(data.begin(), data.end(), m_ram.begin() + offset);
}

std::span<const std::byte> Memory::read_data(int bytes, int offset) {
	return std::as_bytes(std::span{ m_ram }.subspan(offset, bytes));
}
