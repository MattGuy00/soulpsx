#include "System.h"

void System::run() {
   m_cpu.fetch_decode_execute();
}
