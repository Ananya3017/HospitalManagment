#pragma once

#include "../core/Appointment.h"
#include "../core/Billing.h"
#include "../core/Doctor.h"
#include "../core/Patient.h"
#include "../core/User.h"
#include "../infrastructure/repositories/InMemoryRepository.h"
#include "../services/AppointmentService.h"
#include "../services/AuthService.h"
#include "../services/BillingService.h"
#include "../services/DoctorService.h"
#include "../services/PatientService.h"
#include "../utils/Logger.h"

#include <memory>

namespace hms::app {

class HospitalContext final {
public:
    HospitalContext();

    services::PatientService& patientService() noexcept { return patientService_; }
    services::DoctorService& doctorService() noexcept { return doctorService_; }
    services::AppointmentService& appointmentService() noexcept { return appointmentService_; }
    services::BillingService& billingService() noexcept { return billingService_; }
    services::AuthService& authService() noexcept { return authService_; }

private:
    std::shared_ptr<utils::Logger> logger_;

    std::shared_ptr<infrastructure::repositories::InMemoryRepository<core::Patient>> patientRepository_;
    std::shared_ptr<infrastructure::repositories::InMemoryRepository<core::Doctor>> doctorRepository_;
    std::shared_ptr<infrastructure::repositories::InMemoryRepository<core::Appointment>> appointmentRepository_;
    std::shared_ptr<infrastructure::repositories::InMemoryRepository<core::Billing>> billingRepository_;
    std::shared_ptr<infrastructure::repositories::InMemoryRepository<core::User>> userRepository_;

    services::PatientService patientService_;
    services::DoctorService doctorService_;
    services::AppointmentService appointmentService_;
    services::BillingService billingService_;
    services::AuthService authService_;
};

}  // namespace hms::app
