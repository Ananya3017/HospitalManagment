#pragma once

#include "IRepository.h"
#include "../Billing.h"

#include <optional>

namespace hms::core::interfaces {

class IBillingRepository : public IRepository<core::Billing> {
public:
    ~IBillingRepository() override = default;

    [[nodiscard]] virtual bool existsForAppointment(int appointmentId,
                                                    std::optional<int> excludeBillId = std::nullopt) const = 0;

    [[nodiscard]] virtual std::vector<core::Billing> getByPatientId(int patientId) const = 0;
};

}  // namespace hms::core::interfaces
