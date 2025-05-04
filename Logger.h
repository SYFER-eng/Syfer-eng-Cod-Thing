#pragma once

// Disable warnings about unsafe functions in Visual Studio
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996) // Disable deprecation warnings
#endif

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <vector>

enum class LogLevel {
    INFO,
    WARNING,
    ERR,  // Renamed from ERROR to avoid conflict with Windows macro
    DEBUG
};

class Logger {
public:
    Logger(const std::string& logFilePath = "SyferInjector.log", bool consoleOutput = true);
    ~Logger();

    void Log(const std::string& message, LogLevel level = LogLevel::INFO);
    void LogWarning(const std::string& message);
    void LogError(const std::string& message);
    void LogDebug(const std::string& message);

    // Return all log entries
    const std::vector<std::string>& GetLogs() const;

private:
    std::string GetTimestamp();
    std::string LogLevelToString(LogLevel level);

    std::ofstream m_logFile;
    bool m_consoleOutput;
    std::mutex m_mutex;
    std::vector<std::string> m_logs;
};
