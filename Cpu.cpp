#include "Cpu.h"
#include "Instruction.h"

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>

void Cpu::fetch_decode_execute() {
	for (;;) {
		Instruction instruction { Instruction(read_memory(m_pc, 4)) };	

		// Print address and instruction
		std::cout << std::hex << m_bus.to_physical_address(m_pc) << ":";
		std::cout << (instruction.data() >> 24);
		std::cout << ((instruction.data() >> 16) & 0xff);
		std::cout << ((instruction.data() >> 8) & 0xff);
		std::cout << (instruction.data() & 0xff);
		std::cout << ": ";

		m_pc = m_next_pc;
		m_next_pc += 4;

		// load the delayed data into the temp registers.
		// The next next instruction won't see the new values yet.
		// Also allows for the instruction to overwrite the delay slot's data
		// which is a thing
		if (m_load_delay_slot) {
			set_register(m_load_delay_slot->reg, m_load_delay_slot->data);
			m_load_delay_slot = std::nullopt;
		}
			
		using enum Instruction::Opcode;
		switch (instruction.opcode()) {
			case andi: {
				op_andi(instruction);
				break;
			}
			case and_b: {
				op_and(instruction);
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
			case add: {
				op_add(instruction);
				break;
			}
			case subu: {
				op_subu(instruction);
				break;
			}
			case div: {
				op_div(instruction);
				break;
			}
			case divu: {
				op_divu(instruction);
				break;
			}
			case slt: {
				op_slt(instruction);
				break;
			}
			case sltu: {
				op_sltu(instruction);
				break;
			}
			case slti: {
				op_slti(instruction);
				break;
			}
			case sltiu: {
				op_sltiu(instruction);
				break;
			}
			case sll: {
				op_sll(instruction);
				break;
			}
			case srl: {
				op_srl(instruction);
				break;
			}
			case sra: {
				op_sra(instruction);
				break;
			}
			case lw: {
				op_lw(instruction);
				break;
			}
			case lb: {
				op_lb(instruction);
				break;
			}
			case lbu: {
				op_lbu(instruction);
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
			case jalr: {
				op_jalr(instruction);
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
			case beq: {
				op_beq(instruction);
				break;
			}
			case bgtz: {
				op_bgtz(instruction);
				break;
			}
			case bgez: {
				op_bgez(instruction);
				break;
			}
			case blez: {
				op_blez(instruction);
				break;
			}
			case bltz: {
				op_bltz(instruction);
				break;
			}
			case mflo: {
				op_mflo(instruction);
				break;
			}
			case mfhi: {
				op_mfhi(instruction);
				break;
			}
			case mtc0: {
				op_mtc0(instruction);
				break;
			}
			case mfc0: {
				op_mfc0(instruction);
				break;
			}
			case syscall: {
				op_syscall(instruction);
				break;
			}
			case unknown: {
				std::cout << instruction << '\n';
				return;
			}
		}
		
		// Update the registers
		m_registers = m_temp_registers;
		m_cop0_registers = m_cop0_temp_registers;
	}
}

void Cpu::set_register(Register reg, uint32_t data) {
	m_temp_registers[static_cast<uint32_t>(reg)] = data;
	m_temp_registers[0] = 0;
}

uint32_t Cpu::get_register_data(Register reg) {
	return m_registers[static_cast<uint32_t>(reg)];
}

void Cpu::cop0_set_register(Cop0_Register reg, uint32_t data) {
	m_cop0_temp_registers[static_cast<uint32_t>(reg)] = data;
	m_cop0_temp_registers[0] = 0;
}

uint32_t Cpu::cop0_get_register_data(Cop0_Register reg) {
	return m_cop0_registers[static_cast<uint32_t>(reg)];
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

uint8_t to_8(std::span<const std::byte> data) {
	uint8_t byte {};
	std::memcpy(&byte, data.data(), sizeof(byte));

	return byte;
}

void Cpu::branch(uint32_t offset) {
	m_next_pc += offset;
	m_next_pc -= 4;
}

void Cpu::load_delay_data(Register reg, uint32_t data) {
	m_load_delay_slot = { reg, data };
}

// I dont really understand much of this function yet
// Credit to Lionel Flandrin
void Cpu::exception(Exception excode) {
	uint32_t sr { cop0_get_register_data(Cop0_Register::sr) };
	// Find exception handler address based on BEV bit
	uint32_t handler { 0xbfc00180 ? (sr & (1 << 22)) : 0x80000080 };

	uint32_t mode { sr & 0x3f };
	sr &= ~0x3f;
	sr |= (mode << 2) & 0x3f;
	cop0_set_register(Cop0_Register::sr, sr);

	// Store the cause of the exception in the cause register at bits 2:6
	cop0_set_register(Cop0_Register::cause, static_cast<uint32_t>(excode) << 2);

	cop0_set_register(Cop0_Register::epc, m_pc);

	// Immediately jump to the handler
	m_pc = handler;
	m_next_pc = m_pc + 4;
}

void Cpu::op_syscall(const Instruction& instruction) {
	exception(Exception::syscall);

	std::cout << instruction << '\n';
}

void Cpu::op_slt(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int rt_data { static_cast<int>(get_register_data(instruction.rt())) };
	
	set_register(instruction.rd(), rs_data < rt_data);

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << rs_data  << ", " << rt_data << '\n';
}

void Cpu::op_mfhi(const Instruction& instruction) {
	set_register(instruction.rd(), m_hi);

	std::cout << instruction << ' ' << register_name(instruction.rd()) << '\n';
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

	std::cout << instruction << ' ' << rs_data << ", " << divisor << '\n';
}

void Cpu::op_sltiu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	set_register(instruction.rt(), rs_data < instruction.imm16_se());

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << rs_data << " < " << instruction.imm16_se() << '\n';
}

void Cpu::op_srl(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rt_data >> instruction.sa());
	
	std::cout << instruction << " " << register_name(instruction.rd()) << ", ";
	std::cout << rt_data << ", " << instruction.sa() << '\n';
}

void Cpu::op_bgez(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data >= 0) {
		branch(offset);
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << offset << '\n';
}

void Cpu::op_mflo(const Instruction& instruction) {
	set_register(instruction.rd(), m_lo);

	std::cout << instruction << ' ' << register_name(instruction.rd()) << '\n';
}

void Cpu::op_div(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int divisor { static_cast<int>(get_register_data(instruction.rt())) };
	
	if (divisor == 0) {
		m_hi = static_cast<uint32_t>(rs_data);
		if (rs_data >= 0) {
			m_lo = std::numeric_limits<uint32_t>::max();
		} else {
			m_lo = 1;
		}
	} else if (divisor == -1 && (uint32_t)rs_data == 0x80000000) {
		m_lo = 0x80000000;
		m_hi = 0;
	} else {
		m_lo = static_cast<uint32_t>(rs_data / divisor);
		m_hi = static_cast<uint32_t>(rs_data % divisor);
	}

	std::cout << instruction << ' ' << rs_data << ", " << divisor << '\n';
}

void Cpu::op_sra(const Instruction& instruction) {
	int rt_data { static_cast<int>(get_register_data(instruction.rt())) };
	int result { rt_data >> instruction.sa() };
	set_register(instruction.rd(), static_cast<uint32_t>(result));

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << rt_data << " >> " << std::dec << instruction.sa() << " (";
	std::cout << std::hex << result << ")\n";
}

void Cpu::op_subu(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rs_data - rt_data);

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << rs_data << " - " << rt_data << '\n';
}

void Cpu::op_slti(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int imm { static_cast<int>(instruction.imm16_se()) };
	set_register(instruction.rt(), rs_data < imm);

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << rs_data << " < " << imm << '\n';
}

void Cpu::op_bltz(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data < 0) {
		branch(offset);
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << offset << '\n';
}

void Cpu::op_jalr(const Instruction& instruction) {
	set_register(instruction.rd(), m_next_pc);
	m_next_pc = get_register_data(instruction.rs());

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << get_register_data(instruction.rs()) << '\n';
}

void Cpu::op_lbu(const Instruction& instruction) {
	// Cache is isolated
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	
	load_delay_data(instruction.rt(), 
				 static_cast<uint32_t>(to_8(read_memory(address, 1))));
	
	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << std::hex << address << '\n';
}

void Cpu::op_blez(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data <= 0) {
		branch(offset);
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << offset << '\n';
}

void Cpu::op_bgtz(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data > 0) {
		branch(offset);
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << offset << '\n';
}

void Cpu::op_add(const Instruction& instruction) {
	int rs_value { static_cast<int>(get_register_data(instruction.rs())) };
	int rt_value { static_cast<int>(get_register_data(instruction.rt())) };
	long long result { rs_value + rt_value };

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << rs_value << " + " << rt_value << " (" << result << ")\n";

	if (result > std::numeric_limits<int>::max()) {
		std::cout << "Overflow exception\n";
		return;
	}

	set_register(instruction.rd(), static_cast<uint32_t>(result));
}

void Cpu::op_and(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	set_register(instruction.rd(), rs_data & rt_data);

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << rs_data << " & " << rt_data << " (" << (rs_data & rt_data) << ")\n";
}

void Cpu::op_mfc0(const Instruction& instruction) {
	uint32_t rd_data { cop0_get_register_data(instruction.cop0_rd()) };
	load_delay_data(instruction.rt(), rd_data);

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << cop0_register_name(instruction.cop0_rd()) << '\n';
}

void Cpu::op_beq(const Instruction& instruction) {
	uint32_t rs_data { get_register_data(instruction.rs()) };
	uint32_t rt_data { get_register_data(instruction.rt()) };
	uint32_t offset { instruction.imm16_se() << 2 };

	if (rs_data == rt_data) {
		branch(offset);
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << rt_data << ", " << offset << '\n';
}

void Cpu::op_lb(const Instruction& instruction) {
	// Cache is isolated
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };

	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << std::hex << address << '\n';

	load_delay_data(instruction.rt(), 
				 static_cast<uint32_t>(static_cast<int8_t>(to_8(read_memory(address, 1)))));
}

void Cpu::op_jr(const Instruction& instruction) {
	uint32_t address { get_register_data(instruction.rs()) };
	m_next_pc = address;

	std::cout << instruction << " " << address << '\n';
}

void Cpu::op_sb(const Instruction& instruction) {
	// Cache is isolated
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };
	uint8_t result { static_cast<uint8_t>(get_register_data(instruction.rt())) };

	std::cout << instruction << ' ' << std::hex << m_bus.to_physical_address(address);
	std::cout << ", " << result << '\n';

	write_memory(address, std::as_bytes(std::span{ &result, 1 }));
}

