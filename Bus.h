#pragma once

#include "Bios.h"
#include "Ram.h"

#include <cstdint>
#include <span>

#include "Gpu.h"

struct Bus {
	std::span<const std::byte> read_memory(uint32_t address, uint32_t bytes = 0) const;
	void write_memory(uint32_t address, std::span<const std::byte> data);

	static uint32_t to_physical_address(uint32_t virtual_address);

	Bios& m_bios;
	Ram& m_ram;
	Gpu& m_gpu;

	static constexpr uint8_t m_no_expansion { 0xff };
	uint32_t dummy_variable {};

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
