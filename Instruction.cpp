#include "Instruction.h"
#include <bitset>
#include <cstdint>
#include <unistd.h>

std::string_view Instruction::type_string() const {
	using enum Opcode;
	switch (m_opcode) {
		case andi: return "andi";
		case and_b: return "and";
		case or_b: return "or";
		case ori: return "ori";
		case nor: return "nor";
		case Xor: return "xor";
		case addiu: return "addiu";
		case addi: return "addi";
		case addu: return "addu";
		case add: return "add";
		case subu: return "subu";
		case div: return "div";
		case divu: return "divu";
		case multu: return "multu";
		case slt: return "slt";
		case sltu: return "sltu";
		case slti: return "slti";
		case sltiu: return "sltiu";
		case sll: return "sll";
		case srl: return "srl";
		case sllv: return "sllv";
		case srav: return "srav";
		case srlv: return "srlv";
		case sh: return "sh";
		case sb: return "sb";
		case sra: return "sra";
		case lui: return "lui";
		case lw: return "lw";
		case lb: return "lb";
		case lbu: return "lbu";
		case lhu: return "lhu";
		case lwr: return "lwr";
		case lh: return "lh";
		case sw: return "sw";
		case jump: return "jump";
		case jal: return "jal";
		case jr: return "jr";
		case jalr: return "jalr";
		case bne: return "bne";
		case beq: return "beq";
		case bgtz: return "bgtz";
		case bgez: return "bgez";
		case blez: return "blez";
		case bltz: return "bltz";
		case mflo: return "mflo";
		case mfhi: return "mfhi";
		case mtlo: return "mtlo";
		case mthi: return "mthi";
		case mtc0: return "mtc0";
		case mfc0: return "mfc0";
		case syscall: return "syscall";
		case rfe: return "rfe";
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
		case 0b001010: return slti;
		case 0b001011: return sltiu;
		case 0b100011: return lw;
		case 0b100000: return lb;
		case 0b100100: return lbu;
		case 0b100101: return lhu;
		case 0b100001: return lh;
		case 0b100110: return lwr;
		case 0b101011: return sw;
		case 0b101001: return sh;
		case 0b101000: return sb;
		case 0b000010: return jump;
		case 0b000011: return jal;
		case 0b000101: return bne;
		case 0b000100: return beq;
		case 0b000111: return bgtz;
		case 0b000110: return blez;
		// Certain branch instructions. Need to check bits 16 to 20
		case 0b000001: {
			uint8_t branch_opcode { static_cast<uint8_t>(data >> 16 & 0b11111) };
			switch (branch_opcode) {
				case 0b00000: return bltz;
				case 0b00001: return bgez;
				default: ;
			}
		}
		// Special instruction. So we check secondary opcode
		case 0b000000: {
			uint8_t secondary_opcode { static_cast<uint8_t>(data & 0b111111) };
			switch (secondary_opcode) {
				case 0b000000: return sll;
				case 0b000010: return srl;
				case 0b000100: return sllv;
				case 0b000111: return srav;
				case 0b000110: return srlv;
				case 0b000011: return sra;
				case 0b100100: return and_b;
				case 0b100101: return or_b;
				case 0b100111: return nor;
				case 0b100110: return Xor;
				case 0b101010: return slt;
				case 0b101011: return sltu;
				case 0b100001: return addu;
				case 0b100000: return add;
				case 0b100011: return subu;
				case 0b011010: return div;
				case 0b011011: return divu;
				case 0b011001: return multu;
				case 0b001000: return jr;
				case 0b001001: return jalr;
				case 0b010010: return mflo;
				case 0b010000: return mfhi;
				case 0b010011: return mtlo;
				case 0b010001: return mthi;
				case 0b001100: return syscall;
				default:
					return unknown;
			}
		}
		// Cop0 instruction. Check next 5 bits
		case 0b010000: {
			uint8_t cop0_opcode { static_cast<uint8_t>((data >> 21) & 0b111111) };
			switch (cop0_opcode) {
				case 0b00100: return mtc0;
				case 0b00000: return mfc0;
			}

			switch (data & 0b111111) {
				case 0b010000: return rfe;
			}

			return unknown;
		}
		default: 
			return unknown;
	}
}
