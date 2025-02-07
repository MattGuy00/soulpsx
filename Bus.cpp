#include "Bus.h"

#include <cstdint>
#include <cstdlib>

std::span<const std::byte> Bus::read_memory(uint32_t address, uint32_t bytes = 0) {
	uint32_t physical_address { to_physical_address(address) };

	if (address % 4 != 0) {
		std::cout << "Unaligned address exception (" << std::hex;
		std::cout << physical_address << ")\n";
	}

	if (physical_address >= bios_memory_begin && physical_address < bios_memory_end) {
		return bios.read(physical_address - bios_memory_begin, bytes);
	} else if (physical_address >= ram_begin && physical_address < ram_end) {
		return ram.read(physical_address - ram_begin, bytes);
	} else if (physical_address >= io_ports_begin && physical_address < io_ports_end) {
		return std::span{ io_ports }.subspan(physical_address - io_ports_begin, bytes);
	} else if (physical_address >= cache_control_begin && physical_address < cache_control_end) {
		std::cout << "cache control\n";
	} else {
		std::cout << "Read to unknown region\n";
		std::exit(1);
	}

	return std::span<const std::byte>();
}

void Bus::write_memory(uint32_t address, std::span<const std::byte> data) {
	uint32_t physical_address { to_physical_address(address) };
	
	if (address % 4 != 0) {
		std::cout << "Unaligned address exception (" << std::hex;
		std::cout << physical_address << ")\n";
	}

	if (physical_address >= bios_memory_begin && physical_address < bios_memory_end) {
		std::cout << "Illegal write to Read Only Memory (Bios)\n";
	} else if (physical_address >= ram_begin && physical_address < ram_end) {
		ram.write(physical_address - ram_begin, data);
	} else if (physical_address >= io_ports_begin && physical_address < io_ports_end) {
		std::copy(data.begin(), data.end(), io_ports.begin() + address - io_ports_begin);
	} else if (physical_address >= cache_control_begin && physical_address < cache_control_end) {
		std::cout << "Ignoring write to cache control\n";
	} else if (physical_address >= expansion_region_2_begin && physical_address < expansion_region_2_end) { 
		std::cout << "Ignoring write to expansion region 2\n";
	} else {
		std::cout << "Write to unknown region\n";
		std::exit(1);
	}
}

// Translates the virtual memory address into its physical memory address
uint32_t Bus::to_physical_address(uint32_t virtual_address) {
	uint32_t kseg { virtual_address >> 29 };
	uint32_t physical_address { virtual_address & region_mask[kseg] };
	return physical_address;
}
