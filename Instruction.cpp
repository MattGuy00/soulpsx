#include "Instruction.h"
#include <cstdint>


std::string_view Instruction::type_string() const {
	using enum Type;
	switch (m_type) {
		case lui: return "lui";
		default: return "unknown";
	}
}

Instruction::Type Instruction::get_type(uint8_t identifier) {
	using enum Type;
	switch (identifier) {
		case 0b001111: return lui;
		default: return unknown;
	}
}
