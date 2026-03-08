#pragma once

#include "../core/Appointment.h"
#include "../core/Billing.h"
#include "../core/Interfaces/IRepository.h"
#include "../utils/Logger.h"
#include "../utils/Validator.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hms::services {

class BillingService final {
public:
    static constexpr double kAppointmentCharge = 500.0;

    BillingService(std::shared_ptr<core::interfaces::IRepository<core::Billing>> billingRepository,
                   std::shared_ptr<core::interfaces::IRepository<core::Appointment>> appointmentRepository,
                   std::shared_ptr<utils::Logger> logger)
        : billingRepository_(std::move(billingRepository)),
          appointmentRepository_(std::move(appointmentRepository)),
          logger_(std::move(logger)) {
        if (!billingRepository_ || !appointmentRepository_ || !logger_) {
            throw std::invalid_argument("BillingService dependencies cannot be null.");
        }
    }

    void generateBill(int billId, int appointmentId, double amount) const {
        (void)amount;  // Billing amount is fixed per appointment.
        utils::Validator::requirePositiveId(billId, "bill id");
        utils::Validator::requirePositiveId(appointmentId, "appointment id");

        if (!appointmentRepository_->getById(appointmentId).has_value()) {
            throw std::runtime_error("Appointment not found for bill generation.");
        }

        billingRepository_->add(core::Billing{billId, appointmentId, kAppointmentCharge, false});
        logger_->info("Bill generated. ID: " + std::to_string(billId));
    }

    void updateBill(int billId, int appointmentId, double amount, bool isPaid) const {
        (void)amount;  // Billing amount is fixed per appointment.
        utils::Validator::requirePositiveId(billId, "bill id");
        utils::Validator::requirePositiveId(appointmentId, "appointment id");

        if (!appointmentRepository_->getById(appointmentId).has_value()) {
            throw std::runtime_error("Appointment not found for bill update.");
        }

        billingRepository_->update(core::Billing{billId, appointmentId, kAppointmentCharge, isPaid});
        logger_->info("Bill updated. ID: " + std::to_string(billId));
    }

    [[nodiscard]] static constexpr double appointmentCharge() noexcept { return kAppointmentCharge; }

    void deleteBill(int billId) const {
        utils::Validator::requirePositiveId(billId, "bill id");
        billingRepository_->remove(billId);
        logger_->info("Bill deleted. ID: " + std::to_string(billId));
    }

    [[nodiscard]] std::optional<core::Billing> getBillById(int billId) const {
        utils::Validator::requirePositiveId(billId, "bill id");
        return billingRepository_->getById(billId);
    }

    [[nodiscard]] std::vector<core::Billing> getAllBills() const { return billingRepository_->getAll(); }

private:
    std::shared_ptr<core::interfaces::IRepository<core::Billing>> billingRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Appointment>> appointmentRepository_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
