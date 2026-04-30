#pragma once

#include "../core/Doctor.h"
#include "../core/Interfaces/IRepository.h"
#include "../core/User.h"
#include "../utils/PermissionManager.h"
#include "../utils/Logger.h"
#include "../utils/Validator.h"

#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hms::services {

class DoctorService final {
public:
    DoctorService(std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository,
                  std::shared_ptr<utils::Logger> logger)
        : doctorRepository_(std::move(doctorRepository)), logger_(std::move(logger)) {
        if (!doctorRepository_ || !logger_) {
            throw std::invalid_argument("DoctorService dependencies cannot be null.");
        }
    }

    void addDoctor(const std::string& name, const std::string& specialization, const core::User& currentUser) const {
        if (currentUser.getRole() != core::UserRole::Admin) {
            throw std::runtime_error("Only admins can add doctors.");
        }
        hms::utils::Validator::requireNonEmpty(name, "doctor name");
        hms::utils::Validator::requireNonEmpty(specialization, "doctor specialization");

        int id = doctorRepository_->getNextId();
        doctorRepository_->add(core::Doctor{id, name, specialization});
        logger_->info("Doctor added. ID: " + std::to_string(id));
    }

    void updateDoctor(int id, const std::string& name, const std::string& specialization, const core::User& currentUser) const {
        if (currentUser.getRole() != core::UserRole::Admin && currentUser.getId() != id) {
            throw std::runtime_error("You can only update your own profile.");
        }
        utils::Validator::requirePositiveId(id, "doctor id");
        hms::utils::Validator::requireNonEmpty(name, "doctor name");
        hms::utils::Validator::requireNonEmpty(specialization, "doctor specialization");

        doctorRepository_->update(core::Doctor{id, name, specialization});
        logger_->info("Doctor updated. ID: " + std::to_string(id));
    }

    void deleteDoctor(int id, const core::User& currentUser) const {
        if (currentUser.getRole() != core::UserRole::Admin) {
            throw std::runtime_error("Only admins can delete doctors.");
        }
        utils::Validator::requirePositiveId(id, "doctor id");
        doctorRepository_->remove(id);
        logger_->info("Doctor deleted. ID: " + std::to_string(id));
    }

    [[nodiscard]] std::optional<core::Doctor> getDoctorById(int id) const {
        utils::Validator::requirePositiveId(id, "doctor id");
        return doctorRepository_->getById(id);
    }

    [[nodiscard]] std::vector<core::Doctor> getAllDoctors(const core::User& currentUser) const {
        (void)currentUser; // All logged-in users can see the doctor list to book appointments
        return doctorRepository_->getAll();
    }

    [[nodiscard]] std::vector<std::string> getAllSpecializations(const core::User& currentUser) const {
        (void)currentUser;
        const auto doctors = doctorRepository_->getAll();
        std::set<std::string> specializations;
        for (const auto& doctor : doctors) {
            specializations.insert(doctor.getSpecialization());
        }
        return {specializations.begin(), specializations.end()};
    }

    [[nodiscard]] std::vector<core::Doctor> getDoctorsBySpecialization(const std::string& specialization, const core::User& currentUser) const {
        (void)currentUser;
        const auto doctors = doctorRepository_->getAll();
        std::vector<core::Doctor> filtered;
        for (const auto& doctor : doctors) {
            if (doctor.getSpecialization() == specialization) {
                filtered.push_back(doctor);
            }
        }
        return filtered;
    }

private:
    std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
