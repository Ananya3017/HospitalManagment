#pragma once

#include "../core/User.h"
#include "../services/BillingService.h"
#include "../services/AppointmentService.h"
#include "../models/BillModel.h"
#include <QWidget>
#include <memory>

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QTableView;
class QSortFilterProxyModel;
class QComboBox;

namespace hms::ui {

class BillingPage final : public QWidget {
    Q_OBJECT

public:
    explicit BillingPage(services::BillingService& billingService,
                        services::AppointmentService& appointmentService,
                        QWidget* parent = nullptr);

    void setRole(core::UserRole role);
    void setCurrentUser(const core::User& user);
    void refresh();

signals:
    void billSaved();
    void billDeleted();
    void errorOccurred(const std::string& message);

private:
    void setupUi();
    void wireSignals();
    void clearForm();
    void onSelectionChanged();
    void onSaveClicked();
    void onUpdateClicked();
    void onDeleteClicked();

    services::BillingService& billingService_;
    services::AppointmentService& appointmentService_;
    
    std::unique_ptr<core::User> currentUser_ {};
    core::UserRole currentRole_ = core::UserRole::Patient;

    models::BillModel* billModel_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;

    QLineEdit* searchInput_ = nullptr;
    QSpinBox* idInput_ = nullptr;
    QComboBox* appointmentCombo_ = nullptr;
    QDoubleSpinBox* amountInput_ = nullptr;
    QComboBox* paidCombo_ = nullptr;

    QPushButton* saveButton_ = nullptr;
    QPushButton* updateButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;

    QTableView* tableView_ = nullptr;
};

}  // namespace hms::ui
