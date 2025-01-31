#include "Bios.h"
#include "Bus.h"
#include "Memory.h"
#include "Cpu.h"


int main() {
	Bios bios { "scph1001.bin" };   
	Memory memory {};
	Bus bus { bios, memory };

	Cpu cpu { bus };

	memory.write_data(bios.read(0, 64 * 1024));
	cpu.fetch_decode_execute();
	
	return 0;
}
