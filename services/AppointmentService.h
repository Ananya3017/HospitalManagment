#pragma once

#include "../core/Appointment.h"
#include "../core/Doctor.h"
#include "../core/Interfaces/IAppointmentRepository.h"
#include "../database/DatabaseManager.h"
#include "../database/Transaction.h"
#include "../core/Patient.h"
#include "../core/User.h"
#include "../utils/Logger.h"
#include "../utils/Validator.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hms::services {

class AppointmentService final {
public:
    AppointmentService(
        std::shared_ptr<core::interfaces::IAppointmentRepository> appointmentRepository,
        std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository,
        std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository,
        database::DatabaseManager& databaseManager,
        std::shared_ptr<utils::Logger> logger)
        : appointmentRepository_(std::move(appointmentRepository)),
          patientRepository_(std::move(patientRepository)),
          doctorRepository_(std::move(doctorRepository)),
          databaseManager_(databaseManager),
          logger_(std::move(logger)) {
        if (!appointmentRepository_ || !patientRepository_ || !doctorRepository_ || !logger_) {
            throw std::invalid_argument("AppointmentService dependencies cannot be null.");
        }
    }

    void bookAppointment(int patientId, int doctorId, const std::string& dateTime, const core::User& currentUser) const {
        if (currentUser.getRole() != core::UserRole::Admin && currentUser.getRole() != core::UserRole::Receptionist) {
            throw std::runtime_error("Only Receptionist or Admin can book appointments directly.");
        }
        
        hms::utils::Validator::requirePositiveId(patientId, "patient id");
        hms::utils::Validator::requirePositiveId(doctorId, "doctor id");
        hms::utils::Validator::requireFutureDate(dateTime);

        executeBooking(appointmentRepository_->getNextId(), patientId, doctorId, dateTime, core::AppointmentStatus::Scheduled);
    }

    void requestAppointment(int doctorId, const std::string& dateTime, const core::User& currentUser) const {
        if (currentUser.getRole() != core::UserRole::Patient) {
            throw std::runtime_error("Only Patients can request appointments.");
        }
        
        hms::utils::Validator::requirePositiveId(doctorId, "doctor id");
        hms::utils::Validator::requireFutureDate(dateTime);

        executeBooking(appointmentRepository_->getNextId(), currentUser.getId(), doctorId, dateTime, core::AppointmentStatus::Scheduled);
    }

    void updateAppointment(int appointmentId, int patientId, int doctorId, const std::string& dateTime, core::AppointmentStatus status, const core::User& currentUser) const {
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        
        auto current = appointmentRepository_->getById(appointmentId);
        if (!current) throw std::runtime_error("Appointment not found.");

        if (currentUser.getRole() == core::UserRole::Doctor) {
            if (current->getDoctorId() != currentUser.getId()) {
                throw std::runtime_error("Doctors can only update their own appointments.");
            }
            // Doctors can only change status to Completed
            if (status != core::AppointmentStatus::Completed) {
                throw std::runtime_error("Doctors can only mark appointments as Completed.");
            }
            // But we ignore other changes for security if called accidentally
            appointmentRepository_->update(core::Appointment{appointmentId, current->getPatientId(), current->getDoctorId(), current->getDateTime(), status});
            logger_->info("Appointment status updated by Doctor. ID: " + std::to_string(appointmentId));
            return;
        }

        if (currentUser.getRole() != core::UserRole::Admin && currentUser.getRole() != core::UserRole::Receptionist) {
            throw std::runtime_error("Insufficient permissions to update appointment.");
        }

        utils::Validator::requirePositiveId(patientId, "patient id");
        utils::Validator::requirePositiveId(doctorId, "doctor id");

        database::Transaction transaction(databaseManager_);
        ensureRelatedEntitiesExist(patientId, doctorId, "update");
        ensureDoctorAvailable(doctorId, dateTime, appointmentId);
        ensurePatientAvailable(patientId, dateTime, appointmentId);

        appointmentRepository_->update(core::Appointment{appointmentId, patientId, doctorId, dateTime, status});
        transaction.commit();
        logger_->info("Appointment updated. ID: " + std::to_string(appointmentId));
    }

    void deleteAppointment(int appointmentId, const core::User& currentUser) const {
        if (currentUser.getRole() != core::UserRole::Admin) {
            throw std::runtime_error("Only Admin can delete appointments.");
        }
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        appointmentRepository_->remove(appointmentId);
        logger_->info("Appointment deleted. ID: " + std::to_string(appointmentId));
    }

