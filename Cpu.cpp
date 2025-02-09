#include "Cpu.h"
#include "Instruction.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <ranges>
#include <span>

void Cpu::fetch_decode_execute() {
	for (;;) {
		Instruction instruction { m_next_instruction };	
		m_next_instruction = Instruction(read_memory(m_pc, 4));

		// Print address and instruction
		std::cout << std::hex << m_bus.to_physical_address(m_pc) << ":";
		std::cout << (instruction.data() >> 24);
		std::cout << ((instruction.data() >> 16) & 0xff);
		std::cout << ((instruction.data() >> 8) & 0xff);
		std::cout << (instruction.data() & 0xff);
		std::cout << ": ";

		m_pc += 4;
			
		using enum Instruction::Opcode;
		switch (instruction.opcode()) {
			case andi: {
				op_andi(instruction);
				break;
			}
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
			case sh: {
				op_sh(instruction);
				break;
			}
			case sb: {
				op_sb(instruction);
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
			case jal: {
				op_jal(instruction);
				break;
			}
			case jr: {
				op_jr(instruction);
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

		if (m_load_completed_data && m_load_completed_register) {
			if (m_overwritten_reg != m_load_completed_register) {
				set_register(m_load_completed_register.value(), 
				 m_load_completed_data.value());
			}

			m_load_completed_register.reset();
			m_load_completed_data.reset();
		}

		if (m_load_delay_register && m_load_delay_data) {
			m_load_completed_data = m_load_delay_data;
			m_load_completed_register = m_load_delay_register;

			m_load_delay_register.reset();
			m_load_delay_data.reset();
		}
	}
}

void Cpu::set_register(Register reg, uint32_t data) {
	m_registers[static_cast<uint32_t>(reg)] = data;
	m_registers[0] = 0;
}

uint32_t Cpu::get_register_data(Register reg) {
	return m_registers[static_cast<uint32_t>(reg)];
}

std::span<const std::byte> Cpu::read_memory(uint32_t address, uint32_t bytes) {
	return m_bus.read_memory(address, bytes);
}

void Cpu::write_memory(uint32_t address, std::span<const std::byte> data) {
	m_bus.write_memory(address, data);
}

uint32_t to_32(std::span<const std::byte> data) {
	uint32_t word {};
	std::memcpy(&word, data.data(), sizeof(word));

	return word;
}

void Cpu::op_jr(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.rs()) };
	m_pc = address;

	std::cout << instruction << " " << m_pc << '\n';
}

void Cpu::op_sb(const Instruction& instruction) {
	// Cache is isolated
	if (cop0_sr & 0x10000) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };

	uint8_t result { static_cast<uint8_t>(get_register_data(instruction.rt())) };

	write_memory(address, std::as_bytes(std::span{ &result, 1 }));

	std::cout << instruction << ' ' << std::hex << m_bus.to_physical_address(address);
	std::cout << ", " << std::dec << result << '\n';
}

void Cpu::op_andi(const Instruction& instruction) {
	set_register(instruction.rt(), instruction.imm16() & get_register_data(instruction.rs()));

	std::cout << instruction << " " << register_name(instruction.rt()) << ", ";
	std::cout << register_name(instruction.rs()) << ", " << std::dec << instruction.imm16() << '\n';
}

void Cpu::op_jal(const Instruction& instruction) {
	set_register(Register::ra, m_pc);
	op_jump(instruction);
}

void Cpu::op_sh(const Instruction& instruction) {
	// Cache is isolated
	if (cop0_sr & 0x10000) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	 

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << std::hex << address << '\n';

	uint16_t result { static_cast<uint16_t>(rt_data) };
	write_memory(address, std::as_bytes(std::span(&result, 1)));
}

void Cpu::op_addu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };

	set_register(instruction.rd(), rs_data + rt_data);

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << std::dec << rs_data << " + " << rt_data << '\n';
}

