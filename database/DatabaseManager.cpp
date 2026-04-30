#include "DatabaseManager.h"

#include <sqlite3.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace hms::database {

namespace {

std::runtime_error sqliteError(sqlite3* database, const std::string& message) {
    const char* sqliteMessage = database ? sqlite3_errmsg(database) : "unknown sqlite error";
    return std::runtime_error(message + ": " + sqliteMessage);
}

constexpr const char* kSchemaStatements[] = {
    "PRAGMA foreign_keys = ON;",
    "CREATE TABLE IF NOT EXISTS users ("
    "id INTEGER PRIMARY KEY,"
    "username TEXT NOT NULL UNIQUE,"
    "password_hash TEXT NOT NULL,"
    "role TEXT NOT NULL CHECK(role IN ('Admin', 'Doctor', 'Receptionist', 'Patient'))"
    ");",
    "CREATE TABLE IF NOT EXISTS patients ("
    "id INTEGER PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "age INTEGER NOT NULL CHECK(age BETWEEN 1 AND 130),"
    "contact TEXT NOT NULL"
    ");",
    "CREATE TABLE IF NOT EXISTS doctors ("
    "id INTEGER PRIMARY KEY,"
    "name TEXT NOT NULL,"
    "specialization TEXT NOT NULL"
    ");",
    "CREATE TABLE IF NOT EXISTS appointments ("
    "id INTEGER PRIMARY KEY,"
    "patient_id INTEGER NOT NULL,"
    "doctor_id INTEGER NOT NULL,"
    "time TEXT NOT NULL,"
    "status TEXT NOT NULL CHECK(status IN ('Scheduled', 'Completed', 'Cancelled')),"
    "FOREIGN KEY(patient_id) REFERENCES patients(id) ON DELETE RESTRICT,"
    "FOREIGN KEY(doctor_id) REFERENCES doctors(id) ON DELETE RESTRICT,"
    "UNIQUE(doctor_id, time)"
    ");",
    "CREATE TABLE IF NOT EXISTS bills ("
    "id INTEGER PRIMARY KEY,"
    "appointment_id INTEGER NOT NULL UNIQUE,"
    "amount REAL NOT NULL CHECK(amount > 0),"
    "is_paid INTEGER NOT NULL DEFAULT 0 CHECK(is_paid IN (0, 1)),"
    "FOREIGN KEY(appointment_id) REFERENCES appointments(id) ON DELETE RESTRICT"
    ");"
};

}  // namespace

Statement::Statement(sqlite3* database, const std::string& sql) : database_(database) {
    const int rc = sqlite3_prepare_v2(database_, sql.c_str(), -1, &statement_, nullptr);
    if (rc != SQLITE_OK) {
        throw sqliteError(database_, "Failed to prepare statement");
    }
}

Statement::~Statement() {
    if (statement_ != nullptr) {
        sqlite3_finalize(statement_);
    }
}

Statement::Statement(Statement&& other) noexcept
    : database_(other.database_), statement_(other.statement_) {
    other.database_ = nullptr;
    other.statement_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (statement_ != nullptr) {
            sqlite3_finalize(statement_);
        }
        database_ = other.database_;
        statement_ = other.statement_;
        other.database_ = nullptr;
        other.statement_ = nullptr;
    }
    return *this;
}

void Statement::bindInt(int index, int value) {
    if (sqlite3_bind_int(statement_, index, value) != SQLITE_OK) {
        throw sqliteError(database_, "Failed to bind int parameter");
    }
}

void Statement::bindDouble(int index, double value) {
    if (sqlite3_bind_double(statement_, index, value) != SQLITE_OK) {
        throw sqliteError(database_, "Failed to bind double parameter");
    }
}

