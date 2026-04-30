#include "HospitalWindowFactory.h"
#include "HospitalWindowView.h"

#include <memory>

namespace hms::ui {

std::unique_ptr<QMainWindow> createHospitalWindow(services::PatientService& patientService,
                                                  services::DoctorService& doctorService,
                                                  services::AppointmentService& appointmentService,
                                                  services::BillingService& billingService,
                                                  services::AuthService& authService,
                                                  QWidget* parent) {
    return std::make_unique<HospitalWindow>(
        patientService, doctorService, appointmentService, billingService, authService, parent);
}

}  // namespace hms::ui
