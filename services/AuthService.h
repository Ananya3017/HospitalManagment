#pragma once

#include "../core/Interfaces/IRepository.h"
#include "../core/User.h"
#include "../utils/Logger.h"
#include "../utils/Validator.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace hms::services {

class AuthService final {
public:
    AuthService(std::shared_ptr<core::interfaces::IRepository<core::User>> userRepository,
                std::shared_ptr<utils::Logger> logger)
        : userRepository_(std::move(userRepository)), logger_(std::move(logger)) {
        if (!userRepository_ || !logger_) {
            throw std::invalid_argument("AuthService dependencies cannot be null.");
        }
    }

    void registerUser(int id, const std::string& username, const std::string& password, core::UserRole role) const {
        utils::Validator::requirePositiveId(id, "user id");
        utils::Validator::requireNonEmpty(username, "username");
        utils::Validator::requireNonEmpty(password, "password");

        userRepository_->add(core::User{id, username, password, role});
        logger_->info("User registered. Username: " + username);
    }

    [[nodiscard]] std::unique_ptr<core::User> login(const std::string& username, const std::string& password) const {
        utils::Validator::requireNonEmpty(username, "username");
        utils::Validator::requireNonEmpty(password, "password");

        for (const auto& user : userRepository_->getAll()) {
            if (user.getUsername() == username && user.getPassword() == password) {
                logger_->info("User login succeeded. Username: " + username);
                return std::make_unique<core::User>(user);
            }
        }

        logger_->error("User login failed. Username: " + username);
        throw std::runtime_error("Invalid username or password.");
    }

private:
    std::shared_ptr<core::interfaces::IRepository<core::User>> userRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
