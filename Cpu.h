#pragma once

#include "Bus.h"
#include "Instruction.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

class Cpu {
public:
	Cpu(Bus& bus): m_bus { bus }
	{
	}

	void fetch_decode_execute();
private:
	Bus& m_bus;
	Instruction m_next_instruction { 0 };
	uint32_t m_pc { 0xbfc00000 };
	std::array<uint32_t, 32> m_registers {};

	std::span<const std::byte> read_memory(uint32_t offset, uint32_t bytes);
	void write_memory(std::span<const std::byte> data, uint32_t offset);

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
	
	void set_register(Register reg, uint32_t data);
	uint32_t get_register_data(Register reg);
	Register to_register(uint32_t reg_num) { return static_cast<Register>(reg_num); }
	std::string_view register_name(Register reg);
	
	std::optional<Register> m_load_delay_register {};
	std::optional<uint32_t> m_load_delay_data {};

	std::optional<Register> m_load_completed_register {};
	std::optional<uint32_t> m_load_completed_data {};


	void op_lui(const Instruction& instruction);
	void op_ori(const Instruction& instruction);
	void op_sw(const Instruction& instruction);
	void op_sll(const Instruction& instruction);
	void op_addiu(const Instruction& instruction);
	void op_jump(const Instruction& instruction);
	void op_or(const Instruction& instruction);
	void op_mtc0(const Instruction& instruction);
	void op_bne(const Instruction& instruction);
	void op_addi(const Instruction& instruction);
	void op_lw(const Instruction& instruction);
	void op_sltu(const Instruction& instruction);
	void op_addu(const Instruction& instruction);
};
