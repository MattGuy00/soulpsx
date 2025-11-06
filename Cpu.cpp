#include "Cpu.h"
#include "Instruction.h"

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>

#include "Logger.h"

void Cpu::fetch_decode_execute() {
	m_current_pc = m_pc;
	if (m_current_pc % 4 != 0) {
		exception(Exception::load_address_error);
		return;
	}

	m_current_instruction = Instruction(read_memory(m_pc, 4), m_pc);

	m_pc = m_next_pc;
	m_next_pc += 4;

	m_is_branch_delay = m_was_branch;
	m_was_branch = false;

	// load the delayed data into the temp registers.
	// The next next instruction won't see the new values yet.
	// Also allows for the instruction to overwrite the delay slot's data
	// which is a thing
	if (m_load_delay_slot) {
		set_register(m_load_delay_slot->reg, m_load_delay_slot->data);
		m_load_delay_slot = std::nullopt;
	}

	using enum Instruction::Opcode;
	switch (m_current_instruction.opcode()) {
		case andi: {
			op_andi(m_current_instruction);
			break;
		}
		case and_b: {
			op_and(m_current_instruction);
			break;
		}
		case or_b: {
			op_or(m_current_instruction);
			break;
		}
		case ori: {
			op_ori(m_current_instruction);
			break;
		}
		case nor: {
			op_nor(m_current_instruction);
			break;
		}
		case Xor: {
			op_xor(m_current_instruction);
			// std::exit(1);
			break;
		}
		case addiu: {
			op_addiu(m_current_instruction);
			break;
		}
		case addi: {
			op_addi(m_current_instruction);
			break;
		}
		case addu: {
			op_addu(m_current_instruction);
			break;
		}
		case add: {
			op_add(m_current_instruction);
			break;
		}
		case subu: {
			op_subu(m_current_instruction);
			break;
		}
		case div: {
			op_div(m_current_instruction);
			break;
		}
		case divu: {
			op_divu(m_current_instruction);
			break;
		}
		case multu: {
			op_multu(m_current_instruction);
			break;
		}
		case slt: {
			op_slt(m_current_instruction);
			break;
		}
		case sltu: {
			op_sltu(m_current_instruction);
			break;
		}
		case slti: {
			op_slti(m_current_instruction);
			break;
		}
		case sltiu: {
			op_sltiu(m_current_instruction);
			break;
		}
		case sll: {
			op_sll(m_current_instruction);
			break;
		}
		case sllv: {
			op_sllv(m_current_instruction);
			break;
		}
		case srl: {
			op_srl(m_current_instruction);
			break;
		}
		case srav: {
			op_srav(m_current_instruction);
			break;
		}
		case srlv: {
			op_srlv(m_current_instruction);
			break;
		}
		case sra: {
			op_sra(m_current_instruction);
			break;
		}
		case lw: {
			op_lw(m_current_instruction);
			break;
		}
		case lb: {
			op_lb(m_current_instruction);
			break;
		}
		case lbu: {
			op_lbu(m_current_instruction);
			break;
		}
		case lhu: {
			op_lhu(m_current_instruction);
			break;
		}
		case lh: {
			op_lh(m_current_instruction);
			break;
		}
		case lwr: {
			op_lwr(m_current_instruction);
			break;
		}
		case sw: {
			op_sw(m_current_instruction);
			break;
		}
		case sh: {
			op_sh(m_current_instruction);
			break;
		}
		case sb: {
			op_sb(m_current_instruction);
			break;
		}
		case lui: {
			op_lui(m_current_instruction);
			break;
		}
		case jump: {
			op_jump(m_current_instruction);
			break;
		}
		case jal: {
			op_jal(m_current_instruction);
			break;
		}
		case jalr: {
			op_jalr(m_current_instruction);
			break;
		}
		case jr: {
			op_jr(m_current_instruction);
			break;
		}
		case bne: {
			op_bne(m_current_instruction);
			break;
		}
		case beq: {
			op_beq(m_current_instruction);
			break;
		}
		case bgtz: {
			op_bgtz(m_current_instruction);
			break;
		}
		case bgez: {
			op_bgez(m_current_instruction);
			break;
		}
		case blez: {
			op_blez(m_current_instruction);
			break;
		}
		case bltz: {
			op_bltz(m_current_instruction);
			break;
		}
		case mflo: {
			op_mflo(m_current_instruction);
			break;
		}
		case mfhi: {
			op_mfhi(m_current_instruction);
			break;
		}
		case mtlo: {
			op_mtlo(m_current_instruction);
			break;
		}
		case mthi: {
			op_mthi(m_current_instruction);
			break;
		}
		case mtc0: {
			op_mtc0(m_current_instruction);
			break;
		}
		case mfc0: {
			op_mfc0(m_current_instruction);
			break;
		}
		case syscall: {
			op_syscall(m_current_instruction);
			break;
		}
		case rfe: {
			op_rfe(m_current_instruction);
			break;
		}
		case unknown: {
			std::stringstream ss;
			ss << "[CPU] Unknown instruction: 0x" << std::hex << m_current_instruction.data();
			Logger::log(Logger::Level::error, ss.str());
			std::exit(1);
		}
	}

	// Update the registers
	m_registers = m_temp_registers;
	m_cop0_registers = m_cop0_temp_registers;
}

