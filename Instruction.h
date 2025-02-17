#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>

// Register names are from 
// https://psx-spx.consoledev.net/cpuspecifications/
enum class Register {
	// constant (0)
	zero = 0,

	// Assembler Temporary
	at,

	// Subroutine return values
	v0, v1,

	// Subroutine args
	a0, a1, a2,	a3,

	// Temporaries
	t0, t1, t2, t3, t4, t5, t6, t7,

	// Static variables
	s0, s1, s2, s3, s4, s5, s6, s7,

	// Temporaries
	t8, t9,

	// For kernel
	k0, k1,

	// Global pointer
	gp,

	// Stack pointer
	sp,

	// Frame Pointer or static variable
	fp,

	// Return address
	ra,
};

class Instruction {
public:
	enum class Opcode {
		mtc0,
		mfc0,

		mflo,
		andi,
		and_b,
		or_b,
		ori,
		addiu,
		addi,
		addu,
		subu,
		div,
		divu, 
		sltu,
		slti,
		sltiu,
		add,
		sll,
		srl,
		sh,
		sb,
		sra,
		lui,
		lw,
		lb,
		lbu,
		sw,
		jump,
		jal,
		jr,
		jalr,
		bne,
		beq,
		bgtz,
		bgez,
		blez,
		bltz,
		unknown,
	};

	explicit Instruction(std::span<const std::byte> data) {
		memcpy(&m_data, data.data(), sizeof(int));
		m_opcode = determine_opcode(m_data);
	}

	explicit Instruction(uint32_t data) : m_data { data }, m_opcode { determine_opcode(data) } {}
	
	Register rs() const { return static_cast<Register>((m_data >> 21) & 0b11111); }
	Register rt() const { return static_cast<Register>((m_data >> 16) & 0b11111); }
	Register rd() const { return static_cast<Register>((m_data >> 11) & 0b11111); }
	Register base() const { return static_cast<Register>((m_data >> 21) & 0b11111); }
	
	uint32_t sa() const { return (m_data >> 6) & 0b11111; }

	uint32_t imm16() const { return m_data & 0xffff; }
	// Note the order of casts. This ensures that a sign extending instruction is 
	// is generated.
	uint32_t imm16_se() const { 
		return static_cast<uint32_t>(static_cast<short>(m_data & 0xffff));
	}


	// Returns a 26 bit index for the jump instruction
	uint32_t jump_addr() const { return m_data & 0x3ffffff; }

	// Returns a type based on the 5 bit identifier
	Opcode opcode() const { return m_opcode; }
	std::string_view type_string() const;

	uint32_t data() const { return m_data; }
	
	friend std::ostream& operator<<(std::ostream& out, const Instruction& instruction) {
		out << instruction.type_string();	
		return out;
	}

private:
	uint32_t m_data {};
	Opcode m_opcode {};

	Instruction::Opcode determine_opcode(uint32_t data);
};
