#pragma once

#include "Bus.h"

class Cpu {
public:
	Cpu(Bus& bus): m_bus { bus }
	{
	}

private:
	Bus& m_bus;
};
