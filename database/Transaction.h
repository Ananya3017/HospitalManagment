#pragma once

namespace hms::database {

class DatabaseManager;

class Transaction final {
public:
    explicit Transaction(DatabaseManager& databaseManager);
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit();
    void rollback() noexcept;

private:
    DatabaseManager& databaseManager_;
    bool active_ = false;
};

}  // namespace hms::database
