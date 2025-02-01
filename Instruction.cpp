#include "Instruction.h"
#include <bitset>
#include <cstdint>


std::string_view Instruction::type_string() const {
	using enum Opcode;
	switch (m_opcode) {
		case or_b: return "or";
		case ori: return "ori";
		case addiu: return "addiu";
		case addi: return "addi";
		case sll: return "sll";
		case lui: return "lui";
		case lw: return "lw";
		case sw: return "sw";
		case jump: return "jump";
		case bne: return "bne";

		case mtc0: return "mtc0";

		default: return "unknown";
	}
}

Instruction::Opcode Instruction::determine_opcode(uint32_t data) {
	uint8_t primary_opcode { static_cast<uint8_t>(data >> 26) };

	using enum Opcode;
	switch (primary_opcode) {
		case 0b001101: return ori;
		case 0b001001: return addiu;
		case 0b001000: return addi;
		case 0b001111: return lui;
		case 0b100011: return lw;
		case 0b101011: return sw;
		case 0b000010: return jump;
		case 0b000101: return bne;
		// Special instruction. So we check secondary opcode
		case 0b000000: {
			uint8_t secondary_opcode { static_cast<uint8_t>(data & 0b111111) };
			switch (secondary_opcode) {
				case 0b000000: return sll;
				case 0b100101: return or_b;
				default:
					std::cout << "Special: ";
					std::cout << std::bitset<6>{ secondary_opcode } << '\n';
					return unknown;
			}
		}
		// Cop0 instruction. Check next 5 bits
		case 0b010000: {
			uint8_t cop0_opcode { static_cast<uint8_t>((data >> 21) & 0b111111) };
			switch (cop0_opcode) {
				case 0b00100: return mtc0;
				default:
					std::cout << "cop0 opcode: ";
					std::cout << std::bitset<6>{ cop0_opcode } << '\n';
					return unknown;
			}
		}
		default: 
			std::cout << std::bitset<6>{ primary_opcode } << '\n';
			return unknown;
	}
}
