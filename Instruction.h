#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>

class Instruction {
public:
	enum class Opcode {
		ori,
		sw,
		lui,
		lw,
//		bne,
		unknown,
	};

	explicit Instruction(std::span<const std::byte> data) {
		memcpy(&m_data, data.data(), sizeof(int));
		m_opcode = get_opcode(m_data >> 26);
	}

	uint32_t rs() { return (m_data >> 21) & 0b11111; }
	uint32_t rt() { return (m_data >> 16) & 0b11111; }
	uint32_t rd() { return (m_data >> 11) & 0b11111; }
	uint32_t imm16() { return m_data & 0xffff; }

	// Returns a type based on the 5 bit identifier
	Opcode get_opcode(uint8_t identifier);
	Opcode opcode() { return m_opcode; }
	std::string_view type_string() const;
	
	friend std::ostream& operator<<(std::ostream& out, const Instruction& instruction) {
		out << instruction.type_string();	
		return out;
	}

private:
	Opcode m_opcode {};
	uint32_t m_data {};
};
