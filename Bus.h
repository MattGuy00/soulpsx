#pragma once

#include "Bios.h"
#include "Memory.h"

struct Bus {
	Bios& bios;
	Memory& memory;
};
