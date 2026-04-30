#pragma once

#include "../core/Appointment.h"
#include "../core/Billing.h"
#include "../core/Interfaces/IBillingRepository.h"
#include "../database/DatabaseManager.h"
#include "../database/Transaction.h"
#include "../core/Doctor.h"
#include "../core/Interfaces/IRepository.h"
#include "../core/User.h"
#include "../utils/PermissionManager.h"
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

    BillingService(std::shared_ptr<core::interfaces::IBillingRepository> billingRepository,
                    std::shared_ptr<core::interfaces::IRepository<core::Appointment>> appointmentRepository,
                   database::DatabaseManager& databaseManager,
                   std::shared_ptr<utils::Logger> logger)
        : billingRepository_(std::move(billingRepository)),
          appointmentRepository_(std::move(appointmentRepository)),
          databaseManager_(databaseManager),
          logger_(std::move(logger)) {
        if (!billingRepository_ || !appointmentRepository_ || !logger_) {
            throw std::invalid_argument("BillingService dependencies cannot be null.");
        }
    }

    void generateBill(int appointmentId, double amount, const core::User& currentUser) const {
        if (!utils::PermissionManager::canManageBilling(currentUser)) {
            throw std::runtime_error("Insufficient permissions to generate bills.");
        }
        
        utils::Validator::requirePositiveId(appointmentId, "appointment id");
        utils::Validator::requirePositiveAmount(amount);

        database::Transaction transaction(databaseManager_);
        ensureAppointmentExists(appointmentId, "generation");
        
        int billId = billingRepository_->getNextId();
        ensureNoDuplicateBill(appointmentId, std::nullopt);

        billingRepository_->add(core::Billing{billId, appointmentId, amount, false});
        transaction.commit();
        logger_->info("Bill generated. ID: " + std::to_string(billId));
    }

    void updateBill(int billId, int appointmentId, double amount, bool isPaid, const core::User& currentUser) const {
        if (!utils::PermissionManager::canManageBilling(currentUser)) {
            throw std::runtime_error("Insufficient permissions to update bills.");
        }

        hms::utils::Validator::requirePositiveId(billId, "bill id");
        hms::utils::Validator::requirePositiveId(appointmentId, "appointment id");
        hms::utils::Validator::requirePositiveAmount(amount);

        database::Transaction transaction(databaseManager_);
        ensureAppointmentExists(appointmentId, "update");
        ensureNoDuplicateBill(appointmentId, billId);

        billingRepository_->update(core::Billing{billId, appointmentId, amount, isPaid});
        transaction.commit();
        logger_->info("Bill updated. ID: " + std::to_string(billId));
    }

    [[nodiscard]] static constexpr double appointmentCharge() noexcept { return kAppointmentCharge; }

    void deleteBill(int billId, const core::User& currentUser) const {
        if (!utils::PermissionManager::canDeleteBills(currentUser)) {
            throw std::runtime_error("Insufficient permissions to delete bills.");
        }
        hms::utils::Validator::requirePositiveId(billId, "bill id");
        billingRepository_->remove(billId);
        logger_->info("Bill deleted. ID: " + std::to_string(billId));
    }

    [[nodiscard]] std::optional<core::Billing> getBillById(int billId) const {
        hms::utils::Validator::requirePositiveId(billId, "bill id");
        return billingRepository_->getById(billId);
    }

    [[nodiscard]] std::vector<core::Billing> getAllBills(const core::User& currentUser) const {
        if (currentUser.getRole() == core::UserRole::Patient) {
            return billingRepository_->getByPatientId(currentUser.getId());
        }
        if (currentUser.getRole() == core::UserRole::Doctor) {
            std::vector<core::Billing> doctorBills;
            const auto allBills = billingRepository_->getAll();
            for (const auto& bill : allBills) {
                auto app = appointmentRepository_->getById(bill.getAppointmentId());
                if (app && app->getDoctorId() == currentUser.getId()) {
                    doctorBills.push_back(bill);
                }
            }
            return doctorBills;
        }
        return billingRepository_->getAll();
    }

private:
    void ensureAppointmentExists(int appointmentId, const std::string& action) const {
        if (!appointmentRepository_->getById(appointmentId).has_value()) {
            throw std::runtime_error("Appointment not found for bill " + action + ".");
        }
    }

    void ensureNoDuplicateBill(int appointmentId, std::optional<int> excludeBillId) const {
        if (billingRepository_->existsForAppointment(appointmentId, excludeBillId)) {
            throw std::runtime_error("A bill already exists for the selected appointment.");
        }
    }

    std::shared_ptr<core::interfaces::IBillingRepository> billingRepository_;
    std::shared_ptr<core::interfaces::IRepository<core::Appointment>> appointmentRepository_;
    database::DatabaseManager& databaseManager_;
    std::shared_ptr<utils::Logger> logger_;
};

}  // namespace hms::services
