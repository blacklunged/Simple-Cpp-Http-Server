#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>

class Logger {
public:
    enum class Level {
        Info,
        Error
    };

    static void init(const std::string& info_log_path,
                     const std::string& error_log_path);
    static void logInfo(const std::string& message);

    static void logError(const std::string& message);

private:
    static std::ofstream s_info_log;
    static std::ofstream s_error_log;
    static std::mutex s_mutex;
    static bool s_initialized;

    static std::string currentTimestamp();
    static void log(Level level, const std::string& message);
};

#endif 
