#pragma once // S - used only once

#include <chrono> // current sys time 
#include <ctime> // conv to local time
#include <iomanip> // format time
#include <iostream> 
#include <sstream>
#include <string>

namespace hms::utils {

class Logger final {
public:
    void info(const std::string& message) const { write("INFO", message, std::cout); }
    void error(const std::string& message) const { write("ERROR", message, std::cerr); }

private:
    static std::string timestamp() {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tmSnapshot{};
#ifdef _WIN32
        localtime_s(&tmSnapshot, &time);
#else
        localtime_r(&time, &tmSnapshot);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tmSnapshot, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    static void write(const std::string& level, const std::string& message, std::ostream& stream) {
        stream << "[" << timestamp() << "] [" << level << "] " << message << '\n';
    }
};

}  // namespace hms::utils
