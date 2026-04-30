#pragma once

#include <filesystem>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace hms::database {

enum class ColumnType {
    Integer,
    Float,
    Text,
    Blob,
    Null
};

class Statement final {
public:
    Statement(sqlite3* database, const std::string& sql);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    void bindInt(int index, int value);
    void bindDouble(int index, double value);
    void bindText(int index, const std::string& value);
    void bindNull(int index);

    [[nodiscard]] bool stepRow();
    void stepDone();

    [[nodiscard]] int columnInt(int index) const;
    [[nodiscard]] double columnDouble(int index) const;
    [[nodiscard]] std::string columnText(int index) const;
    [[nodiscard]] ColumnType columnType(int index) const;

private:
    sqlite3* database_ = nullptr;
    sqlite3_stmt* statement_ = nullptr;
};

class DatabaseManager final {
public:
    explicit DatabaseManager(std::filesystem::path databasePath);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void open();
    void close() noexcept;

    void initialize();
    void execute(const std::string& sql) const;

    [[nodiscard]] Statement prepare(const std::string& sql) const;
    [[nodiscard]] int changes() const;
    [[nodiscard]] sqlite3* handle() const noexcept { return database_; }
    [[nodiscard]] const std::filesystem::path& databasePath() const noexcept { return databasePath_; }

private:
    std::filesystem::path databasePath_;
    sqlite3* database_ = nullptr;
};

}  // namespace hms::database
