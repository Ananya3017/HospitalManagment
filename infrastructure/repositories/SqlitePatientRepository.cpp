#include "SqlitePatientRepository.h"

#include <stdexcept>
#include <string>

namespace hms::infrastructure::repositories {

namespace {

core::Patient readPatient(const database::Statement& statement) {
    return core::Patient{
        statement.columnInt(0),
        statement.columnText(1),
        statement.columnInt(2),
        statement.columnText(3)
    };
}

}  // namespace

SqlitePatientRepository::SqlitePatientRepository(database::DatabaseManager& databaseManager)
    : databaseManager_(databaseManager) {}

void SqlitePatientRepository::add(const core::Patient& entity) {
    auto statement = databaseManager_.prepare(
        "INSERT INTO patients (id, name, age, contact) VALUES (?, ?, ?, ?);");
    statement.bindInt(1, entity.getId());
    statement.bindText(2, entity.getName());
    statement.bindInt(3, entity.getAge());
    statement.bindText(4, entity.getContactNumber());
    statement.stepDone();
}

void SqlitePatientRepository::update(const core::Patient& entity) {
    auto statement = databaseManager_.prepare(
        "UPDATE patients SET name = ?, age = ?, contact = ? WHERE id = ?;");
    statement.bindText(1, entity.getName());
    statement.bindInt(2, entity.getAge());
    statement.bindText(3, entity.getContactNumber());
    statement.bindInt(4, entity.getId());
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Patient not found for update.");
    }
}

void SqlitePatientRepository::remove(int id) {
    auto statement = databaseManager_.prepare("DELETE FROM patients WHERE id = ?;");
    statement.bindInt(1, id);
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Patient not found for deletion.");
    }
}

std::optional<core::Patient> SqlitePatientRepository::getById(int id) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, name, age, contact FROM patients WHERE id = ?;");
    statement.bindInt(1, id);
    if (!statement.stepRow()) {
        return std::nullopt;
    }
    return readPatient(statement);
}

std::vector<core::Patient> SqlitePatientRepository::getAll() const {
    auto statement = databaseManager_.prepare(
        "SELECT id, name, age, contact FROM patients ORDER BY id ASC;");
    std::vector<core::Patient> patients;
    while (statement.stepRow()) {
        patients.push_back(readPatient(statement));
    }
    return patients;
}

int SqlitePatientRepository::getNextId() const {
    auto statement = databaseManager_.prepare("SELECT MAX(id) FROM patients;");
    return statement.stepRow() ? statement.columnInt(0) + 1 : 1;
}

}  // namespace hms::infrastructure::repositories
