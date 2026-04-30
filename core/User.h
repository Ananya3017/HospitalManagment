#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace hms::core {

enum class UserRole { Admin, Doctor, Receptionist, Patient };

inline std::string toString(UserRole role) {
    switch (role) {
        case UserRole::Admin: return "Admin";
        case UserRole::Doctor: return "Doctor";
        case UserRole::Receptionist: return "Receptionist";
        case UserRole::Patient: return "Patient";
    }
    return "Unknown";
}

inline UserRole userRoleFromString(const std::string& value) {
    if (value == "Admin") return UserRole::Admin;
    if (value == "Doctor") return UserRole::Doctor;
    if (value == "Receptionist") return UserRole::Receptionist;
    if (value == "Patient") return UserRole::Patient;
    throw std::invalid_argument("Unknown user role: " + value);
}

class User final {
public:
    User(int id, std::string username, std::string passwordHash, UserRole role)
        : id_(id), username_(std::move(username)), passwordHash_(std::move(passwordHash)), role_(role) {}

    [[nodiscard]] int getId() const noexcept { return id_; }
    [[nodiscard]] const std::string& getUsername() const noexcept { return username_; }
    [[nodiscard]] const std::string& getName() const noexcept { return username_; }
    [[nodiscard]] const std::string& getPasswordHash() const noexcept { return passwordHash_; }
    [[nodiscard]] UserRole getRole() const noexcept { return role_; }

private:
    int id_;
    std::string username_;
    std::string passwordHash_;
    UserRole role_;
};

}  // namespace hms::core
