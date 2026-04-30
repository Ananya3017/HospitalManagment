#pragma once

#include "../../core/Interfaces/IBillingRepository.h"
#include "../../database/DatabaseManager.h"

#include <optional>
#include <vector>

namespace hms::infrastructure::repositories {

class SqliteBillingRepository final : public core::interfaces::IBillingRepository {
public:
    explicit SqliteBillingRepository(database::DatabaseManager& databaseManager);

    void add(const core::Billing& entity) override;
    void update(const core::Billing& entity) override;
    void remove(int id) override;
    [[nodiscard]] int getNextId() const override;
    [[nodiscard]] std::optional<core::Billing> getById(int id) const override;
    [[nodiscard]] std::vector<core::Billing> getAll() const override;
    [[nodiscard]] bool existsForAppointment(int appointmentId,
                                            std::optional<int> excludeBillId = std::nullopt) const override;
    [[nodiscard]] std::vector<core::Billing> getByPatientId(int patientId) const override;

private:
    database::DatabaseManager& databaseManager_;
    core::Billing readBilling(database::Statement& statement) const;
};

}  // namespace hms::infrastructure::repositories
