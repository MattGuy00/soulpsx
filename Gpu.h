#pragma once
#include <cstdint>
#include <span>

namespace std {
    enum class byte : unsigned char;
}

class Gpu {
public:
    void receive_command(uint32_t command);
    std::span<const std::byte> get_response(uint32_t physical_address);
private:
    uint32_t m_gpustat { 0x10000000 }; // Hardcoded response for now
    uint32_t m_dummy_var {};
};
