#pragma once

#include "../persistence/IDataStore.h"
#include "../services/AppointmentService.h"
#include "../services/BillingService.h"
#include "../services/DoctorService.h"
#include "../services/PatientService.h"

#include <string>

namespace hms::app {

class HospitalDataCoordinator final {
public:
    HospitalDataCoordinator(services::PatientService& patientService,
                            services::DoctorService& doctorService,
                            services::AppointmentService& appointmentService,
                            services::BillingService& billingService,
                            persistence::IDataStore& dataStore);

    bool load(std::string& error);
    bool save(std::string& error) const;

private:
    services::PatientService& patientService_;
    services::DoctorService& doctorService_;
    services::AppointmentService& appointmentService_;
    services::BillingService& billingService_;
    persistence::IDataStore& dataStore_;
};

}  // namespace hms::app
