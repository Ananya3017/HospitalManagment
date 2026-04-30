#include "SqliteUserRepository.h"

#include <stdexcept>

namespace hms::infrastructure::repositories {

namespace {

core::User readUser(const database::Statement& statement) {
    return core::User{
        statement.columnInt(0),
        statement.columnText(1),
        statement.columnText(2),
        core::userRoleFromString(statement.columnText(3))
    };
}

}  // namespace

SqliteUserRepository::SqliteUserRepository(database::DatabaseManager& databaseManager)
    : databaseManager_(databaseManager) {}

void SqliteUserRepository::add(const core::User& entity) {
    auto statement = databaseManager_.prepare(
        "INSERT INTO users (id, username, password_hash, role) VALUES (?, ?, ?, ?);");
    statement.bindInt(1, entity.getId());
    statement.bindText(2, entity.getUsername());
    statement.bindText(3, entity.getPasswordHash());
    statement.bindText(4, core::toString(entity.getRole()));
    statement.stepDone();
}

void SqliteUserRepository::update(const core::User& entity) {
    auto statement = databaseManager_.prepare(
        "UPDATE users SET username = ?, password_hash = ?, role = ? WHERE id = ?;");
    statement.bindText(1, entity.getUsername());
    statement.bindText(2, entity.getPasswordHash());
    statement.bindText(3, core::toString(entity.getRole()));
    statement.bindInt(4, entity.getId());
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("User not found for update.");
    }
}

void SqliteUserRepository::remove(int id) {
    auto statement = databaseManager_.prepare("DELETE FROM users WHERE id = ?;");
    statement.bindInt(1, id);
    statement.stepDone();
    if (databaseManager_.changes() == 0) {
        throw std::runtime_error("User not found for deletion.");
    }
}

std::optional<core::User> SqliteUserRepository::getById(int id) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, username, password_hash, role FROM users WHERE id = ?;");
    statement.bindInt(1, id);
    if (!statement.stepRow()) {
        return std::nullopt;
    }
    return readUser(statement);
}

std::vector<core::User> SqliteUserRepository::getAll() const {
    auto statement = databaseManager_.prepare(
        "SELECT id, username, password_hash, role FROM users ORDER BY id ASC;");
    std::vector<core::User> users;
    while (statement.stepRow()) {
        users.push_back(readUser(statement));
    }
    return users;
}

std::optional<core::User> SqliteUserRepository::getByUsername(const std::string& username) const {
    auto statement = databaseManager_.prepare(
        "SELECT id, username, password_hash, role FROM users WHERE username = ?;");
    statement.bindText(1, username);
    if (!statement.stepRow()) {
        return std::nullopt;
    }
    return readUser(statement);
}

int SqliteUserRepository::getNextId() const {
    auto statement = databaseManager_.prepare("SELECT MAX(id) FROM users;");
    return statement.stepRow() ? statement.columnInt(0) + 1 : 1;
}

}  // namespace hms::infrastructure::repositories
