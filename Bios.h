#pragma once

#include <cstdint>
#include <array>
#include <fstream>
#include <iostream>
#include <span>

class Bios {
public:
	// Bios size is 512KB
	static constexpr int m_bios_size { 512 * 1024 };

	Bios(const std::string& bios_path) {
		std::ifstream bios_file { bios_path, std::ios::binary };
		if (!bios_file.good()) {
			std::cerr << "Bad BIOS file.\n";
		}

		bios_file.read(reinterpret_cast<char*>(m_rom.data()), m_bios_size);
	}

	// Fetches 32 bits of data from the rom
	uint32_t fetch_32(int offset); 

	// Returns the first 64k of bios rom	
	std::span<const std::byte> first_64k();

	std::array<std::uint8_t, m_bios_size> m_rom {};
};
