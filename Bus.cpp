#include "Bus.h"

#include <cstdint>
#include <cstdlib>
#include <span>

std::span<const std::byte> Bus::read_memory(uint32_t address, uint32_t bytes) const {
	uint32_t physical_address { to_physical_address(address) };

	if (physical_address == 0x1f8010f0) {
		std::cout << "GPU send / recieve" << '\n';
		std::exit(0);
	}

	if (physical_address == 0x1f801814) {
		std::cout << "GPU send / recieve" << '\n';
		std::exit(0);
	}

	Region region { get_region(address) };
	using enum Region;
	switch (region) {
		case bios: {
			return m_bios.read(physical_address - bios_memory_begin, bytes);
		}
		case ram: {
			return m_ram.read(physical_address - ram_begin, bytes);
		}
		case io_ports: {
			std::cout << "I/O ports: " << std::hex << physical_address << '\n';
			return std::span{ m_io_ports }.subspan(physical_address - io_ports_begin, bytes);
		}
		case cache_control: {
			return {};
		}
		case expansion_1: {
			return std::as_bytes(std::span{ &m_no_expansion, 1 });
		}
		case expansion_2: {
			return {};
		}
		default: {
			std::cout << "unknown region\n";
			std::exit(1);
		}
	}
}

std::span<const std::byte> Bus::read_memory(Region memory_region, uint32_t offset, uint32_t bytes) const {
	using enum Region;
	switch (memory_region) {
		case bios: {
			return m_bios.read(offset, bytes);
		}
		case ram: {
			return m_ram.read(offset, bytes);
		}
		case io_ports: {
			return std::span{ m_io_ports }.subspan(offset, bytes);
		}
		case cache_control: {
			return {};
		}
		case expansion_1: {
			return std::as_bytes(std::span{ &m_no_expansion, 1 });
		}
		case expansion_2: {
			return {};
		}
		default: {
			std::cout << "unknown region\n";
			std::exit(1);
		}
	}
}

void Bus::write_memory(uint32_t address, std::span<const std::byte> data) {
	uint32_t physical_address { to_physical_address(address) };

	Region region { get_region(address) };
	using enum Region;
	switch (region) {
		case bios: {
			std::cout << "Illegal write to Read Only Memory (Bios)\n";
			break;
		}
		case ram: {
			m_ram.write(physical_address - ram_begin, data);
			break;
		}
		case io_ports: {
			//std::ranges::copy(data, m_io_ports.begin() + (address - io_ports_begin));
			break;
		}
		case cache_control: {
			std::cout << "Ignoring write to cache control\n";
			break;
		}
		case expansion_1: {
			std::cout << "Ignoring write to expansion region 1\n";
			break;
		}
		case expansion_2: {
			std::cout << "Ignoring write to expansion region 2\n";
			break;
		}
		default: {
			std::cout << "Write to unknown region " << "( " << std::hex << physical_address << ")\n";
			std::exit(1);
		}
	}
}

// Translates the virtual memory address into its physical memory address
uint32_t Bus::to_physical_address(uint32_t virtual_address) {
	uint32_t kseg { virtual_address >> 29 };
	uint32_t physical_address { virtual_address & region_mask[kseg] };
	return physical_address;
}

Region Bus::get_region(uint32_t virtual_address) const {
	uint32_t physical_address { to_physical_address(virtual_address) };

	if (physical_address >= bios_memory_begin && physical_address < bios_memory_end) {
		return Region::bios;
	}

	if (physical_address >= ram_begin && physical_address < ram_end) {
		return Region::ram;
	}

	if (physical_address >= io_ports_begin && physical_address < io_ports_end) {
		return Region::io_ports;
	}

	if (physical_address >= cache_control_begin && physical_address < cache_control_end) {
		return Region::cache_control;
	}

	if (physical_address >= expansion_region_1_begin && physical_address < expansion_region_1_end) {
		return Region::expansion_1;
	}

	if (physical_address >= expansion_region_2_begin && physical_address < expansion_region_2_end) {
		return Region::expansion_2;
	}

	return Region::unknown;
}

uint32_t Bus::get_region_start(Region region) {
	using enum Region;
	switch (region) {
		case bios: return bios_memory_begin;
		case ram: return ram_begin;
		case io_ports: return io_ports_begin;
		case cache_control: return cache_control_begin;
		case expansion_1: return expansion_region_1_begin;
		case expansion_2: return expansion_region_2_begin;
		case unknown: return 0;
	}

	return 0;
}

uint32_t Bus::get_region_end(Region region) {
	using enum Region;
	switch (region) {
		case bios: return bios_memory_end;
		case ram: return ram_end;
		case io_ports: return io_ports_end;
		case cache_control: return cache_control_end;
		case expansion_1: return expansion_region_1_end;
		case expansion_2: return expansion_region_2_end;
		case unknown: return 0;
	}
	return 0;
}

uint32_t Bus::get_relative_offset(uint32_t virtual_address) const {
	uint32_t physical_address { to_physical_address(virtual_address) };
	Region region { get_region(virtual_address) };
	using enum Region;
	switch (region) {
		case bios: {
			return physical_address - bios_memory_begin;
		}
		case ram: {
			return physical_address - ram_begin;
		}
		case io_ports: {
			return physical_address - io_ports_begin;
		}
		case cache_control: {
			return physical_address - cache_control_begin;
		}
		case expansion_1: {
			return physical_address - expansion_region_1_begin;
		}
		case expansion_2: {
			return physical_address - expansion_region_2_begin;
		}
		case unknown: {
			return virtual_address;
		}

	}
	return virtual_address;
}

std::string_view Bus::region_name(Region region) {
	using enum Region;
	switch (region) {
		case bios: return "bios";
		case ram: return "ram";
		case io_ports: return "io_ports";
		case cache_control: return "cache_control";
		case expansion_1: return "expansion_1";
		case expansion_2: return "expansion_2";
		default: return "unknown";
	}
}
