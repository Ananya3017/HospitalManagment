#pragma once

#include "../../core/Interfaces/IAppointmentRepository.h"
#include "../../database/DatabaseManager.h"

#include <optional>
#include <vector>
#include <string>

namespace hms::infrastructure::repositories {

class SqliteAppointmentRepository final : public core::interfaces::IAppointmentRepository {
public:
    explicit SqliteAppointmentRepository(database::DatabaseManager& databaseManager);

    void add(const core::Appointment& entity) override;
    void update(const core::Appointment& entity) override;
    void remove(int id) override;
    [[nodiscard]] int getNextId() const override;
    [[nodiscard]] std::optional<core::Appointment> getById(int id) const override;
    [[nodiscard]] std::vector<core::Appointment> getAll() const override;
    [[nodiscard]] bool doctorHasAppointmentAt(int doctorId,
                                              const std::string& dateTime,
                                              std::optional<int> excludeAppointmentId = std::nullopt) const override;
    [[nodiscard]] bool patientHasAppointmentAt(int patientId,
                                               const std::string& dateTime,
                                               std::optional<int> excludeAppointmentId = std::nullopt) const override;
    [[nodiscard]] std::vector<core::Appointment> getByPatientId(int patientId) const override;
    [[nodiscard]] std::vector<core::Appointment> getByDoctorId(int doctorId) const override;

private:
    database::DatabaseManager& databaseManager_;
};

}  // namespace hms::infrastructure::repositories
