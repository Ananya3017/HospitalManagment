#pragma once

#include <string>
#include <utility>

namespace hms::core {

enum class UserRole {
    Admin,
    Doctor,
    Receptionist
};

inline std::string toString(UserRole role) {
    switch (role) {
        case UserRole::Admin:
            return "Admin";
        case UserRole::Doctor:
            return "Doctor";
        case UserRole::Receptionist:
            return "Receptionist";
    }

    return "Unknown";
}

class User final {
public:
    User(int id, std::string username, std::string password, UserRole role)
        : id_(id), username_(std::move(username)), password_(std::move(password)), role_(role) {}

    [[nodiscard]] int getId() const noexcept { return id_; }
    [[nodiscard]] const std::string& getUsername() const noexcept { return username_; }
    [[nodiscard]] const std::string& getPassword() const noexcept { return password_; }
    [[nodiscard]] UserRole getRole() const noexcept { return role_; }

private:
    int id_;
    std::string username_;
    std::string password_;
    UserRole role_;
};

}  // namespace hms::core
