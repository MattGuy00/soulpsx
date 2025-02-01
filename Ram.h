#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

class Ram {

public:
	static constexpr int m_ram_size { 2048 * 1024 };
	static constexpr uint32_t m_address_start { 0x0 };

	void write_data(std::span<const std::byte> data, int offset = 0);
	std::span<const std::byte> read_data(int bytes, int offset = 0);

	std::array<std::byte, m_ram_size> m_ram {};
};