void Cpu::set_register(Register reg, uint32_t data) {
	m_temp_registers[static_cast<uint32_t>(reg)] = data;
	m_temp_registers[0] = 0;
}

uint32_t Cpu::get_register_data(Register reg) const {
	return m_registers[static_cast<uint32_t>(reg)];
}

void Cpu::cop0_set_register(Cop0_Register reg, uint32_t data) {
	m_cop0_temp_registers[static_cast<uint32_t>(reg)] = data;
	m_cop0_temp_registers[0] = 0;
}

uint32_t Cpu::cop0_get_register_data(Cop0_Register reg) const {
	return m_cop0_registers[static_cast<uint32_t>(reg)];
}

std::span<const std::byte> Cpu::read_memory(uint32_t address, uint32_t bytes) {
	return m_bus.read_memory(address, bytes);
}

void Cpu::write_memory(uint32_t address, std::span<const std::byte> data) {
	// Cache is isolated
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
		Logger::log(Logger::Level::warning, "[CPU] Cache isolated: Ignoring write");
		return;
	}
	m_bus.write_memory(address, data);
}

uint32_t to_32(std::span<const std::byte> data) {
	uint32_t word {};
	std::memcpy(&word, data.data(), sizeof(word));
	return word;
}

uint16_t to_16(std::span<const std::byte> data) {
	uint16_t h_word {};
	std::memcpy(&h_word, data.data(), sizeof(h_word));

	return h_word;
}

uint8_t to_8(std::span<const std::byte> data) {
	uint8_t byte {};
	std::memcpy(&byte, data.data(), sizeof(byte));

	return byte;
}

void Cpu::branch(uint32_t offset) {
	m_next_pc += offset;
	m_next_pc -= 4;

	m_was_branch = true;
}

void Cpu::load_delay_data(Register reg, uint32_t data) {
	m_load_delay_slot = { reg, data };
}

void Cpu::exception(Exception excode) {
	uint32_t sr { cop0_get_register_data(Cop0_Register::sr) };
	// Find exception handler address based on BEV bit
	uint32_t handler { (sr & (1 << 22)) ? 0xbfc00180 : 0x80000080 };

	uint32_t mode { sr & 0x3f };
	sr &= ~0x3f;
	sr |= (mode << 2) & 0x3f;
	cop0_set_register(Cop0_Register::sr, sr);

	uint32_t cause { static_cast<uint32_t>(excode) << 2 };

	if (m_is_branch_delay) {
		cause |= 1 << 31;

		// this is ok because we overwrite it with the handler
		m_current_pc -= 4;
	}

	// Store the cause of the exception in the cause register at bits 2:6
	cop0_set_register(Cop0_Register::cause, cause);
	cop0_set_register(Cop0_Register::epc, m_current_pc);

	// Immediately jump to the handler
	m_pc = handler;
	m_next_pc = m_pc + 4;

}

void Cpu::op_lwr(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	
	uint32_t latest_value { m_temp_registers[static_cast<uint32_t>(instruction.rt())] };

	uint32_t bytes { address & 0x3 };
	// Get the number of bytes we are out of alignment by and subtract it from size of a word
	uint32_t word { to_32(read_memory(address, 4 - bytes)) };

	// Shave off bytes
	latest_value >>= 32 - 8 * bytes;
	latest_value <<= 32 - 8 * bytes;
	latest_value |= word;
	
	load_delay_data(instruction.rt(), latest_value);

}

void Cpu::op_xor(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };

	set_register(instruction.rd(), rs_data ^ rt_data);
}

void Cpu::op_multu(const Instruction& instruction) {
	uint64_t rs_data { get_register_data(instruction.rs()) };
	uint64_t rt_data { get_register_data(instruction.rt()) };
	auto result { rs_data * rt_data };

	// Store first 32 bits in lo and the rest in hi
	m_lo = static_cast<uint32_t>(result);
	m_hi = static_cast<uint32_t>(result >> 32);
}