void Cpu::op_andi(const Instruction& instruction) {
	set_register(instruction.rt(), instruction.imm16() & get_register_data(instruction.rs()));

	std::cout << instruction << " " << register_name(instruction.rt()) << ", ";
	std::cout << register_name(instruction.rs()) << ", " << instruction.imm16() << '\n';
}

void Cpu::op_jal(const Instruction& instruction) {
	set_register(Register::ra, m_next_pc);
	op_jump(instruction);
}

void Cpu::op_sh(const Instruction& instruction) {
	// Cache is isolated
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
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
	std::cout << rs_data << " + " << rt_data << '\n';
}

void Cpu::op_sltu(const Instruction& instruction) {
	set_register(instruction.rd(), 
			  get_register_data(instruction.rs()) < get_register_data(instruction.rt()));

	std::cout << instruction << ' ' << register_name(instruction.rd()) << ", ";
	std::cout << register_name(instruction.rs()) << ", " << register_name(instruction.rt()) << '\n';
}

void Cpu::op_lw(const Instruction& instruction) {
	// Cache is isolated
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
		std::cout << "cache isolated\n";
		return;
	}

	uint32_t address { get_register_data(instruction.base()) + instruction.imm16_se() };

	load_delay_data(instruction.rt(), to_32(read_memory(address, 4)));
	
	std::cout << instruction << ' ' << register_name(instruction.rt()) << ", ";
	std::cout << std::hex << address << '\n';
}

