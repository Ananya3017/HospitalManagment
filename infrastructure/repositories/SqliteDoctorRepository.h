#pragma once

#include "../../core/Doctor.h"
#include "../../core/Interfaces/IRepository.h"
#include "../../database/DatabaseManager.h"

#include <optional>
#include <vector>

namespace hms::infrastructure::repositories {

class SqliteDoctorRepository final : public core::interfaces::IRepository<core::Doctor> {
public:
    explicit SqliteDoctorRepository(database::DatabaseManager& databaseManager);

    void add(const core::Doctor& entity) override;
    void update(const core::Doctor& entity) override;
    void remove(int id) override;
    [[nodiscard]] std::optional<core::Doctor> getById(int id) const override;
    [[nodiscard]] std::vector<core::Doctor> getAll() const override;
    [[nodiscard]] int getNextId() const override;

private:
    database::DatabaseManager& databaseManager_;
};

}  // namespace hms::infrastructure::repositories
