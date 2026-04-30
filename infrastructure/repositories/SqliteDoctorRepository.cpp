#include "SqliteDoctorRepository.h"

#include <stdexcept>

namespace hms::infrastructure::repositories {

namespace {

core::Doctor readDoctor(const database::Statement& statement) {
    return core::Doctor{
        statement.columnInt(0),
        statement.columnText(1),
        statement.columnText(2)
    };
}

}  // namespace

SqliteDoctorRepository::SqliteDoctorRepository(database::DatabaseManager& databaseManager)
    : databaseManager_(databaseManager) {}

void SqliteDoctorRepository::add(const core::Doctor& entity) {
    auto statement = databaseManager_.prepare(
        "INSERT INTO doctors (id, name, specialization) VALUES (?, ?, ?);");
    statement.bindInt(1, entity.getId());
    statement.bindText(2, entity.getName());
    statement.bindText(3, entity.getSpecialization());
    statement.stepDone();
}

void SqliteDoctorRepository::update(const core::Doctor& entity) {
    auto statement = databaseManager_.prepare(
        "UPDATE doctors SET name = ?, specialization = ? WHERE id = ?;");
    statement.bindText(1, entity.getName());
    statement.bindText(2, entity.getSpecialization());
    statement.bindInt(3, entity.getId());
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Doctor not found for update.");
    }
}

void SqliteDoctorRepository::remove(int id) {
    auto statement = databaseManager_.prepare("DELETE FROM doctors WHERE id = ?;");
    statement.bindInt(1, id);
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Doctor not found for deletion.");
    }
}

std::optional<core::Doctor> SqliteDoctorRepository::getById(int id) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, name, specialization FROM doctors WHERE id = ?;");
    statement.bindInt(1, id);
    if (!statement.stepRow()) {
        return std::nullopt;
    }
    return readDoctor(statement);
}

std::vector<core::Doctor> SqliteDoctorRepository::getAll() const {
    auto statement = databaseManager_.prepare(
        "SELECT id, name, specialization FROM doctors ORDER BY id ASC;");
    std::vector<core::Doctor> doctors;
    while (statement.stepRow()) {
        doctors.push_back(readDoctor(statement));
    }
    return doctors;
}

int SqliteDoctorRepository::getNextId() const {
    auto statement = databaseManager_.prepare("SELECT MAX(id) FROM doctors;");
    return statement.stepRow() ? statement.columnInt(0) + 1 : 1;
}

}  // namespace hms::infrastructure::repositories
