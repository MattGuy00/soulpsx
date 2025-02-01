#pragma once

#include "Bios.h"
#include "Memory.h"

#include <cstdint>
#include <span>


struct Bus {
	std::span<const std::byte> read_memory(uint32_t offset, uint32_t bytes);

	Bios& bios;
	Memory& memory;

	static constexpr uint32_t bios_memory_begin { 0x1fc00000 };
	static constexpr uint32_t bios_size { 512 * 1024 };
	static constexpr uint32_t bios_memory_end { bios_memory_begin + bios_size };

	static constexpr uint32_t ram_begin { 0x0 };
	static constexpr uint32_t ram_size { 2048 * 1024 };
	static constexpr uint32_t ram_end { ram_begin + ram_size };
};
