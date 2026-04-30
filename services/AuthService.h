#pragma once

#include "../core/Interfaces/IUserRepository.h"
#include "../core/User.h"
#include "../utils/Logger.h"

#include "../utils/Validator.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace hms::services {

class AuthService final {
public:
    AuthService(std::shared_ptr<core::interfaces::IUserRepository> userRepository,
                std::shared_ptr<utils::Logger> logger)
        : userRepository_(std::move(userRepository)), logger_(std::move(logger)) {
        if (!userRepository_ || !logger_) {
            throw std::invalid_argument("AuthService dependencies cannot be null.");
        }
    }

    [[nodiscard]] bool hasAnyUser() const {
        return !userRepository_->getAll().empty();
    }

    void registerUser(int id, const std::string& username, const std::string& password, core::UserRole role) const {
        utils::Validator::requirePositiveId(id, "user id");
        utils::Validator::requireNonEmpty(username, "username");
        
        if (userRepository_->getByUsername(username).has_value()) {
            throw std::runtime_error("Username already exists.");
        }

        userRepository_->add(core::User{id, username, password, role});
        logger_->info("User registered. Username: " + username);
    }

    [[nodiscard]] std::unique_ptr<core::User> login(const std::string& username, const std::string& password) const {
        utils::Validator::requireNonEmpty(username, "username");
        utils::Validator::requireNonEmpty(password, "password");

        const auto user = userRepository_->getByUsername(username);
        if (user.has_value() && user->getPasswordHash() == password) {
            logger_->info("User login succeeded. Username: " + username);
            return std::make_unique<core::User>(*user);
        }

        logger_->error("User login failed. Username: " + username);
        throw std::runtime_error("Invalid username or password.");
    }

    [[nodiscard]] std::unique_ptr<core::User> loginWithRole(const std::string& username, const std::string& password, core::UserRole expectedRole) const {
        utils::Validator::requireNonEmpty(username, "username");
        utils::Validator::requireNonEmpty(password, "password");

        const auto user = userRepository_->getByUsername(username);
        if (user.has_value() && user->getPasswordHash() == password) {
            if (user->getRole() != expectedRole) {
                logger_->error("User login failed - role mismatch. Username: " + username + ", Expected role: " + core::toString(expectedRole) + ", Actual role: " + core::toString(user->getRole()));
                throw std::runtime_error("Invalid role for this user.");
            }
            logger_->info("User login succeeded with role validation. Username: " + username + ", Role: " + core::toString(expectedRole));
            return std::make_unique<core::User>(*user);
        }

        logger_->error("User login failed. Username: " + username);
        throw std::runtime_error("Invalid username or password.");
    }

    [[nodiscard]] std::optional<core::User> getUserByUsername(const std::string& username) const {
        utils::Validator::requireNonEmpty(username, "username");
        return userRepository_->getByUsername(username);
    }

    void seedDemoUsers() const {
        if (hasAnyUser()) {
            return;
        }

        try {
            registerUser(1001, "admin", "admin123", core::UserRole::Admin);
            registerUser(1002, "doctor", "doc123", core::UserRole::Doctor);
            registerUser(1003, "patient", "pat123", core::UserRole::Patient);
            registerUser(1004, "reception", "rec123", core::UserRole::Receptionist);
            logger_->info("Demo users seeded successfully.");
        } catch (const std::exception& e) {
            logger_->error("Failed to seed demo users: " + std::string(e.what()));
        }
    }

private:
    std::shared_ptr<core::interfaces::IUserRepository> userRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
