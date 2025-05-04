#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string& logFilePath, bool consoleOutput) : m_consoleOutput(consoleOutput) {
    // Open log file
    m_logFile.open(logFilePath, std::ios::out | std::ios::app);
    
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
    
    Log("Logger initialized", LogLevel::INFO);
}

Logger::~Logger() {
    Log("Logger shutting down", LogLevel::INFO);
    
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::Log(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string timestamp = GetTimestamp();
    std::string levelStr = LogLevelToString(level);
    
    std::string logEntry = timestamp + " [" + levelStr + "] " + message;
    
    // Add to memory log
    m_logs.push_back(logEntry);
    
    // Output to console if enabled
    if (m_consoleOutput) {
        std::cout << logEntry << std::endl;
    }
    
    // Write to log file
    if (m_logFile.is_open()) {
        m_logFile << logEntry << std::endl;
        m_logFile.flush();
    }
}

void Logger::LogWarning(const std::string& message) {
    Log(message, LogLevel::WARNING);
}

void Logger::LogError(const std::string& message) {
    Log(message, LogLevel::ERR);  // Changed from ERROR to ERR
}

void Logger::LogDebug(const std::string& message) {
    Log(message, LogLevel::DEBUG);
}

const std::vector<std::string>& Logger::GetLogs() const {
    return m_logs;
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    
#ifdef _WIN32
    // Windows safe version
    struct tm timeinfo;
    localtime_s(&timeinfo, &time);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
#else
    // Unix version
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
#endif
    
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERR:      // Changed from ERROR to ERR
            return "ERROR";      // Keep the output string as "ERROR"
        case LogLevel::DEBUG:
            return "DEBUG";
        default:
            return "UNKNOWN";
    }
}
