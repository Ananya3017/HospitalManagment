#pragma once

#include <string>
#include <utility>

namespace hms::core {

enum class AppointmentStatus {
    Scheduled,
    Completed,
    Cancelled
};

class Appointment final {
public:
    Appointment(int id, int patientId, int doctorId, std::string dateTime, AppointmentStatus status)
        : id_(id),
          patientId_(patientId),
          doctorId_(doctorId),
          dateTime_(std::move(dateTime)),
          status_(status) {}

    [[nodiscard]] int getId() const noexcept { return id_; }
    [[nodiscard]] int getPatientId() const noexcept { return patientId_; }
    [[nodiscard]] int getDoctorId() const noexcept { return doctorId_; }
    [[nodiscard]] const std::string& getDateTime() const noexcept { return dateTime_; }
    [[nodiscard]] AppointmentStatus getStatus() const noexcept { return status_; }
    void setStatus(AppointmentStatus status) noexcept { status_ = status; }

private:
    int id_;
    int patientId_;
    int doctorId_;
    std::string dateTime_;
    AppointmentStatus status_;
};

}  // namespace hms::core
