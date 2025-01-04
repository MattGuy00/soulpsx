#pragma once

#include <cstdint>

class Instruction {
public:
	enum class Type {
		lui,
		unknown,
	};

	explicit Instruction(uint32_t data)
		: m_type { get_type(data >> 26) },
		m_data { data }
	{
	}

	uint32_t rs() { return (m_data >> 21) & 0b11111; }
	uint32_t rt() { return (m_data >> 16) & 0b11111; }
	uint32_t rd() { return (m_data >> 11) & 0b11111; }
	uint32_t imm16() { return m_data & 0xffff; }
	

	Type m_type {};
	uint32_t m_data {};
private:
	Type get_type(uint8_t identifier);
};
