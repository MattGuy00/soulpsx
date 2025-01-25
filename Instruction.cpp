#include "Instruction.h"
#include <bitset>
#include <cstdint>


std::string_view Instruction::type_string() const {
	using enum Opcode;
	switch (m_opcode) {
		case ori: return "ori";
		case sw: return "sw";
		case lui: return "lui";
		case lw: return "lw";
//		case bne: return "bne";
		default: return "unknown";
	}
}

Instruction::Opcode Instruction::get_opcode(uint8_t identifier) {
	using enum Opcode;
	switch (identifier) {
		case 0b001101: return ori;
		case 0b101011: return sw;
		case 0b001111: return lui;
		case 0b110001: return lw;
//		case 0b000101: return bne;
		default: 
			std::cout << std::bitset<8>{ identifier } << '\n';
			return unknown;
	}
}
