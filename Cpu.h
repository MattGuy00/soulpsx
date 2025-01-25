#pragma once

#include "Bus.h"
#include <array>
#include <cstdint>
#include <optional>

class Cpu {
public:
	Cpu(Bus& bus): m_bus { bus }
	{
	}

	void fetch_decode_execute();

private:
	Bus& m_bus;
	int m_pc {};
	std::array<uint32_t, 32> m_registers {};

	// Register names are from 
	// https://psx-spx.consoledev.net/cpuspecifications/
	enum class Register {
		// constant (0)
		zero = 0,

		// Assembler Temporary
		at,
		
		// Subroutine return values
		v0, v1,
		
		// Subroutine args
		a0, a1, a2,	a3,
		
		// Temporaries
		t0, t1, t2, t3, t4, t5, t6, t7,
		
		// Static variables
		s0, s1, s2, s3, s4, s5, s6, s7,
		
		// Temporaries
		t8, t9,
		
		// For kernel
		k0, k1,
		
		// Global pointer
		gp,

		// Stack pointer
		sp,

		// Frame Pointer or static variable
		fp,

		// Return address
		ra,
	};
	
	std::optional<Register> m_load_delay_register {};
	std::optional<uint32_t> m_load_delay_data {};

	std::optional<Register> m_load_completed_register {};
	std::optional<uint32_t> m_load_completed_data {};
};
