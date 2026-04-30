#include "SqliteBillingRepository.h"

#include <stdexcept>

namespace hms::infrastructure::repositories {

namespace {

core::Billing readBill(const database::Statement& statement) {
    return core::Billing{
        statement.columnInt(0),
        statement.columnInt(1),
        statement.columnDouble(2),
        statement.columnInt(3) != 0
    };
}

}  // namespace

core::Billing SqliteBillingRepository::readBilling(database::Statement& statement) const {
    return readBill(statement);
}

SqliteBillingRepository::SqliteBillingRepository(database::DatabaseManager& databaseManager)
    : databaseManager_(databaseManager) {}

void SqliteBillingRepository::add(const core::Billing& entity) {
    auto statement = databaseManager_.prepare(
        "INSERT INTO bills (id, appointment_id, amount, is_paid) VALUES (?, ?, ?, ?);");
    statement.bindInt(1, entity.getId());
    statement.bindInt(2, entity.getAppointmentId());
    statement.bindDouble(3, entity.getAmount());
    statement.bindInt(4, entity.isPaid() ? 1 : 0);
    statement.stepDone();
}

void SqliteBillingRepository::update(const core::Billing& entity) {
    auto statement = databaseManager_.prepare(
        "UPDATE bills SET appointment_id = ?, amount = ?, is_paid = ? WHERE id = ?;");
    statement.bindInt(1, entity.getAppointmentId());
    statement.bindDouble(2, entity.getAmount());
    statement.bindInt(3, entity.isPaid() ? 1 : 0);
    statement.bindInt(4, entity.getId());
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Bill not found for update.");
    }
}

void SqliteBillingRepository::remove(int id) {
    auto statement = databaseManager_.prepare("DELETE FROM bills WHERE id = ?;");
    statement.bindInt(1, id);
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("Bill not found for deletion.");
    }
}

int SqliteBillingRepository::getNextId() const {
    auto statement = databaseManager_.prepare("SELECT MAX(id) FROM bills;");
    if (!statement.stepRow() || statement.columnType(0) == database::ColumnType::Null) {
        return 1;
    }
    return statement.columnInt(0) + 1;
}

std::optional<core::Billing> SqliteBillingRepository::getById(int id) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, appointment_id, amount, is_paid FROM bills WHERE id = ?;");
    statement.bindInt(1, id);
    if (!statement.stepRow()) {
        return std::nullopt;
    }
    return readBill(statement);
}

std::vector<core::Billing> SqliteBillingRepository::getByPatientId(int patientId) const {
    auto statement = databaseManager_.prepare(
        "SELECT b.id, b.appointment_id, b.amount, b.is_paid FROM bills b "
        "JOIN appointments a ON b.appointment_id = a.id WHERE a.patient_id = ?;");
    statement.bindInt(1, patientId);
    std::vector<core::Billing> bills;
    while (statement.stepRow()) {
        bills.push_back(readBill(statement));
    }
    return bills;
}

std::vector<core::Billing> SqliteBillingRepository::getAll() const {
    auto statement = databaseManager_.prepare(
        "SELECT id, appointment_id, amount, is_paid FROM bills ORDER BY id ASC;");
    std::vector<core::Billing> bills;
    while (statement.stepRow()) {
        bills.push_back(readBill(statement));
    }
    return bills;
}

bool SqliteBillingRepository::existsForAppointment(int appointmentId, std::optional<int> excludeBillId) const {
    const std::string sql = excludeBillId.has_value()
        ? "SELECT 1 FROM bills WHERE appointment_id = ? AND id <> ? LIMIT 1;"
        : "SELECT 1 FROM bills WHERE appointment_id = ? LIMIT 1;";
    auto statement = databaseManager_.prepare(sql);
    statement.bindInt(1, appointmentId);
    if (excludeBillId.has_value()) {
        statement.bindInt(2, *excludeBillId);
    }
    return statement.stepRow();
}

}  // namespace hms::infrastructure::repositories
