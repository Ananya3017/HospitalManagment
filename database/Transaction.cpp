#include "Transaction.h"

#include "DatabaseManager.h"

namespace hms::database {

Transaction::Transaction(DatabaseManager& databaseManager) : databaseManager_(databaseManager), active_(true) {
    databaseManager_.execute("BEGIN IMMEDIATE TRANSACTION;");
}

Transaction::~Transaction() {
    if (active_) {
        try {
            databaseManager_.execute("ROLLBACK;");
        } catch (...) {
        }
    }
}

void Transaction::commit() {
    databaseManager_.execute("COMMIT;");
    active_ = false;
}

void Transaction::rollback() noexcept {
    if (!active_) {
        return;
    }

    try {
        databaseManager_.execute("ROLLBACK;");
    } catch (...) {
    }
    active_ = false;
}

}  // namespace hms::database
