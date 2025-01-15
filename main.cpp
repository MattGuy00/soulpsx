#include "Bios.h"
#include "Bus.h"
#include "Instruction.h"

#include <iostream>
#include <bitset>

int main() {
	Bios bios { "scph1001.bin" };   
	Bus bus { bios };

	Instruction instr { bios.fetch_32(0)};
	std::cout << std::hex << instr << '\n';
	
	std::cout << std::bitset<32>{bios.fetch_32(0)} << '\n';

	return 0;
}
