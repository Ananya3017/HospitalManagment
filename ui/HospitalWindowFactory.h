#pragma once

#include "../services/AppointmentService.h"
#include "../services/AuthService.h"
#include "../services/BillingService.h"
#include "../services/DoctorService.h"
#include "../services/PatientService.h"

#include <memory>

class QMainWindow;
class QWidget;

namespace hms::ui {

std::unique_ptr<QMainWindow> createHospitalWindow(
    services::PatientService& patientService,
    services::DoctorService& doctorService,
    services::AppointmentService& appointmentService,
    services::BillingService& billingService,
    services::AuthService& authService,
    QWidget* parent = nullptr);

}  // namespace hms::ui
