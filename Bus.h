#pragma once

#include "Bios.h"
#include "Ram.h"

#include <cstdint>
#include <span>


struct Bus {
	std::span<const std::byte> read_memory(uint32_t address, uint32_t bytes);
	void write_memory(uint32_t address, std::span<const std::byte> data);
	uint32_t to_physical_address(uint32_t virtual_address);

	Bios& bios;
	Ram& ram;

	static constexpr uint32_t bios_memory_begin { 0x1fc00000 };
	static constexpr uint32_t bios_size { 512 * 1024 };
	static constexpr uint32_t bios_memory_end { bios_memory_begin + bios_size };

	static constexpr uint32_t ram_begin { 0x0 };
	static constexpr uint32_t ram_size { 2048 * 1024 };
	static constexpr uint32_t ram_end { ram_begin + ram_size };

	static constexpr uint32_t io_ports_begin { 0x1f801000 };
	static constexpr uint32_t io_ports_size { 4 * 1024 };
	static constexpr uint32_t io_ports_end { io_ports_begin + io_ports_size };

	static constexpr uint32_t cache_control_begin { 0xfffe0000 };
	static constexpr uint32_t cache_control_size { 512 };
	static constexpr uint32_t cache_control_end { cache_control_begin + cache_control_size };

	static constexpr uint32_t expansion_region_2_begin { 0x1f802000 };
	static constexpr uint32_t expansion_region_2_size { 8 * 1024 };
	static constexpr uint32_t expansion_region_2_end { expansion_region_2_begin + expansion_region_2_size };


	// Creating it here for now as its likely not used anywhere
	std::array<std::byte, io_ports_size> io_ports {};

	static constexpr std::array<uint32_t, 8> region_mask {
		// KUSEG
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		// KSEG0
		0x7fffffff,
		// KSEG1,
		0x1fffffff,
		// KSEG2
		0xffffffff, 0xffffffff
	};
};
