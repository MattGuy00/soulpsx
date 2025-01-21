#include "Bios.h"
#include <cstdint>
#include <span>

uint32_t Bios::fetch_32(int offset) {
	uint32_t data {};
	data |= m_rom[offset];
	data |= m_rom[offset + 1] << 8;
	data |= m_rom[offset + 2] << 16;
	data |= m_rom[offset + 3] << 24;

	return data;
}

std::span<const std::byte> Bios::first_64k() {
	return std::as_bytes(std::span{m_rom});
}
