#pragma once

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace hms::utils {

class Validator final {
public:
    static void requirePositiveId(int value, const std::string& fieldName) {
        if (value <= 0) {
            throw std::invalid_argument(fieldName + " must be greater than zero.");
        }
    }

    static void requireValidAge(int age) {
        if (age <= 0 || age > 130) {
            throw std::invalid_argument("age must be between 1 and 130.");
        }
    }

    static void requirePositiveAmount(double amount) {
        if (amount <= 0.0) {
            throw std::invalid_argument("amount must be greater than zero.");
        }
    }

    static void requireNonEmpty(const std::string& value, const std::string& fieldName) {
        if (value.empty() ||
            std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; })) {
            throw std::invalid_argument(fieldName + " cannot be empty.");
        }
    }

    static void requireFutureDate(const std::string& dateTime) {
        if (dateTime.empty()) {
            throw std::invalid_argument("date/time cannot be empty.");
        }

        std::tm tm{};
        std::istringstream ss(dateTime);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
        if (ss.fail()) {
            throw std::invalid_argument("date/time must be in format YYYY-MM-DD HH:MM.");
        }

        tm.tm_isdst = -1;
        const std::time_t parsed = std::mktime(&tm);
        if (parsed == -1) {
            throw std::invalid_argument("date/time value is out of range.");
        }
        if (parsed <= std::time(nullptr)) {
            throw std::runtime_error("date/time must be in the future.");
        }
    }
};

}  // namespace hms::utils
