#pragma once
#include <cstdint>

namespace Memory {
    class Range {
        public:
        constexpr Range(const uint32_t start, const uint32_t size)
            : m_start { start }, m_size { size }, m_end { m_start + m_size } {
        }
        constexpr uint32_t start() const { return m_start; }
        constexpr uint32_t end() const { return m_end; }
        constexpr uint32_t size() const { return m_size; }

        // Checks if the given address is within bounds of the memory range.
        constexpr bool contains(uint32_t address) const {
            return address >= m_start && address < m_end;
        }

        // Gets the offset of the address from the start of the range.
        constexpr uint32_t offset(uint32_t address) const {
            return address - m_start;
        }
    private:
        uint32_t m_start {};
        uint32_t m_size {};
        uint32_t m_end {};
    };
}

namespace Memory::Map {
    static constexpr Range bios { 0x1fc00000, 512 * 1024 };
    static constexpr Range ram { 0x0, 2048 * 1024 };
    static constexpr Range gpu { 0x1f801810, 8 };
    static constexpr Range irq_control { 0x1f801070, 8 };
    static constexpr Range timers { 0x1f801100, 0x2c };
    static constexpr Range cache_control { 0xfffe0130, 4 };
    static constexpr Range expansion_region_1 { 0x1f000000, 1024 * 1024 };
    static constexpr Range expansion_region_2 { 0x1f802000, 0x80 };
    static constexpr Range dma { 0x1f801080, 0x80 };
    static constexpr Range mem_control_1 { 0x1f801000, 0x24 };
    static constexpr Range mem_control_2 { 0x1f801060, 4 };
    static constexpr Range spu { 0x1f801c00, 0x280 };
}
