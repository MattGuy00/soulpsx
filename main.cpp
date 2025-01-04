#include "Bios.h"
#include "Instruction.h"

#include <cstdint>
#include <iostream>
#include <fstream>
#include <array>
#include <bitset>

int main() {
	Bios bios { "scph1001.bin" };   


	Instruction instr { bios.fetch_32(0)};
	std::cout << std::hex << "lui " <<  instr.rt() << ", " << instr.imm16() << '\n';
	
	std::cout << std::bitset<32>{bios.fetch_32(0)} << '\n';

	return 0;
}
