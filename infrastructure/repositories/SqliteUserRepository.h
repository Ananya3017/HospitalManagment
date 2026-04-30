#pragma once

#include "../../core/Interfaces/IUserRepository.h"
#include "../../database/DatabaseManager.h"

#include <optional>
#include <vector>

namespace hms::infrastructure::repositories {

class SqliteUserRepository final : public core::interfaces::IUserRepository {
public:
    explicit SqliteUserRepository(database::DatabaseManager& databaseManager);

    void add(const core::User& entity) override;
    void update(const core::User& entity) override;
    void remove(int id) override;
    [[nodiscard]] std::optional<core::User> getById(int id) const override;
    [[nodiscard]] std::vector<core::User> getAll() const override;
    [[nodiscard]] int getNextId() const override;
    [[nodiscard]] std::optional<core::User> getByUsername(const std::string& username) const override;

private:
    database::DatabaseManager& databaseManager_;
};

}  // namespace hms::infrastructure::repositories