    [[nodiscard]] std::optional<core::Appointment> getAppointmentById(int appointmentId) const {
        hms::utils::Validator::requirePositiveId(appointmentId, "appointment id");
        return appointmentRepository_->getById(appointmentId);
    }

    [[nodiscard]] std::vector<core::Appointment> getAllAppointments(const core::User& currentUser) const {
        switch (currentUser.getRole()) {
            case core::UserRole::Patient:
                return appointmentRepository_->getByPatientId(currentUser.getId());
            case core::UserRole::Doctor:
                return appointmentRepository_->getByDoctorId(currentUser.getId());
            default:
                return appointmentRepository_->getAll();
        }
    }

private:
    void executeBooking(int id, int patientId, int doctorId, const std::string& dateTime, core::AppointmentStatus status) const {
        database::Transaction transaction(databaseManager_);
        ensureRelatedEntitiesExist(patientId, doctorId, "booking");
        ensureDoctorAvailable(doctorId, dateTime, std::nullopt);
        ensurePatientAvailable(patientId, dateTime, std::nullopt);

        appointmentRepository_->add(core::Appointment{id, patientId, doctorId, dateTime, status});
        transaction.commit();
        logger_->info("Appointment processed. ID: " + std::to_string(id));
    }

    void ensureDoctorAvailable(int doctorId, const std::string& dateTime, std::optional<int> excludeAppointmentId) const {
        if (appointmentRepository_->doctorHasAppointmentAt(doctorId, dateTime, excludeAppointmentId)) {
            throw std::runtime_error("Doctor is already booked for the selected time.");
        }
    }

    void ensurePatientAvailable(int patientId, const std::string& dateTime, std::optional<int> excludeAppointmentId) const {
        if (appointmentRepository_->patientHasAppointmentAt(patientId, dateTime, excludeAppointmentId)) {
            throw std::runtime_error("Patient already has an appointment at the selected time.");
        }
    }

    static std::time_t parseDateTime(const std::string& dateTime) {
        std::tm tmSnapshot{};
        std::istringstream input(dateTime);
        input >> std::get_time(&tmSnapshot, "%Y-%m-%d %H:%M");
        if (input.fail()) {
            throw std::invalid_argument("appointment date/time must use format YYYY-MM-DD HH:MM.");
        }

        tmSnapshot.tm_sec = 0;
        const std::time_t parsed = std::mktime(&tmSnapshot);
        if (parsed == static_cast<std::time_t>(-1)) {
            throw std::invalid_argument("appointment date/time is invalid.");
        }
        return parsed;
    }

    static void requireFutureDateTime(const std::string& dateTime, const std::string& message) {
        const std::time_t scheduledTime = parseDateTime(dateTime);
        const std::time_t currentTime = std::time(nullptr);
        if (scheduledTime < currentTime) {
            throw std::runtime_error(message);
        }
    }

    static void validateAppointmentBasics(int appointmentId, int patientId, int doctorId, const std::string& dateTime) {
        hms::utils::Validator::requirePositiveId(appointmentId, "appointment id");
        hms::utils::Validator::requirePositiveId(patientId, "patient id");
        hms::utils::Validator::requirePositiveId(doctorId, "doctor id");
        hms::utils::Validator::requireNonEmpty(dateTime, "appointment date/time");
        (void)parseDateTime(dateTime);
    }

    void ensureRelatedEntitiesExist(int patientId, int doctorId, const std::string& action) const {
        if (!patientRepository_->getById(patientId).has_value()) {
            throw std::runtime_error("Patient not found for appointment " + action + ".");
        }

        if (!doctorRepository_->getById(doctorId).has_value()) {
            throw std::runtime_error("Doctor not found for appointment " + action + ".");
        }
    }

    void ensureNoSchedulingConflict(int appointmentId,
                                    int patientId,
                                    int doctorId,
                                    const std::string& dateTime,
                                    std::optional<int> excludeAppointmentId) const {
        (void)appointmentId;
        if (appointmentRepository_->doctorHasAppointmentAt(doctorId, dateTime, excludeAppointmentId)) {
            throw std::runtime_error("Doctor is already booked for the selected time.");
        }
        if (appointmentRepository_->patientHasAppointmentAt(patientId, dateTime, excludeAppointmentId)) {
            throw std::runtime_error("Patient already has an appointment at the selected time.");
        }
    }

    std::shared_ptr<core::interfaces::IAppointmentRepository> appointmentRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository_;
    database::DatabaseManager& databaseManager_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
