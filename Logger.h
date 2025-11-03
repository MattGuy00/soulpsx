#pragma once
#include <string>
#include <vector>
#include <chrono>

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
        m_entries.emplace_back(Entry{ level, std::move(message), timestamp });

        if (m_entries.size() > m_max_entries) {
            m_entries.erase(m_entries.begin(), m_entries.begin() + 100);
        }
    }

    static void clear() { m_entries.clear(); }
    static const std::vector<Entry>& entries() { return m_entries; }

    friend std::ostream& operator<<(std::ostream& out, const Entry& entry) {
        out << '[' << entry.timestamp << "]";
        out << '[' <<  level_as_string(entry.level) << "]";
        out << entry.message << '\n';
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
private:
    inline static std::vector<Entry> m_entries {};
    inline static uint32_t m_max_entries { 1000 };
    inline static Level m_min_level { Level::debug };
};
