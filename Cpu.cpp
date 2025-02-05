#include "Cpu.h"
#include "Instruction.h"

#include <cstdint>
#include <limits>

void Cpu::fetch_decode_execute() {
	for (;;) {
		Instruction instruction { m_next_instruction };	
		m_next_instruction = Instruction(read_memory(m_pc, 4));

		// Print Hex virtual address and instruction
		std::cout << std::hex << m_bus.to_physical_address(m_pc) << ":";
		std::cout << (instruction.data() >> 24);
		std::cout << ((instruction.data() >> 16) & 0xff);
		std::cout << ((instruction.data() >> 8) & 0xff);
		std::cout << (instruction.data() & 0xff);
		std::cout << ": ";

		m_pc += 4;

		if (m_load_delay_register && m_load_delay_data) {
			m_load_completed_data = m_load_delay_data;
			m_load_completed_register = m_load_delay_register;

			m_load_delay_register.reset();
			m_load_delay_data.reset();
		} else if (m_load_completed_data && m_load_completed_register) {
			set_register(m_load_completed_register.value(), 
				m_load_completed_data.value());

			m_load_completed_register.reset();
			m_load_completed_data.reset();
		}
			
		using enum Instruction::Opcode;
		switch (instruction.opcode()) {
			case or_b: {
				op_or(instruction);
				break;
			}
			case ori: {
				op_ori(instruction);
				break;
			}
			case addiu: {
				op_addiu(instruction);
				break;
			}
			case addi: {
				op_addi(instruction);
				break;
			}
			case addu: {
				op_addu(instruction);
				break;
			}
			case sltu: {
				op_sltu(instruction);
				break;
			}
			case sll: {
				op_sll(instruction);
				break;
			}
			case lw: {
				op_lw(instruction);
				break;
			}
			case sw: {
				op_sw(instruction);
				break;
			}
			case lui: {
				op_lui(instruction);
				break;
			}
			case jump: {
				op_jump(instruction);
				break;
			}
			case bne: {
				op_bne(instruction);
				break;
			}
			case mtc0: {
				op_mtc0(instruction);
				break;
			}
			case unknown: {
				std::cout << instruction << '\n';
				return;
			}
		}
	}
}

void Cpu::set_register(Register reg, uint32_t data) {
	m_registers[static_cast<uint32_t>(reg)] = data;
}

uint32_t Cpu::get_register_data(Register reg) {
	return m_registers[static_cast<uint32_t>(reg)];
}

std::span<const std::byte> Cpu::read_memory(uint32_t offset, uint32_t bytes) {
	return m_bus.read_memory(offset, bytes);
}

void Cpu::write_memory(std::span<const std::byte> data, uint32_t address) {
	// Skip writes outside of main memory for now
	if (address > 0x1f000000) return;
	m_bus.write_memory(address, data);
}

uint32_t to_word(std::span<const std::byte> data) {
	uint32_t word {};
	word |= static_cast<uint8_t>(data[0]) << 24;
	word |= static_cast<uint8_t>(data[1]) << 16;
	word |= static_cast<uint8_t>(data[2]) << 8;
	word |= static_cast<uint8_t>(data[3]);
	return word;
}

void Cpu::op_addu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(to_register(instruction.rs())) };
	uint32_t rt_data { get_register_data(to_register(instruction.rt())) };

	Register rd { to_register(instruction.rd()) };
	set_register(rd, rs_data + rt_data);

	std::cout << instruction << ' ' << register_name(rd) << ", ";
	std::cout << std::dec << rs_data << " + " << rt_data << '\n';
}

void Cpu::op_sltu(const Instruction& instruction) {
	Register rs { to_register(instruction.rs()) };
	Register rt { to_register(instruction.rt()) };
	Register rd { to_register(instruction.rd()) };
	
	set_register(rd, get_register_data(rs) < get_register_data(rt));

	std::cout << instruction << ' ' << register_name(rd) << ", ";
	std::cout << register_name(rs) << ", " << register_name(rt) << '\n';
}

void Cpu::op_lw(const Instruction& instruction) {
	short offset { static_cast<short>(instruction.imm16_se()) };
	uint32_t base { instruction.rs() };
	
	Register rt { to_register(instruction.rt()) };

	std::cout << instruction << ' ' << register_name(rt) << ", ";
	std::cout << base <<  " + " << std::dec << offset << '\n';

	// delay the load by one instruction
	m_load_delay_register = rt;
	uint32_t word { to_word(read_memory(base + offset, 4)) };
	m_load_delay_data = word;
}

