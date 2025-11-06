#include "System.h"

void System::run() {
    if (m_pause_system) {
        return;
    }
    m_cpu.fetch_decode_execute();
}

void System::pause(bool pause_state) {
    m_pause_system = pause_state;
}

