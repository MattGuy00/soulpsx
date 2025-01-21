#include "Bios.h"
#include "Bus.h"
#include "Instruction.h"
#include "Memory.h"

#include <iostream>
#include <bitset>

int main() {
	Bios bios { "scph1001.bin" };   
	Memory memory {};
	Bus bus { bios, memory };

	memory.write_data(bios.first_64k());
	Instruction instr { memory.read_data(0) };
	std::cout << std::hex << instr << '\n';
	
	std::cout << std::bitset<32>{bios.fetch_32(0)} << '\n';

	return 0;
}
