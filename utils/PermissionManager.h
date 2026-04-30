#pragma once

#include "../core/User.h"

namespace hms::utils {

class PermissionManager {
public:
    static bool canManagePatients(const core::User& user) {
        const auto role = user.getRole();
        return role == core::UserRole::Admin || role == core::UserRole::Receptionist;
    }

    static bool canManageDoctors(const core::User& user) {
        return user.getRole() == core::UserRole::Admin;
    }

    static bool canManageAppointments(const core::User& user) {
        return true; // All logged-in users can manage appointments, but with restrictions
    }

    static bool canManageBilling(const core::User& user) {
        const auto role = user.getRole();
        return role == core::UserRole::Admin || role == core::UserRole::Receptionist;
    }

    static bool canViewAllPatients(const core::User& user) {
        return user.getRole() != core::UserRole::Patient;
    }

    static bool canViewDoctors(const core::User& user) {
        return user.getRole() != core::UserRole::Patient;
    }

    static bool canDeletePatients(const core::User& user) {
        return user.getRole() == core::UserRole::Admin;
    }

    static bool canDeleteAppointments(const core::User& user) {
        return user.getRole() != core::UserRole::Patient;
    }

    static bool canDeleteBills(const core::User& user) {
        return user.getRole() == core::UserRole::Admin;
    }

    static bool canAccessPatientPage(const core::User& user) {
        return user.getRole() != core::UserRole::Patient;
    }

    static bool canAccessDoctorPage(const core::User& user) {
        const auto role = user.getRole();
        return role == core::UserRole::Admin || role == core::UserRole::Receptionist;
    }

    static bool canAccessBillingPage(const core::User& user) {
        return user.getRole() != core::UserRole::Doctor;
    }
};

}  // namespace hms::utils