#pragma once

#include "Bios.h"
#include "Memory.h"

#include <cstdint>
#include <span>

struct Bus {
	Bios& bios;
	Memory& memory;

	std::span<const std::byte> read_memory(uint32_t offset, uint32_t bytes);
};