void Cpu::op_srlv(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t sa { get_register_data(instruction.rs()) & 0b11111 };

	set_register(instruction.rd(), rt_data >> sa);
}

void Cpu::op_srav(const Instruction& instruction) {
	int rt_data { static_cast<int>(get_register_data(instruction.rt())) };
	uint32_t sa { get_register_data(instruction.rs()) & 0b11111 };

	set_register(instruction.rd(), rt_data >> sa);
}

void Cpu::op_nor(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };

	set_register(instruction.rd(), !(rs_data | rt_data));
}

void Cpu::op_lh(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.rs()) + instruction.imm16_se() };
	if (address % 2 != 0) {
		exception(Exception::load_address_error);
		return;
	}

	load_delay_data(instruction.rt(), static_cast<int16_t>(to_16(read_memory(address, 2))));
}

void Cpu::op_sllv(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t sa { get_register_data(instruction.rs()) & 0b11111 };

	set_register(instruction.rd(), rt_data << sa);
}

void Cpu::op_lhu(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	if (address % 2 != 0) {
		exception(Exception::load_address_error);
		return;
	}

	load_delay_data(instruction.rt(), to_16(read_memory(address, 2)));
}

void Cpu::op_rfe(const Instruction& instruction) {
	uint32_t sr { cop0_get_register_data(Cop0_Register::sr) };

	// // Get previous and old status bits
	// uint32_t status_bits { (sr >> 2) & 0b1111 };
	//
	// // Shave off the old status bits and add the correct bits
	// sr >>= 4;
	// sr <<= 4;
	// sr |= status_bits;

	auto mode { sr & 0x3f };
	sr &= ~0x3f;
	sr |= mode >> 2;

	cop0_set_register(Cop0_Register::sr, sr);
}

void Cpu::op_mthi(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	m_hi = rs_data;
}

void Cpu::op_mtlo(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	m_lo = rs_data;
}

void Cpu::op_syscall(const Instruction& instruction) {
	exception(Exception::syscall);
}

void Cpu::op_slt(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int rt_data { static_cast<int>(get_register_data(instruction.rt())) };
	
	set_register(instruction.rd(), rs_data < rt_data);
}

void Cpu::op_mfhi(const Instruction& instruction) {
	set_register(instruction.rd(), m_hi);
}

void Cpu::op_divu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t divisor { get_register_data(instruction.rt()) };

	if (divisor == 0) {
		m_lo = std::numeric_limits<uint32_t>::max();
		m_hi = rs_data;
	} else {
		m_lo = rs_data / divisor;
		m_hi = rs_data % divisor;
	}
}

void Cpu::op_sltiu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	set_register(instruction.rt(), rs_data < instruction.imm16_se());
}

void Cpu::op_srl(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rt_data >> instruction.sa());
}

void Cpu::op_bgez(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data >= 0) {
		branch(offset);
	}
}

void Cpu::op_mflo(const Instruction& instruction) {
	set_register(instruction.rd(), m_lo);
}

void Cpu::op_div(const Instruction& instruction) {
	int numerator { static_cast<int>(get_register_data(instruction.rs())) };
	int denominator { static_cast<int>(get_register_data(instruction.rt())) };
	
	if (denominator == 0) {
		m_hi = static_cast<uint32_t>(numerator);
		if (numerator >= 0) {
			m_lo = std::numeric_limits<uint32_t>::max();
		} else {
			m_lo = 1;
		}
	} else if (denominator == std::numeric_limits<uint32_t>::max() && numerator == 0x80000000) {
		m_lo = 0x80000000;
		m_hi = 0;
	} else {
		m_lo = static_cast<uint32_t>(numerator / denominator);
		m_hi = static_cast<uint32_t>(numerator % denominator);
	}
}

void Cpu::op_sra(const Instruction& instruction) {
	int rt_data { static_cast<int>(get_register_data(instruction.rt())) };
	int result { rt_data >> instruction.sa() };
	set_register(instruction.rd(), result);
}

void Cpu::op_subu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rs_data - rt_data);
}

void Cpu::op_slti(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int imm { static_cast<int>(instruction.imm16_se()) };
	set_register(instruction.rt(), rs_data < imm);
}

void Cpu::op_bltz(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data < 0) {
		branch(offset);
	}
}

void Cpu::op_jalr(const Instruction& instruction) {
	set_register(instruction.rd(), m_next_pc);
	m_next_pc = get_register_data(instruction.rs());
}

void Cpu::op_lbu(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };

	load_delay_data(instruction.rt(), to_8(read_memory(address, 1)));
}

void Cpu::op_blez(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data <= 0) {
		branch(offset);
	}
}

void Cpu::op_bgtz(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data > 0) {
		branch(offset);
	}

}

