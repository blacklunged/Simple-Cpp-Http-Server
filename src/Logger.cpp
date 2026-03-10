#include "../include/Logger.hpp"

std::ofstream Logger::s_info_log;
std::ofstream Logger::s_error_log;
std::mutex Logger::s_mutex;
bool Logger::s_initialized = false;

void Logger::init(const std::string& info_log_path,
                  const std::string& error_log_path)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (s_initialized) {
        return;
    }

    s_info_log.open(info_log_path, std::ios::app);
    s_error_log.open(error_log_path, std::ios::app);

    if (!s_info_log.is_open() || !s_error_log.is_open()) {
        throw std::runtime_error("Failed to open log files");
    }

    s_initialized = true;
}

void Logger::logInfo(const std::string& message) {
    log(Level::Info, message);
}

void Logger::logError(const std::string& message) {
    log(Level::Error, message);
}

std::string Logger::currentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_initialized) return;

    std::ostream* out = nullptr;
    const char* level_str = nullptr;

    switch (level) {
        case Level::Info:
            out = &s_info_log;
            level_str = "INFO";
            break;
        case Level::Error:
            out = &s_error_log;
            level_str = "ERROR";
            break;
    }

    if (!out) return;

    (*out) << "[" << currentTimestamp() << "] "
           << level_str << " "
           << message << std::endl;
}
