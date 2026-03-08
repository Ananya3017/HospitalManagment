#pragma once

#include "../core/Doctor.h"
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

class DoctorService final {
public:
    DoctorService(std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository,
                  std::shared_ptr<utils::Logger> logger)
        : doctorRepository_(std::move(doctorRepository)), logger_(std::move(logger)) {
        if (!doctorRepository_ || !logger_) {
            throw std::invalid_argument("DoctorService dependencies cannot be null.");
        }
    }

    void addDoctor(int id, const std::string& name, const std::string& specialization) const {
        utils::Validator::requirePositiveId(id, "doctor id");
        utils::Validator::requireNonEmpty(name, "doctor name");
        utils::Validator::requireNonEmpty(specialization, "doctor specialization");

        doctorRepository_->add(core::Doctor{id, name, specialization});
        logger_->info("Doctor added. ID: " + std::to_string(id));
    }

    void updateDoctor(int id, const std::string& name, const std::string& specialization) const {
        utils::Validator::requirePositiveId(id, "doctor id");
        utils::Validator::requireNonEmpty(name, "doctor name");
        utils::Validator::requireNonEmpty(specialization, "doctor specialization");

        doctorRepository_->update(core::Doctor{id, name, specialization});
        logger_->info("Doctor updated. ID: " + std::to_string(id));
    }

    void deleteDoctor(int id) const {
        utils::Validator::requirePositiveId(id, "doctor id");
        doctorRepository_->remove(id);
        logger_->info("Doctor deleted. ID: " + std::to_string(id));
    }

    [[nodiscard]] std::optional<core::Doctor> getDoctorById(int id) const {
        utils::Validator::requirePositiveId(id, "doctor id");
        return doctorRepository_->getById(id);
    }

    [[nodiscard]] std::vector<core::Doctor> getAllDoctors() const { return doctorRepository_->getAll(); }

private:
    std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
