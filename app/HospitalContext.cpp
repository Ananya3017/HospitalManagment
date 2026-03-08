#include "HospitalContext.h"

namespace hms::app {

HospitalContext::HospitalContext()
    : logger_(std::make_shared<utils::Logger>()),
      patientRepository_(std::make_shared<infrastructure::repositories::InMemoryRepository<core::Patient>>(
          [](const core::Patient& p) { return p.getId(); })),
      doctorRepository_(std::make_shared<infrastructure::repositories::InMemoryRepository<core::Doctor>>(
          [](const core::Doctor& d) { return d.getId(); })),
      appointmentRepository_(std::make_shared<infrastructure::repositories::InMemoryRepository<core::Appointment>>(
          [](const core::Appointment& a) { return a.getId(); })),
      billingRepository_(std::make_shared<infrastructure::repositories::InMemoryRepository<core::Billing>>(
          [](const core::Billing& b) { return b.getId(); })),
      userRepository_(std::make_shared<infrastructure::repositories::InMemoryRepository<core::User>>(
          [](const core::User& u) { return u.getId(); })),
      patientService_(patientRepository_, logger_),
      doctorService_(doctorRepository_, logger_),
      appointmentService_(appointmentRepository_, patientRepository_, doctorRepository_, logger_),
      billingService_(billingRepository_, appointmentRepository_, logger_),
      authService_(userRepository_, logger_) {
    authService_.registerUser(1, "admin", "admin123", core::UserRole::Admin);
    authService_.registerUser(2, "reception", "rec123", core::UserRole::Receptionist);
    authService_.registerUser(3, "doctor1", "doc123", core::UserRole::Doctor);
}

}  // namespace hms::app
