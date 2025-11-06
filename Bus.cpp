#include "Bus.h"

#include <span>

#include "Logger.h"
#include "Memory.h"

std::span<const std::byte> Bus::read_memory(uint32_t address, uint32_t bytes) const {
	uint32_t physical_address { to_physical_address(address) };
	if (Memory::Map::bios.contains(physical_address)){
		return m_bios.read(Memory::Map::bios.offset(physical_address), bytes);
	}

	if (Memory::Map::ram.contains(physical_address)) {
		return m_ram.read(Memory::Map::ram.offset(physical_address), bytes);
	}

	if (Memory::Map::gpu.contains(physical_address)) {
		std::stringstream ss;
		ss << "[BUS] Requesting GPU response at address (0x" << std::hex << physical_address << ")";
		Logger::log(Logger::Level::warning, ss.str());
		return m_gpu.get_response(physical_address);
	}

	if (Memory::Map::irq_control.contains(physical_address)) {
		std::stringstream ss;
		ss << "[BUS] IRQ Control: reading 0x" << std::hex << physical_address;
		Logger::log(Logger::Level::info, ss.str());
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::timers.contains(physical_address)) {
		std::stringstream ss;
		ss << "[BUS] Timer: reading 0x" << std::hex << physical_address;
		Logger::log(Logger::Level::info, ss.str());
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::dma.contains(physical_address)) {
		Logger::log(Logger::Level::info, "[BUS] Reading DMA.");
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::cache_control.contains(physical_address)) {
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::expansion_region_1.contains(physical_address)) {
		return std::as_bytes(std::span{ &m_no_expansion, 1 });
	}

	if (Memory::Map::expansion_region_2.contains(physical_address)) {
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::mem_control_1.contains(physical_address)) {
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::mem_control_2.contains(physical_address)) {
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	if (Memory::Map::spu.contains(physical_address)) {
		return std::as_bytes(std::span{ &dummy_variable, 1});
	}

	// If we have made it this far, then there is a read to an unknown area of memory
	std::stringstream ss;
	ss << "[BUS] Unknown read: " << std::hex << physical_address;
	Logger::log(Logger::Level::error, ss.str());
	std::exit(1);
}

void Bus::write_memory(uint32_t address, std::span<const std::byte> data) {
	uint32_t physical_address { to_physical_address(address) };
	if (Memory::Map::bios.contains(physical_address)){
		Logger::log(Logger::Level::error, "[BUS] Illegal write to Read Only Memory");
	} else if (Memory::Map::ram.contains(physical_address)) {
		m_ram.write(Memory::Map::ram.offset(physical_address), data);
	} else if (Memory::Map::gpu.contains(physical_address)) {
		std::stringstream ss;
		ss << "[BUS] Writing GPU address (0x" << std::hex << physical_address << ")";
		uint32_t word {};
		std::memcpy(&word, data.data(), sizeof(word));
		ss << " Command: 0x" << word;
		Logger::log(Logger::Level::warning, ss.str());

		m_gpu.receive_command(word);
	} else if (Memory::Map::irq_control.contains(physical_address)) {
		std::stringstream ss;
		ss << "[BUS] IRQ Control: Ignoring write to 0x" << std::hex << physical_address;
		Logger::log(Logger::Level::info, ss.str());
	} else if (Memory::Map::timers.contains(physical_address)) {
		std::stringstream ss;
		ss << "[BUS] Timer: Ignoring write to 0x" << std::hex << physical_address;
		Logger::log(Logger::Level::info, ss.str());
	} else if (Memory::Map::dma.contains(physical_address)) {
		Logger::log(Logger::Level::warning, "[BUS] Ignoring write to dma.");
	} else if (Memory::Map::cache_control.contains(physical_address)) {
		Logger::log(Logger::Level::warning, "[BUS] Ignoring write to cache control");
	} else if (Memory::Map::expansion_region_1.contains(physical_address)) {
		Logger::log(Logger::Level::warning, "[BUS] Ignoring write to expansion region 1");
	} else if (Memory::Map::expansion_region_2.contains(physical_address)) {
		Logger::log(Logger::Level::warning, "[BUS] Ignoring write to expansion region 2");
	} else if (Memory::Map::mem_control_1.contains(physical_address)) {
	} else if (Memory::Map::mem_control_2.contains(physical_address)) {
	} else if (Memory::Map::spu.contains(physical_address)) {
	} else {
		std::stringstream ss;
		ss << "Write to unknown memory region" << '(' << std::hex << physical_address << ')';
		Logger::log(Logger::Level::error, ss.str());
		std::exit(1);
	}
}

// Translates the virtual memory address into its physical memory address
uint32_t Bus::to_physical_address(uint32_t virtual_address) {
	uint32_t kseg { virtual_address >> 29 };
	uint32_t physical_address { virtual_address & region_mask[kseg] };
	return physical_address;
}