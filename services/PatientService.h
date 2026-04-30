#pragma once

#include "../core/Patient.h"
#include "../core/Interfaces/IRepository.h"
#include "../core/User.h"
#include "../utils/Logger.h"
#include "../utils/PermissionManager.h"
#include "../utils/Validator.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hms::services {

class PatientService final {
public:
    PatientService(std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository,
                   std::shared_ptr<utils::Logger> logger)
        : patientRepository_(std::move(patientRepository)), logger_(std::move(logger)) {
        if (!patientRepository_ || !logger_) {
            throw std::invalid_argument("PatientService dependencies cannot be null.");
        }
    }

    void registerPatient(const std::string& name, int age, const std::string& contactNumber, const core::User& currentUser) const {
        if (!utils::PermissionManager::canManagePatients(currentUser)) {
            throw std::runtime_error("Insufficient permissions to register patients");
        }
        utils::Validator::requireNonEmpty(name, "patient name");
        utils::Validator::requireValidAge(age);
        utils::Validator::requireNonEmpty(contactNumber, "contact number");

        int id = patientRepository_->getNextId();
        patientRepository_->add(core::Patient{id, name, age, contactNumber});
        logger_->info("Patient registered. ID: " + std::to_string(id));
    }

    void updatePatient(int id, const std::string& name, int age, const std::string& contactNumber, const core::User& currentUser) const {
        if (!utils::PermissionManager::canManagePatients(currentUser) && currentUser.getId() != id) {
            throw std::runtime_error("Insufficient permissions to update this patient");
        }
        utils::Validator::requirePositiveId(id, "patient id");
        utils::Validator::requireNonEmpty(name, "patient name");
        utils::Validator::requireValidAge(age);
        utils::Validator::requireNonEmpty(contactNumber, "contact number");

        patientRepository_->update(core::Patient{id, name, age, contactNumber});
        logger_->info("Patient updated. ID: " + std::to_string(id));
    }

    void deletePatient(int id, const core::User& currentUser) const {
        if (!utils::PermissionManager::canDeletePatients(currentUser)) {
            throw std::runtime_error("Insufficient permissions to delete patients");
        }
        utils::Validator::requirePositiveId(id, "patient id");
        patientRepository_->remove(id);
        logger_->info("Patient deleted. ID: " + std::to_string(id));
    }

    [[nodiscard]] std::optional<core::Patient> getPatientById(int id, const core::User& currentUser) const {
        utils::Validator::requirePositiveId(id, "patient id");
        if (currentUser.getRole() == core::UserRole::Patient && currentUser.getId() != id) {
            throw std::runtime_error("Patients can only access their own records");
        }
        return patientRepository_->getById(id);
    }

    [[nodiscard]] std::vector<core::Patient> getAllPatients(const core::User& currentUser) const {
        if (currentUser.getRole() == core::UserRole::Patient) {
            // Patients can only see their own record
            auto patient = patientRepository_->getById(currentUser.getId());
            return patient ? std::vector{*patient} : std::vector<core::Patient>{};
        }
        return patientRepository_->getAll();
    }

private:
    std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
