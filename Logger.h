#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

namespace ANSI_Colours {
    static std::string reset { "\033[0m" };
    static std::string grey { "\033[90m" };
    static std::string bright_red { "\033[91m" };
    static std::string bright_yellow { "\033[93m" };
    static std::string white { "\033[37m" };

}

class Logger {
public:
    enum class Level {
        debug,
        info,
        warning,
        error,
    };

    struct Entry {
        Level level { Level::info };
        std::string message {};
        std::chrono::time_point<std::chrono::system_clock> timestamp {};
    };

    static void log(Level level, std::string message) {
        if (level < m_min_level) return;

        const auto timestamp = std::chrono::system_clock::now();
        Entry entry { level, std::move(message), timestamp };
        m_entries.emplace_back(entry);
        std::cout << entry;

        if (m_entries.size() > m_max_entries) {
            m_entries.erase(m_entries.begin(), m_entries.begin() + 100);
        }
    }

    static void clear() { m_entries.clear(); }
    static const std::vector<Entry>& entries() { return m_entries; }

    friend std::ostream& operator<<(std::ostream& out, const Entry& entry) {
#ifdef _WIN32
        Handle console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(console, &mode);
        SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
        out << ANSI_Colours::grey << '[' << entry.timestamp << "]";
        out << level_colour_code(entry.level) << '[' <<  level_as_string(entry.level) << "]";
        out << entry.message << ANSI_Colours::reset << '\n';
        return out;
    }

    static std::string_view level_as_string(Level level) {
        switch (level) {
            case Level::debug: return "DEBUG";
            case Level::info: return "INFO";
            case Level::warning: return "WARNING";
            case Level::error: return "ERROR";
        }
        return "UNKNOWN";
    }

    static std::string_view level_colour_code(Level level) {
        switch (level) {
            case Level::debug: return ANSI_Colours::grey;
            case Level::info: return ANSI_Colours::white;
            case Level::warning: return ANSI_Colours::bright_yellow;
            case Level::error: return ANSI_Colours::bright_red;
        }
        return ANSI_Colours::reset;
    }
private:
    inline static std::vector<Entry> m_entries {};
    inline static uint32_t m_max_entries { 1000 };
    inline static Level m_min_level { Level::debug };
};
