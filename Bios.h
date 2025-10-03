#pragma once
#include <cstdint>
#include <array>
#include <fstream>
#include <iostream>
#include <span>

class Bios {
public:
	explicit Bios(const std::string& bios_path) {
		std::ifstream bios_file { bios_path, std::ios::binary };
		if (!bios_file.good()) {
			std::cerr << "Bad BIOS file.\n";
			std::exit(1);
		}

		bios_file.read(reinterpret_cast<char*>(m_rom.data()), m_bios_size);
	}
	
	std::span<const std::byte> read(uint32_t offset, uint32_t bytes) const;

	uint32_t rom_size() const { return m_bios_size; }
	uint32_t memory_region() const { return m_memory_region_start; }
private:
	// Bios size is 512KB
	static constexpr uint32_t m_bios_size { 512 * 1024 };
	std::array<std::byte, m_bios_size> m_rom {};
	static constexpr uint32_t m_memory_region_start { 0xbfc00000 };
};
