#include "Instruction.h"
#include <bitset>
#include <cstdint>


std::string_view Instruction::type_string() const {
	using enum Opcode;
	switch (m_opcode) {
		case andi: return "andi";
		case and_b: return "and";
		case or_b: return "or";
		case ori: return "ori";
		case addiu: return "addiu";
		case addi: return "addi";
		case addu: return "addu";
		case add: return "add";
		case sltu: return "sltu";
		case sll: return "sll";
		case sh: return "sh";
		case sb: return "sb";
		case lui: return "lui";
		case lw: return "lw";
		case lb: return "lb";
		case lbu: return "lbu";
		case sw: return "sw";
		case jump: return "jump";
		case jal: return "jal";
		case jr: return "jr";
		case bne: return "bne";
		case beq: return "beq";
		case bgtz: return "bgtz";
		case blez: return "blez";
		case mtc0: return "mtc0";
		case mfc0: return "mfc0";
		default: return "unknown";
	}
}

Instruction::Opcode Instruction::determine_opcode(uint32_t data) {
	uint8_t primary_opcode { static_cast<uint8_t>(data >> 26) };

	using enum Opcode;
	switch (primary_opcode) {
		case 0b001100: return andi;
		case 0b001101: return ori;
		case 0b001001: return addiu;
		case 0b001000: return addi;
		case 0b001111: return lui;
		case 0b100011: return lw;
		case 0b100000: return lb;
		case 0b100100: return lbu;
		case 0b101011: return sw;
		case 0b101001: return sh;
		case 0b101000: return sb;
		case 0b000010: return jump;
		case 0b000011: return jal;
		case 0b000101: return bne;
		case 0b000100: return beq;
		case 0b000111: return bgtz;
		case 0b000110: return blez;
		// Special instruction. So we check secondary opcode
		case 0b000000: {
			uint8_t secondary_opcode { static_cast<uint8_t>(data & 0b111111) };
			switch (secondary_opcode) {
				case 0b000000: return sll;
				case 0b100100: return and_b;
				case 0b100101: return or_b;
				case 0b101011: return sltu;
				case 0b100001: return addu;
				case 0b100000: return add;
				case 0b001000: return jr;
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
				case 0b00000: return mfc0;
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
