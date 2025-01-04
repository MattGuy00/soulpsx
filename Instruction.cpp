#include "Instruction.h"
#include <cstdint>

Instruction::Type Instruction::get_type(uint8_t identifier) {
	switch (identifier) {
		case 0b001111: return Type::lui;
	}

	return Type::unknown;
}
