#pragma once

#include "../core/User.h"
#include "../services/PatientService.h"
#include "../models/PatientModel.h"
#include <QWidget>
#include <memory>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QTableView;
class QSortFilterProxyModel;

namespace hms::ui {

class PatientsPage final : public QWidget {
    Q_OBJECT

public:
    explicit PatientsPage(services::PatientService& patientService, QWidget* parent = nullptr);

    void setRole(core::UserRole role);
    void setCurrentUser(const core::User& user);
    void refresh();

signals:
    void patientAdded();
    void patientUpdated();
    void patientDeleted();
    void errorOccurred(const std::string& message);

private:
    void setupUi();
    void wireSignals();
    void clearForm();
    void onSelectionChanged();
    void onSaveClicked();
    void onUpdateClicked();
    void onDeleteClicked();

    services::PatientService& patientService_;
    std::unique_ptr<core::User> currentUser_ {};
    core::UserRole currentRole_ = core::UserRole::Patient;

    models::PatientModel* patientModel_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;

    QLineEdit* searchInput_ = nullptr;
    QSpinBox* idInput_ = nullptr;
    QLineEdit* nameInput_ = nullptr;
    QSpinBox* ageInput_ = nullptr;
    QLineEdit* contactInput_ = nullptr;

    QPushButton* saveButton_ = nullptr;
    QPushButton* updateButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;

    QTableView* tableView_ = nullptr;
};

}  // namespace hms::ui
