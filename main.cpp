#include "Bios.h"
#include "Bus.h"
#include "Ram.h"
#include "Cpu.h"

int main() {
	Bios bios { "scph1001.bin" };   
	Ram memory {};

	Bus bus { bios, memory };

	Cpu cpu { bus };

	cpu.fetch_decode_execute();
	
	return 0;
}
