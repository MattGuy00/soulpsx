#include "Cpu.h"
#include "Instruction.h"
#include <cstdint>
#include <optional>

void Cpu::fetch_decode_execute() {
	for (;;) {
		// If the last instruction was a load, we want to
		// overwrite the register after this instruction
		if (m_load_delay_data && m_load_delay_register) {
			m_load_completed_register = m_load_delay_register;
			m_load_completed_data = m_load_delay_data;

			m_load_delay_register.reset();
			m_load_delay_data.reset();
		}
		
		Instruction instr { m_bus.memory.read_data(4, m_pc) };	
		
		for (auto byte : m_bus.memory.read_data(4, m_pc)) {
			std::cout << std::hex << std::to_integer<int>(byte);
		}
		std::cout << ": ";

		m_pc += 4;
			
		using enum Instruction::Opcode;
		switch (instr.opcode()) {
			case ori: {
				uint32_t upper { instr.imm16() << 16 };
				m_registers[instr.rt()] = upper | m_registers[instr.rs()];

				std::cout << instr << " " << instr.rt() << ", ";
				std::cout << instr.rs() << ", " << instr.imm16() << '\n';
				break;
			}
			case sw: {
				// Convert into an array of bytes 
				// so it can be written into memory
				uint32_t word { m_registers[instr.rt()] }	;
				std::array<std::byte, 4> bytes {
					static_cast<std::byte>((word >> 24) & 0xff),
					static_cast<std::byte>((word >> 16) & 0xff),
					static_cast<std::byte>((word >> 8) & 0xff),
					static_cast<std::byte>(word & 0xff),
				};

				uint32_t base { m_registers[instr.rs()] };
				// Offset is signed value
				uint32_t e_addr { base + static_cast<int>(instr.imm16()) };
				m_bus.memory.write_data(bytes, e_addr);

				std::cout << instr << " " << instr.rt() << ", ";
				std::cout << e_addr << '\n';
				break;
			}
			case lui: {
				uint32_t upper { instr.imm16() << 16 };
				m_load_delay_register = std::make_optional((Register)instr.rt()) ;
				m_load_delay_data = std::make_optional(upper);

				std::cout << instr << " " << instr.rt() << ", " << upper << '\n';
				break;
			}
			case unknown: {
				std::cout << instr << '\n';
				return;
			}
		}

		if (m_load_completed_register && m_load_completed_data) {
			m_registers[static_cast<uint32_t>(m_load_completed_register.value())] = m_load_completed_data.value();

			m_load_completed_register.reset();
			m_load_completed_data.reset();
		}

		for (auto reg : m_registers) {
			std::cout << reg << ' ';
		}
		std::cout << '\n';
	}
}
