#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>

class Instruction {
public:
	enum class Type {
		lui,
		unknown,
	};

	explicit Instruction(std::span<const std::byte> data) {
		memcpy(&m_data, data.data(), sizeof(int));
		m_type = get_type(m_data >> 26);
	}

	uint32_t rs() { return (m_data >> 21) & 0b11111; }
	uint32_t rt() { return (m_data >> 16) & 0b11111; }
	uint32_t rd() { return (m_data >> 11) & 0b11111; }
	uint32_t imm16() { return m_data & 0xffff; }

	Type get_type(uint8_t identifier);
	std::string_view type_string() const;
	
	friend std::ostream& operator<<(std::ostream& out, const Instruction& instruction) {
		out << instruction.type_string();	
		return out;
	}

private:
	Type m_type {};
	uint32_t m_data {};
};
