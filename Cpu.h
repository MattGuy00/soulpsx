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

	uint32_t cop0_sr {};

	std::span<const std::byte> read_memory(uint32_t address, uint32_t bytes);
	void write_memory(uint32_t address, std::span<const std::byte> data);

	// To be used with load delay
	Register m_overwritten_reg {};
	
	void set_register(Register reg, uint32_t data);
	uint32_t get_register_data(Register reg);
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
	void op_sh(const Instruction& instruction);
	void op_jal(const Instruction& instruction);
	void op_andi(const Instruction& instruction);
	void op_sb(const Instruction& instruction);
	void op_jr(const Instruction& instruction);
};
