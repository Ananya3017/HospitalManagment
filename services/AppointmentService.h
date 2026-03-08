#pragma once

#include "../core/Appointment.h"
#include "../core/Doctor.h"
#include "../core/Interfaces/IRepository.h"
#include "../core/Patient.h"
#include "../utils/Logger.h"
#include "../utils/Validator.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hms::services {

class AppointmentService final {
public:
    AppointmentService(
        std::shared_ptr<core::interfaces::IRepository<core::Appointment>> appointmentRepository,
        std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository,
        std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository,
        std::shared_ptr<utils::Logger> logger)
        : appointmentRepository_(std::move(appointmentRepository)),
          patientRepository_(std::move(patientRepository)),
          doctorRepository_(std::move(doctorRepository)),
          logger_(std::move(logger)) {
        if (!appointmentRepository_ || !patientRepository_ || !doctorRepository_ || !logger_) {
            throw std::invalid_argument("AppointmentService dependencies cannot be null.");
        }
    }

    void bookAppointment(int appointmentId, int patientId, int doctorId, const std::string& dateTime) const {
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        utils::Validator::requirePositiveId(patientId, "patient id");
        utils::Validator::requirePositiveId(doctorId, "doctor id");
        utils::Validator::requireNonEmpty(dateTime, "appointment date/time");

        if (!patientRepository_->getById(patientId).has_value()) {
            throw std::runtime_error("Patient not found for appointment booking.");
        }

        if (!doctorRepository_->getById(doctorId).has_value()) {
            throw std::runtime_error("Doctor not found for appointment booking.");
        }

        appointmentRepository_->add(
            core::Appointment{appointmentId, patientId, doctorId, dateTime, core::AppointmentStatus::Scheduled});
        logger_->info("Appointment booked. ID: " + std::to_string(appointmentId));
    }

    void updateAppointment(int appointmentId,
                           int patientId,
                           int doctorId,
                           const std::string& dateTime,
                           core::AppointmentStatus status) const {
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        utils::Validator::requirePositiveId(patientId, "patient id");
        utils::Validator::requirePositiveId(doctorId, "doctor id");
        utils::Validator::requireNonEmpty(dateTime, "appointment date/time");

        if (!patientRepository_->getById(patientId).has_value()) {
            throw std::runtime_error("Patient not found for appointment update.");
        }

        if (!doctorRepository_->getById(doctorId).has_value()) {
            throw std::runtime_error("Doctor not found for appointment update.");
        }

        appointmentRepository_->update(core::Appointment{appointmentId, patientId, doctorId, dateTime, status});
        logger_->info("Appointment updated. ID: " + std::to_string(appointmentId));
    }

    void deleteAppointment(int appointmentId) const {
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        appointmentRepository_->remove(appointmentId);
        logger_->info("Appointment deleted. ID: " + std::to_string(appointmentId));
    }

    [[nodiscard]] std::optional<core::Appointment> getAppointmentById(int appointmentId) const {
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        return appointmentRepository_->getById(appointmentId);
    }

    [[nodiscard]] std::vector<core::Appointment> getAllAppointments() const {
        return appointmentRepository_->getAll();
    }

private:
    std::shared_ptr<core::interfaces::IRepository<core::Appointment>> appointmentRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
