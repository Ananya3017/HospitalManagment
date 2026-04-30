#pragma once

#include "../core/Appointment.h"
#include "../core/Billing.h"
#include "../core/Doctor.h"
#include "../core/Interfaces/IAppointmentRepository.h"
#include "../core/Interfaces/IBillingRepository.h"
#include "../core/Interfaces/IRepository.h"
#include "../core/Interfaces/IUserRepository.h"
#include "../core/Patient.h"
#include "../core/User.h"
#include "../database/DatabaseManager.h"
#include "../services/AppointmentService.h"
#include "../services/AuthService.h"
#include "../services/BillingService.h"
#include "../services/DoctorService.h"
#include "../services/PatientService.h"
#include "../utils/Logger.h"

#include <filesystem>
#include <memory>

namespace hms::app {

class HospitalContext final {
public:
    explicit HospitalContext(std::filesystem::path databasePath);

    services::PatientService& patientService() noexcept { return patientService_; }
    services::DoctorService& doctorService() noexcept { return doctorService_; }
    services::AppointmentService& appointmentService() noexcept { return appointmentService_; }
    services::BillingService& billingService() noexcept { return billingService_; }
    services::AuthService& authService() noexcept { return authService_; }

private:
    void seedDefaultUsers();

    std::shared_ptr<utils::Logger> logger_;
    std::unique_ptr<database::DatabaseManager> databaseManager_;

    std::shared_ptr<core::interfaces::IRepository<core::Patient>> patientRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Doctor>> doctorRepository_;
    std::shared_ptr<core::interfaces::IAppointmentRepository> appointmentRepository_;
    std::shared_ptr<core::interfaces::IBillingRepository> billingRepository_;
    std::shared_ptr<core::interfaces::IUserRepository> userRepository_;

    services::PatientService patientService_;
    services::DoctorService doctorService_;
    services::AppointmentService appointmentService_;
    services::BillingService billingService_;
    services::AuthService authService_;

};

}  // namespace hms::app
