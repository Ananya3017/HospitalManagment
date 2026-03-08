#pragma once

#include <algorithm>
#include <cctype>
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
};

}  // namespace hms::utils
