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

enum class Cop0_Register {
	// Breakpoint on execute
	bpc = 3,

	// Breakpoint on data access
	bda = 5,

	// Randomly memorised jump address
	jumpdest = 6,

	// Breakpoint control
	dcic = 7,

	// Bad Virtual Address
	badvaddr = 8,

	// Data access breakpoint mask
	bdam = 9,

	// Execute breakpoint mask
	bpcm = 11,

	// System status 
	sr = 12,

	// Describes the most recently recognised exception
	cause = 13,

	// Return address from trap
	epc = 14,

	// Processor ID
	prid = 15,
	
	// Catch all for all unused / unknown registers
	unused
};

class Instruction {
public:
	enum class Opcode {
		mtc0,
		mfc0,

		mflo,
		mfhi,
		mtlo,
		mthi,
		andi,
		and_b,
		or_b,
		ori,
		nor,
		Xor,
		addiu,
		addi,
		addu,
		subu,
		div,
		divu, 
		multu,
		slt,
		sltu,
		slti,
		sltiu,
		add,
		sll,
		srl,
		sllv,
		sra,
		srav,
		srlv,
		sh,
		sb,
		lui,
		lw,
		lb,
		lbu,
		lhu,
		lh,
		lwr,
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
		syscall,
		rfe,
		unknown,
	};

	explicit Instruction(std::span<const std::byte> data, uint32_t pc);
	explicit Instruction();

	explicit Instruction(uint32_t data) : m_data { data }, m_opcode { determine_opcode(data) } {}
	
	Register rs() const { return static_cast<Register>((m_data >> 21) & 0b11111); }
	Register base() const { return static_cast<Register>((m_data >> 21) & 0b11111); }
	Register rt() const { return static_cast<Register>((m_data >> 16) & 0b11111); }
	Register rd() const { return static_cast<Register>((m_data >> 11) & 0b11111); }
	Cop0_Register cop0_rd() const { return static_cast<Cop0_Register>((m_data >> 11) & 0b11111); }

	uint32_t sa() const { return (m_data >> 6) & 0b11111; }

	uint32_t imm16() const { return m_data & 0xffff; }
	// Note the order of casts. This ensures that a sign extending instruction is 
	// is generated.
	uint32_t imm16_se() const { 
		return static_cast<uint32_t>(static_cast<int16_t>(m_data & 0xffff));
	}


	// Returns a 26 bit index for the jump instruction
	uint32_t jump_addr() const { return m_data & 0x3ffffff; }

	// Returns a type based on the 5 bit identifier
	Opcode opcode() const { return m_opcode; }
	std::string_view opcode_as_string() const;

	uint32_t data() const { return m_data; }
	std::string as_hex() const;

	std::string_view as_string() const { return m_str_representation; }
	void to_string(uint32_t pc);

	void instruction_to_string(const std::vector<std::string_view> &values);
	static std::string_view cop0_register_name(Cop0_Register reg) {
		using enum Cop0_Register;
		switch (reg) {
			case bpc: return "bpc";
			case bda: return "bda";
			case jumpdest: return "jumpdest";
			case dcic: return "dcic";
			case badvaddr: return "badvaddr";
			case bdam: return "bdam";
			case bpcm: return "bpcm";
			case sr: return "sr";
			case cause: return "cause";
			case epc: return "epc";
			case prid: return "prid";
			default: return "unused";
		}
	}

	static std::string_view register_name(Register reg) {
		using enum Register;
		switch (reg) {
			case zero: return "r0";
			case at: return "at";
			case v0: return "v0";
			case v1: return "v1";
			case a0: return "a0";
			case a1: return "a1";
			case a2: return "a2";
			case a3: return "a3";
			case t0: return "t0";
			case t1: return "t1";
			case t2: return "t2";
			case t3: return "t3";
			case t4: return "t4";
			case t5: return "t5";
			case t6: return "t6";
			case t7: return "t7";
			case t8: return "t8";
			case t9: return "t9";
			case s0: return "s0";
			case s1: return "s1";
			case s2: return "s2";
			case s3: return "s3";
			case s4: return "s4";
			case s5: return "s5";
			case s6: return "s6";
			case s7: return "s7";
			case k0: return "k0";
			case k1: return "k1";
			case gp: return "gp";
			case sp: return "sp";
			case fp: return "fp";
			case ra: return "ra";
			default: return "unknown";
		}
	}

	friend std::ostream& operator<<(std::ostream& out, const Instruction& instruction) {
		out << instruction.opcode_as_string();
		return out;
	}

	static constexpr uint8_t instruction_length { 4 };
private:
	uint32_t m_data {};
	Opcode m_opcode {};
	std::string m_str_representation {};

	Instruction::Opcode determine_opcode(uint32_t data);
};
