#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>

class Instruction {
public:
	enum class Opcode {
		ori,
		addiu,
		sll,
		lui,
		lw,
		sw,
		jump,
		unknown,
	};

	explicit Instruction(std::span<const std::byte> data) {
		memcpy(&m_data, data.data(), sizeof(int));
		m_opcode = determine_opcode(m_data);
	}

	explicit Instruction(uint32_t data) : m_data { data }, m_opcode { determine_opcode(data) } {}

	uint32_t rs() const { return (m_data >> 21) & 0b11111; }
	uint32_t rt() const { return (m_data >> 16) & 0b11111; }
	uint32_t rd() const { return (m_data >> 11) & 0b11111; }

	uint32_t imm16() const { return m_data & 0xffff; }
	// Note the order of casts. This ensures that a sign extending instruction is 
	// is generated.
	uint32_t imm16_se() const { 
		return static_cast<uint32_t>(static_cast<short>(m_data & 0xffff));
	}

	uint32_t sa() const { return (m_data >> 6) & 0b11111; }


	uint32_t jump_addr() const { return m_data & 0x3ffffff; }

	// Returns a type based on the 5 bit identifier
	Opcode opcode() const { return m_opcode; }
	std::string_view type_string() const;

	uint32_t data() const { return m_data; }
	
	friend std::ostream& operator<<(std::ostream& out, const Instruction& instruction) {
		out << instruction.type_string();	
		return out;
	}

private:
	uint32_t m_data {};
	Opcode m_opcode {};

	Instruction::Opcode determine_opcode(uint32_t data);
};