void Cpu::op_sltu(const Instruction& instruction) {
	
	set_register(instruction.rd(), 
			  get_register_data(instruction.rs()) < get_register_data(instruction.rt()));

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << register_name(instruction.rs()) << ", " << register_name(instruction.rt()) << '\n';
}

void Cpu::op_lw(const Instruction& instruction) {
	// Cache is isolated
	if (cop0_sr & 0x10000) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	// delay the load by one instruction
	m_load_delay_register = instruction.rt();
	m_load_delay_data = to_32(read_memory(address, 4));
	
	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << std::hex << address << '\n';
}

void Cpu::op_addi(const Instruction& instruction) {
	long long rs_data { get_register_data(instruction.rs()) };
	long long result { rs_data + static_cast<int>(instruction.imm16_se()) };

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << std::dec << rs_data << " + " << instruction.imm16() << '\n';

	// TODO: Throw an Integer Overflow exception
	if (result > std::numeric_limits<uint32_t>::max()) {
		std::cout << " overflow exception\n";
		return;
	}
	
	set_register(instruction.rt(), result);
}

void Cpu::op_bne(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data != rt_data) {
		m_pc += offset;
		m_pc -= 4;
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << std::dec << rt_data << ", " << offset << '\n';
}

void Cpu::op_mtc0(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t cop0_reg { static_cast<uint32_t>(instruction.rd()) };

	std::cout << instruction << ' ';
	// status register
	if (cop0_reg == 12) {
		cop0_sr = rt_data;
		std::cout << std::dec << rt_data << ", " << cop0_reg << '\n';
	} else {
		std::cout << "unimplemented\n";
	}
}

void Cpu::op_or(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	
	set_register(instruction.rd(), rs_data | rt_data);

	std::cout << instruction << " " << register_name(instruction.rd()) << ", ";
	std::cout << (rs_data | rt_data) << '\n';
}

void Cpu::op_jump(const Instruction& instruction) {
	uint32_t addr { instruction.jump_addr() << 2 };
	m_pc = (m_pc & 0xf0000000) | addr;

	std::cout << instruction << " " << m_pc << '\n';
}

void Cpu::op_addiu(const Instruction& instruction) {
	uint32_t rs_value { get_register_data(instruction.rs()) };
	uint32_t result { rs_value + instruction.imm16_se() };

	set_register(instruction.rt(), result);

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << rs_value << " + " << instruction.imm16_se() << '\n' ;
}

void Cpu::op_lui(const Instruction& instruction) {
	uint32_t constant { instruction.imm16() << 16 };

	set_register(instruction.rt(), constant);
	std::cout << instruction << " " << register_name(instruction.rt()) << ", " << constant << '\n';
}

void Cpu::op_sw(const Instruction& instruction) {
	uint32_t base { get_register_data(instruction.base()) };
	uint32_t address { base + instruction.imm16_se() };

	// Cache is isolated
	if (cop0_sr & 0x10000) {
		std::cout << "cache isolated\n";
		return;
	}
	
	uint32_t result { get_register_data(instruction.rt()) };

	std::cout << instruction << " " << register_name(instruction.rt()) << ", ";
	std::cout << m_bus.to_physical_address(address) << '\n';

	write_memory(address, std::as_bytes(std::span{ &result, 1 }));
}

void Cpu::op_ori(const Instruction& instruction) {
	set_register(instruction.rt(), instruction.imm16() | get_register_data(instruction.rs()));

	std::cout << instruction << " " << register_name(instruction.rt()) << ", ";
	std::cout << register_name(instruction.rs()) << ", " << instruction.imm16() << '\n';
}

void Cpu::op_sll(const Instruction& instruction) {
	// If the instruction is 0, then its a NOP
	if (instruction.sa() == 0) {
		std::cout << "nop\n";
		return;
	}

	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rt_data << instruction.sa());
	
	std::cout << instruction << " " << register_name(instruction.rd()) << ", ";
	std::cout << rt_data << ", " << instruction.sa() << '\n';
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