void Cpu::op_addi(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(to_register(instruction.rs()))) };
	long long result { rs_data + static_cast<int>(instruction.imm16_se()) };

	// TODO: Don't modify register rt and throw an Integer Overflow exception
	if (result > std::numeric_limits<int>::max()) {
		std::cout << instruction << " overflow exception\n";
		return;
	}
	
	Register rt { to_register(instruction.rt()) };
	set_register(rt, result);

	std::cout << instruction << ' ' << register_name(rt) << ", ";
	std::cout << std::dec << rs_data << " + " << instruction.imm16() << '\n';
}

void Cpu::op_bne(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(to_register(instruction.rs())) };
	uint32_t rt_data { get_register_data(to_register(instruction.rt())) };
	int offset { static_cast<int>(instruction.imm16_se() << 2) };

	if (rs_data != rt_data) {
		m_pc += offset;
		m_pc -= 4;

	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << std::dec << rt_data << ", " << offset << '\n';
}

void Cpu::op_mtc0(const Instruction& instruction) {
	// TODO: Implement cop0
	std::cout << instruction << "(unimplemented)\n";
}

void Cpu::op_or(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(to_register(instruction.rs())) };
	uint32_t rt_data { get_register_data(to_register(instruction.rt())) };
	
	Register rd { to_register(instruction.rd()) };
	set_register(rd, rs_data | rt_data);

	std::cout << instruction << " " << register_name(rd) << ", ";
	std::cout << (rs_data | rt_data) << '\n';
}

void Cpu::op_jump(const Instruction& instruction) {
	uint32_t addr { instruction.jump_addr() << 2 };
	m_pc = (m_pc & 0xf0000000) | addr;

	std::cout << instruction << " " << m_pc << '\n';
}

void Cpu::op_addiu(const Instruction& instruction) {
	Register rs { to_register(instruction.rs()) };
	uint32_t rs_value { get_register_data(rs) };
	
	uint32_t result { rs_value + static_cast<int>(instruction.imm16_se()) };
	Register rt { to_register(instruction.rt()) };
	set_register(rt, result);

	std::cout << instruction << ' ' << register_name(rt) << ", ";
	std::cout  << std::dec << rs_value << " + " << rs_value + static_cast<int>(instruction.imm16_se()) << '\n' ;
}

void Cpu::op_lui(const Instruction& instruction) {
	uint32_t upper { instruction.imm16() << 16 };
	/*m_load_delay_register = std::make_optional((Register)instr.rt()) ;*/
	/*m_load_delay_data = std::make_optional(upper);*/

	Register reg { to_register(instruction.rt()) };
	set_register(reg, upper);
	std::cout << instruction << " " << register_name(reg) << ", " << upper << '\n';
}

void Cpu::op_sw(const Instruction& instruction) {
	// Convert into an array of bytes 
	// so it can be written into memory
	Register t_reg {to_register(instruction.rt()) };
	uint32_t word { get_register_data(t_reg) }	;
	std::array<std::byte, 4> bytes {
		static_cast<std::byte>((word >> 24) & 0xff),
		static_cast<std::byte>((word >> 16) & 0xff),
		static_cast<std::byte>((word >> 8) & 0xff),
		static_cast<std::byte>(word & 0xff),
	};

	Register s_reg { to_register(instruction.rs()) };
	uint32_t base { get_register_data(s_reg) };

	uint32_t e_addr { base + instruction.imm16_se() };
	write_memory(bytes, e_addr);

	std::cout << instruction << " " << register_name(t_reg) << ", ";
	std::cout << e_addr << '\n';
}

void Cpu::op_ori(const Instruction& instruction) {
	Register t_reg { to_register(instruction.rt()) };
	Register s_reg { to_register(instruction.rs()) };

	set_register(t_reg, instruction.imm16() | get_register_data(s_reg));

	std::cout << instruction << " " << register_name(t_reg) << ", ";
	std::cout << register_name(t_reg) << ", " << instruction.imm16() << '\n';
}

void Cpu::op_sll(const Instruction& instruction) {
	// If the instruction is 0, then its a NOP
	if (instruction.data() == 0) {
		std::cout << "sll NOP\n";
		return;
	}

	uint32_t rt_contents { get_register_data(to_register(instruction.rt())) };
	Register rd { to_register(instruction.rd()) };
	set_register(rd, rt_contents << instruction.sa());
	
	std::cout << instruction << " ";
	std::cout << register_name(rd) << ", " << rt_contents << ", " << instruction.sa() << '\n';
}

std::string_view Cpu::register_name(Register reg) { 
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
	}
}
