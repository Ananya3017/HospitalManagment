#pragma once

#include "../core/Patient.h"
#include "../core/Interfaces/IRepository.h"
#include "../utils/Logger.h"
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

    void registerPatient(int id, const std::string& name, int age, const std::string& contactNumber) const {
        utils::Validator::requirePositiveId(id, "patient id");
        utils::Validator::requireNonEmpty(name, "patient name");
        utils::Validator::requireValidAge(age);
        utils::Validator::requireNonEmpty(contactNumber, "contact number");

        patientRepository_->add(core::Patient{id, name, age, contactNumber});
        logger_->info("Patient registered. ID: " + std::to_string(id));
    }

    void updatePatient(int id, const std::string& name, int age, const std::string& contactNumber) const {
        utils::Validator::requirePositiveId(id, "patient id");
        utils::Validator::requireNonEmpty(name, "patient name");
        utils::Validator::requireValidAge(age);
        utils::Validator::requireNonEmpty(contactNumber, "contact number");

        patientRepository_->update(core::Patient{id, name, age, contactNumber});
        logger_->info("Patient updated. ID: " + std::to_string(id));
    }

    void deletePatient(int id) const {
        utils::Validator::requirePositiveId(id, "patient id");
        patientRepository_->remove(id);
        logger_->info("Patient deleted. ID: " + std::to_string(id));
    }

    [[nodiscard]] std::optional<core::Patient> getPatientById(int id) const {
        utils::Validator::requirePositiveId(id, "patient id");
        return patientRepository_->getById(id);
    }

    [[nodiscard]] std::vector<core::Patient> getAllPatients() const { return patientRepository_->getAll(); }

private:
    std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
