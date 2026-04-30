#include "HospitalContext.h"

#include "../infrastructure/repositories/SqliteAppointmentRepository.h"
#include "../infrastructure/repositories/SqliteBillingRepository.h"
#include "../infrastructure/repositories/SqliteDoctorRepository.h"
#include "../infrastructure/repositories/SqlitePatientRepository.h"
#include "../infrastructure/repositories/SqliteUserRepository.h"


namespace hms::app {

HospitalContext::HospitalContext(std::filesystem::path databasePath)
    : logger_(std::make_shared<utils::Logger>()),
      databaseManager_(std::make_unique<database::DatabaseManager>(std::move(databasePath))),
      patientRepository_(std::make_shared<infrastructure::repositories::SqlitePatientRepository>(*databaseManager_)),
      doctorRepository_(std::make_shared<infrastructure::repositories::SqliteDoctorRepository>(*databaseManager_)),
      appointmentRepository_(std::make_shared<infrastructure::repositories::SqliteAppointmentRepository>(*databaseManager_)),
      billingRepository_(std::make_shared<infrastructure::repositories::SqliteBillingRepository>(*databaseManager_)),
      userRepository_(std::make_shared<infrastructure::repositories::SqliteUserRepository>(*databaseManager_)),
      patientService_(patientRepository_, logger_),
      doctorService_(doctorRepository_, logger_),
      appointmentService_(appointmentRepository_, patientRepository_, doctorRepository_, *databaseManager_, logger_),
      billingService_(billingRepository_, appointmentRepository_, *databaseManager_, logger_),
      authService_(userRepository_, logger_) {
    databaseManager_->initialize();
    seedDefaultUsers();
}

void HospitalContext::seedDefaultUsers() {
    authService_.seedDemoUsers();

    const auto demoDoctor = userRepository_->getById(1002);
    if (demoDoctor.has_value() &&
        demoDoctor->getRole() == core::UserRole::Doctor &&
        !doctorRepository_->getById(demoDoctor->getId()).has_value()) {
        doctorRepository_->add(core::Doctor{
            demoDoctor->getId(),
            "Demo Doctor",
            "General Medicine"
        });
        logger_->info("Seeded default doctor profile for demo login.");
    }

    const auto demoPatient = userRepository_->getById(1003);
    if (demoPatient.has_value() &&
        demoPatient->getRole() == core::UserRole::Patient &&
        !patientRepository_->getById(demoPatient->getId()).has_value()) {
        patientRepository_->add(core::Patient{
            demoPatient->getId(),
            "Demo Patient",
            30,
            "9000000000"
        });
        logger_->info("Seeded default patient profile for demo login.");
    }
}

}  // namespace hms::app