void Cpu::op_add(const Instruction& instruction) {
	int rs_value { static_cast<int>(get_register_data(instruction.rs())) };
	int rt_value { static_cast<int>(get_register_data(instruction.rt())) };
	int64_t result { rs_value + rt_value };

	if (result > std::numeric_limits<int>::max()) {
		exception(Exception::arithmetic_overflow);
		return;
	}

	set_register(instruction.rd(), static_cast<uint32_t>(result));
}

void Cpu::op_and(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rs_data & rt_data);
}

void Cpu::op_mfc0(const Instruction& instruction) {
	uint32_t rd_data { cop0_get_register_data(instruction.cop0_rd()) };
	load_delay_data(instruction.rt(), rd_data);
}

void Cpu::op_beq(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data == rt_data) {
		branch(offset);
	}
}

void Cpu::op_lb(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };

	load_delay_data(instruction.rt(), 
				 static_cast<uint32_t>(static_cast<int8_t>(to_8(read_memory(address, 1)))));
}

void Cpu::op_jr(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.rs()) };
	m_next_pc = address;
}

void Cpu::op_sb(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	uint8_t result { static_cast<uint8_t>(get_register_data(instruction.rt())) };

	write_memory(address, std::as_bytes(std::span{ &result, 1 }));
}

void Cpu::op_andi(const Instruction& instruction) {
	set_register(instruction.rt(), instruction.imm16() & get_register_data(instruction.rs()));
}

void Cpu::op_jal(const Instruction& instruction) {
	set_register(Register::ra, m_next_pc);
	op_jump(instruction);
}

void Cpu::op_sh(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };

	if (address % 2 != 0) {
		exception(Exception::store_address_error);
		return;
	}

	uint16_t result { static_cast<uint16_t>(rt_data) };
	write_memory(address, std::as_bytes(std::span(&result, 1)));
}

void Cpu::op_addu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };

	set_register(instruction.rd(), rs_data + rt_data);
}

void Cpu::op_sltu(const Instruction& instruction) {
	set_register(instruction.rd(), 
			  get_register_data(instruction.rs()) < get_register_data(instruction.rt()));
}

void Cpu::op_lw(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	if (address % 4 != 0) {
		exception(Exception::load_address_error);
		return;
	}


	load_delay_data(instruction.rt(), to_32(read_memory(address, 4)));
}

void Cpu::op_addi(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int64_t result { rs_data + static_cast<int>(instruction.imm16_se()) };

	if (result > std::numeric_limits<uint32_t>::max()) {
		exception(Exception::arithmetic_overflow);
		return;
	}
	
	set_register(instruction.rt(), static_cast<uint32_t>(result));
}

void Cpu::op_bne(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data != rt_data) {
		branch(offset);
	}
}

void Cpu::op_mtc0(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	cop0_set_register(instruction.cop0_rd(), rt_data);
}

void Cpu::op_or(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	
	set_register(instruction.rd(), rs_data | rt_data);
}

void Cpu::op_jump(const Instruction& instruction) {
	uint32_t addr { instruction.jump_addr() << 2 };
	m_next_pc = (m_pc & 0xf0000000) | addr;
}

void Cpu::op_addiu(const Instruction& instruction) {
	uint32_t rs_value { get_register_data(instruction.rs()) };
	uint32_t result { rs_value + instruction.imm16_se() };

	set_register(instruction.rt(), result);
}

void Cpu::op_lui(const Instruction& instruction) {
	uint32_t constant { instruction.imm16() << 16 };

	set_register(instruction.rt(), constant);
}

void Cpu::op_sw(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	if (address % 4 != 0) {
		exception(Exception::store_address_error);
		return;
	}
	
	uint32_t result { get_register_data(instruction.rt()) };
	write_memory(address, std::as_bytes(std::span{ &result, 1 }));
}

void Cpu::op_ori(const Instruction& instruction) {
	set_register(instruction.rt(), instruction.imm16() | get_register_data(instruction.rs()));
}

void Cpu::op_sll(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rt_data << instruction.sa());
}


std::string_view Cpu::exception_name(Exception exception) const {
	using enum Exception;
	switch (exception) {
		case interrupt: return "interrupt";
		case load_address_error: return "load_address_error";
		case store_address_error: return "store_address_error";
		case instruction_bus_error: return "instruction_bus_error";
		case data_bus_error: return "data_bus_error";
		case syscall: return "syscall";
		case breakpoint: return "breakpoint";
		case reserved_instruction: return "reserved_instruction";
		case coprocessor_unusable: return "coprocessor_unusable";
		case arithmetic_overflow: return "arithmetic_overflow";
		default: return "unknown";
	}
}
