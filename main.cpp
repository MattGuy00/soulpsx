#include "Bios.h"
#include "Bus.h"
#include "Ram.h"
#include "Cpu.h"


int main() {
	Bios bios { "scph1001.bin" };   
	Ram memory {};
	Bus bus { bios, memory };

	Cpu cpu { bus };

	memory.write(0x0, bios.read(0, 64 * 1024));
	cpu.fetch_decode_execute();
	
	return 0;
}