void Cpu::op_addi(const Instruction& instruction) {
	int rs_data { static_cast<int>(get_register_data(instruction.rs())) };
	int64_t result { rs_data + static_cast<int>(instruction.imm16_se()) };

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
		branch(offset);
	}

	std::cout << instruction << ' ';
	std::cout << rs_data << ", " << rt_data << ", " << offset << '\n';
}

void Cpu::op_mtc0(const Instruction& instruction) {
	uint32_t rt_data { get_register_data(instruction.rt()) };
	cop0_set_register(instruction.cop0_rd(), rt_data);

	std::cout << instruction << ' ' << rt_data << ", ";
	std::cout << cop0_register_name(instruction.cop0_rd()) << '\n';
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
	m_next_pc = (m_pc & 0xf0000000) | addr;

	std::cout << instruction << " " << m_next_pc << '\n';
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
	if ((cop0_get_register_data(Cop0_Register::sr) & 0x10000) != 0) {
		std::cout << "cache isolated\n";
		return;
	}
	

	std::cout << instruction << " " << register_name(instruction.rt()) << ", ";
	std::cout << m_bus.to_physical_address(address) << '\n';

	uint32_t result { get_register_data(instruction.rt()) };
	write_memory(address, std::as_bytes(std::span{ &result, 1 }));
}

void Cpu::op_ori(const Instruction& instruction) {
	set_register(instruction.rt(), instruction.imm16() | get_register_data(instruction.rs()));

	std::cout << instruction << " " << register_name(instruction.rt()) << ", ";
	std::cout << register_name(instruction.rs()) << ", " << instruction.imm16() << '\n';
}

void Cpu::op_sll(const Instruction& instruction) {
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

std::string_view Cpu::cop0_register_name(Cop0_Register reg) {
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
		case unused: return "unused";
	}
}
