#pragma once

#include "IRepository.h"
#include "../Appointment.h"

#include <optional>
#include <string>

namespace hms::core::interfaces {

class IAppointmentRepository : public IRepository<core::Appointment> {
public:
    ~IAppointmentRepository() override = default;

    [[nodiscard]] virtual bool doctorHasAppointmentAt(int doctorId,
                                                      const std::string& dateTime,
                                                      std::optional<int> excludeAppointmentId = std::nullopt) const = 0;
    [[nodiscard]] virtual bool patientHasAppointmentAt(int patientId,
                                                       const std::string& dateTime,
                                                       std::optional<int> excludeAppointmentId = std::nullopt) const = 0;

    [[nodiscard]] virtual std::vector<core::Appointment> getByPatientId(int patientId) const = 0;
    [[nodiscard]] virtual std::vector<core::Appointment> getByDoctorId(int doctorId) const = 0;
};

}  // namespace hms::core::interfaces
