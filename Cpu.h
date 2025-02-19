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
	uint32_t m_pc { 0xbfc00000 };
	uint32_t m_next_pc { m_pc + 4};
	std::array<uint32_t, 32> m_registers {};
	std::array<uint32_t, 32> m_temp_registers {};

	uint32_t m_hi {};
	uint32_t m_lo {};

	uint32_t cop0_sr {};

	std::span<const std::byte> read_memory(uint32_t address, uint32_t bytes);
	void write_memory(uint32_t address, std::span<const std::byte> data);

	
	void set_register(Register reg, uint32_t data);
	uint32_t get_register_data(Register reg);
	std::string_view register_name(Register reg);
	
	// Delay moving data from memory into registers by 1 cycle
	struct Load_delay {
		Register reg {};
		uint32_t data {};
	};
	
	std::optional<Load_delay> m_load_delay_slot {};

	void load_delay_data(Register reg, uint32_t data);

	void branch(uint32_t offset);

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
	void op_lb(const Instruction& instruction);
	void op_beq(const Instruction& instruction);
	void op_mfc0(const Instruction& instruction);
	void op_and(const Instruction& instruction);
	void op_add(const Instruction& instruction);
	void op_bgtz(const Instruction& instruction);
	void op_blez(const Instruction& instruction);
	void op_lbu(const Instruction& instruction);
	void op_jalr(const Instruction& instruction);
	void op_bltz(const Instruction& instruction);
	void op_slti(const Instruction& instruction);
	void op_subu(const Instruction& instruction);
	void op_sra(const Instruction& instruction);
	void op_div(const Instruction& instruction);
	void op_mflo(const Instruction& instruction);
	void op_bgez(const Instruction& instruction);
	void op_srl(const Instruction& instruction);
	void op_sltiu(const Instruction& instruction);
	void op_divu(const Instruction& instruction);
	void op_mfhi(const Instruction& instruction);
	void op_slt(const Instruction& instruction);
};
