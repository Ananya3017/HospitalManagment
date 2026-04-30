#pragma once

#include "../../core/Interfaces/IRepository.h"
#include "../../core/Patient.h"
#include "../../database/DatabaseManager.h"

#include <optional>
#include <vector>

namespace hms::infrastructure::repositories {

class SqlitePatientRepository final : public core::interfaces::IRepository<core::Patient> {
public:
    explicit SqlitePatientRepository(database::DatabaseManager& databaseManager);

    void add(const core::Patient& entity) override;
    void update(const core::Patient& entity) override;
    void remove(int id) override;
    [[nodiscard]] std::optional<core::Patient> getById(int id) const override;
    [[nodiscard]] std::vector<core::Patient> getAll() const override;
    [[nodiscard]] int getNextId() const override;

private:
    database::DatabaseManager& databaseManager_;
};

}  // namespace hms::infrastructure::repositories
