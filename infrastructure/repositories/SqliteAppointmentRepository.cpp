#include "SqliteAppointmentRepository.h"

#include <stdexcept>

namespace hms::infrastructure::repositories {

namespace {

core::Appointment readAppointment(const database::Statement& statement) {
    return core::Appointment{
        statement.columnInt(0),
        statement.columnInt(1),
        statement.columnInt(2),
        statement.columnText(3),
        core::appointmentStatusFromString(statement.columnText(4))
    };
}

bool hasMatchingAppointment(database::DatabaseManager& databaseManager,
                            const std::string& baseSql,
                            int ownerId,
                            const std::string& dateTime,
                            std::optional<int> excludeAppointmentId) {
    auto statement = databaseManager.prepare(baseSql);
    statement.bindInt(1, ownerId);
    statement.bindText(2, dateTime);
    if (excludeAppointmentId.has_value()) {
        statement.bindInt(3, *excludeAppointmentId);
    }
    return statement.stepRow();
}

}  // namespace

SqliteAppointmentRepository::SqliteAppointmentRepository(database::DatabaseManager& databaseManager)
    : databaseManager_(databaseManager) {}

void SqliteAppointmentRepository::add(const core::Appointment& entity) {
    auto statement = databaseManager_.prepare(
        "INSERT INTO appointments (id, patient_id, doctor_id, time, status) VALUES (?, ?, ?, ?, ?);");
    statement.bindInt(1, entity.getId());
    statement.bindInt(2, entity.getPatientId());
    statement.bindInt(3, entity.getDoctorId());
    statement.bindText(4, entity.getDateTime());
    statement.bindText(5, core::toString(entity.getStatus()));
    statement.stepDone();
}

void SqliteAppointmentRepository::update(const core::Appointment& entity) {
    auto statement = databaseManager_.prepare(
        "UPDATE appointments SET patient_id = ?, doctor_id = ?, time = ?, status = ? WHERE id = ?;");
    statement.bindInt(1, entity.getPatientId());
    statement.bindInt(2, entity.getDoctorId());
    statement.bindText(3, entity.getDateTime());
    statement.bindText(4, core::toString(entity.getStatus()));
    statement.bindInt(5, entity.getId());
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Appointment not found for update.");
    }
}

void SqliteAppointmentRepository::remove(int id) {
    auto statement = databaseManager_.prepare("DELETE FROM appointments WHERE id = ?;");
    statement.bindInt(1, id);
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Appointment not found for deletion.");
    }
}

int SqliteAppointmentRepository::getNextId() const {
    auto statement = databaseManager_.prepare("SELECT MAX(id) FROM appointments;");
    if (!statement.stepRow() || statement.columnType(0) == database::ColumnType::Null) {
        return 1;
    }
    return statement.columnInt(0) + 1;
}

std::optional<core::Appointment> SqliteAppointmentRepository::getById(int id) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, patient_id, doctor_id, time, status FROM appointments WHERE id = ?;");
    statement.bindInt(1, id);
    if (!statement.stepRow()) {
        return std::nullopt;
    }
    return readAppointment(statement);
}

std::vector<core::Appointment> SqliteAppointmentRepository::getAll() const {
    auto statement = databaseManager_.prepare(
        "SELECT id, patient_id, doctor_id, time, status FROM appointments ORDER BY id ASC;");
    std::vector<core::Appointment> appointments;
    while (statement.stepRow()) {
        appointments.push_back(readAppointment(statement));
    }
    return appointments;
}

bool SqliteAppointmentRepository::doctorHasAppointmentAt(int doctorId,
                                                         const std::string& dateTime,
                                                         std::optional<int> excludeAppointmentId) const {
    const std::string sql = excludeAppointmentId.has_value()
        ? "SELECT 1 FROM appointments WHERE doctor_id = ? AND time = ? AND id <> ? LIMIT 1;"
        : "SELECT 1 FROM appointments WHERE doctor_id = ? AND time = ? LIMIT 1;";
    return hasMatchingAppointment(databaseManager_, sql, doctorId, dateTime, excludeAppointmentId);
}

bool SqliteAppointmentRepository::patientHasAppointmentAt(int patientId,
                                                          const std::string& dateTime,
                                                          std::optional<int> excludeAppointmentId) const {
    const std::string sql = excludeAppointmentId.has_value()
        ? "SELECT 1 FROM appointments WHERE patient_id = ? AND time = ? AND id <> ? LIMIT 1;"
        : "SELECT 1 FROM appointments WHERE patient_id = ? AND time = ? LIMIT 1;";
    return hasMatchingAppointment(databaseManager_, sql, patientId, dateTime, excludeAppointmentId);
}

std::vector<core::Appointment> SqliteAppointmentRepository::getByPatientId(int patientId) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, patient_id, doctor_id, time, status FROM appointments WHERE patient_id = ? ORDER BY time DESC;");
    statement.bindInt(1, patientId);
    std::vector<core::Appointment> appointments;
    while (statement.stepRow()) {
        appointments.push_back(readAppointment(statement));
    }
    return appointments;
}

std::vector<core::Appointment> SqliteAppointmentRepository::getByDoctorId(int doctorId) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, patient_id, doctor_id, time, status FROM appointments WHERE doctor_id = ? ORDER BY time DESC;");
    statement.bindInt(1, doctorId);
    std::vector<core::Appointment> appointments;
    while (statement.stepRow()) {
        appointments.push_back(readAppointment(statement));
    }
    return appointments;
}

}  // namespace hms::infrastructure::repositories
