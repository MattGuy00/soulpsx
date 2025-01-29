#include "Instruction.h"
#include <bitset>
#include <cstdint>


std::string_view Instruction::type_string() const {
	using enum Opcode;
	switch (m_opcode) {
		case ori: return "ori";
		case addiu: return "addiu";
		case sll: return "sll";
		case lui: return "lui";
		case lw: return "lw";
		case sw: return "sw";
		default: return "unknown";
	}
}

Instruction::Opcode Instruction::determine_opcode(uint32_t data) {
	uint8_t primary_opcode { static_cast<uint8_t>(data >> 26) };

	using enum Opcode;
	switch (primary_opcode) {
		case 0b001101: return ori;
		case 0b001001: return addiu;
		case 0b001111: return lui;
		case 0b110001: return lw;
		case 0b101011: return sw;
		// Special instruction. So we check secondary opcode
		case 0b000000: {
			uint8_t secondary_opcode { static_cast<uint8_t>(data & 0b111111) };
			switch (secondary_opcode) {
				case 0b000000: return sll;
			}
		}
		default: 
			std::cout << std::bitset<8>{ primary_opcode } << '\n';
			return unknown;
	}
}
