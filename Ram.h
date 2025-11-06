#pragma once

#include <array>
#include <cstdint>
#include <span>

class Ram {
public:

	std::span<const std::byte> read(uint32_t address, uint32_t bytes);
	void write(uint32_t address, std::span<const std::byte> data);
	std::span<const std::byte> get_memory() const { return { m_ram.data(), m_ram_size }; }
private:
	static constexpr int m_ram_size { 2048 * 1024 };
	std::array<std::byte, m_ram_size> m_ram {};
};
