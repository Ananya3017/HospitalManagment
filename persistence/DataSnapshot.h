#pragma once

#include "../core/Appointment.h"

#include <string>
#include <vector>

namespace hms::persistence {

struct PatientRecord {
    int id = 0;
    std::string name;
    int age = 0;
    std::string contact;
};

struct DoctorRecord {
    int id = 0;
    std::string name;
    std::string specialization;
};

struct AppointmentRecord {
    int id = 0;
    int patientId = 0;
    int doctorId = 0;
    std::string dateTime;
    core::AppointmentStatus status = core::AppointmentStatus::Scheduled;
};

struct BillRecord {
    int id = 0;
    int appointmentId = 0;
    double amount = 0.0;
    bool isPaid = false;
};

struct DataSnapshot {
    std::vector<PatientRecord> patients;
    std::vector<DoctorRecord> doctors;
    std::vector<AppointmentRecord> appointments;
    std::vector<BillRecord> bills;
};

}  // namespace hms::persistence