void Statement::bindText(int index, const std::string& value) {
    if (sqlite3_bind_text(statement_, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        throw sqliteError(database_, "Failed to bind text parameter");
    }
}

void Statement::bindNull(int index) {
    if (sqlite3_bind_null(statement_, index) != SQLITE_OK) {
        throw sqliteError(database_, "Failed to bind null parameter");
    }
}

bool Statement::stepRow() {
    const int rc = sqlite3_step(statement_);
    if (rc == SQLITE_ROW) {
        return true;
    }
    if (rc == SQLITE_DONE) {
        return false;
    }
    throw sqliteError(database_, "Failed to step statement");
}

void Statement::stepDone() {
    const int rc = sqlite3_step(statement_);
    if (rc != SQLITE_DONE) {
        throw sqliteError(database_, "Failed to execute statement");
    }
}

int Statement::columnInt(int index) const { return sqlite3_column_int(statement_, index); }

double Statement::columnDouble(int index) const { return sqlite3_column_double(statement_, index); }

std::string Statement::columnText(int index) const {
    const unsigned char* text = sqlite3_column_text(statement_, index);
    return text != nullptr ? reinterpret_cast<const char*>(text) : std::string{};
}
ColumnType Statement::columnType(int index) const {
    switch (sqlite3_column_type(statement_, index)) {
        case SQLITE_INTEGER: return ColumnType::Integer;
        case SQLITE_FLOAT:   return ColumnType::Float;
        case SQLITE_TEXT:    return ColumnType::Text;
        case SQLITE_BLOB:    return ColumnType::Blob;
        default:             return ColumnType::Null;
    }
}

DatabaseManager::DatabaseManager(std::filesystem::path databasePath) : databasePath_(std::move(databasePath)) {}

DatabaseManager::~DatabaseManager() { close(); }

void DatabaseManager::open() {
    if (database_ != nullptr) {
        return;
    }

    if (databasePath_.has_parent_path()) {
        std::filesystem::create_directories(databasePath_.parent_path());
    }

    const int rc = sqlite3_open_v2(
        databasePath_.string().c_str(),
        &database_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        nullptr);
    if (rc != SQLITE_OK) {
        std::runtime_error error = sqliteError(database_, "Failed to open database");
        close();
        throw error;
    }
}

void DatabaseManager::close() noexcept {
    if (database_ != nullptr) {
        sqlite3_close(database_);
        database_ = nullptr;
    }
}

void DatabaseManager::initialize() {
    open();

    // Create all tables first (all statements use IF NOT EXISTS, safe on any DB state)
    for (const auto* sql : kSchemaStatements) {
        execute(sql);
    }

    // Migration: ensure the 'Patient' role is accepted by the CHECK constraint.
    // If the constraint is already correct this no-ops; otherwise recreate the table.
    try {
        execute("INSERT INTO users (id, username, password_hash, role) VALUES (-1, '__temp__', '', 'Patient');");
        execute("DELETE FROM users WHERE id = -1;");
    } catch (...) {
        execute("BEGIN TRANSACTION;");
        execute("ALTER TABLE users RENAME TO users_old;");
        execute("CREATE TABLE users ("
                "id INTEGER PRIMARY KEY,"
                "username TEXT NOT NULL UNIQUE,"
                "password_hash TEXT NOT NULL,"
                "role TEXT NOT NULL CHECK(role IN ('Admin', 'Doctor', 'Receptionist', 'Patient'))"
                ");");
        execute("INSERT INTO users SELECT * FROM users_old;");
        execute("DROP TABLE users_old;");
        execute("COMMIT;");
    }
}

void DatabaseManager::execute(const std::string& sql) const {
    char* errorMessage = nullptr;
    const int rc = sqlite3_exec(database_, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        const std::string sqliteMessage = errorMessage != nullptr ? errorMessage : "unknown sqlite error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed to execute SQL: " + sqliteMessage);
    }
}

Statement DatabaseManager::prepare(const std::string& sql) const { return Statement(database_, sql); }

int DatabaseManager::changes() const { return sqlite3_changes(database_); }

}  // namespace hms::database
