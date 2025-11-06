#include "Gpu.h"

#include "Logger.h"

// Receive a command from the bus.
void Gpu::receive_command(uint32_t command) {
    if ((command >> 24) == 0xe1) {
        Logger::log(Logger::Level::warning, "[GPU] Received Texpage command");
    }
}

// Returns a response based on the given address.
// 0x1f801814 -> GPUSTAT (GPU status register)
// 0x1f801810 -> Response to GP0 and GP1 commands.
std::span<const std::byte> Gpu::get_response(uint32_t physical_address) {
    if (physical_address == 0x1f801814) {
        Logger::log(Logger::Level::info, "[GPU] Sent GPUSTAT.");
        return std::as_bytes(std::span{ &m_gpustat, 1 });
    }

    Logger::log(Logger::Level::info, "[GPU] Sent GP1 response.");
    return std::as_bytes(std::span{ &m_dummy_var, 1 });
}
