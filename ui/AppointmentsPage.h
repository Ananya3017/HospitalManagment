#pragma once

#include "../core/User.h"
#include "../services/AppointmentService.h"
#include "../services/PatientService.h"
#include "../services/DoctorService.h"
#include "../models/AppointmentModel.h"
#include <QWidget>
#include <memory>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QTableView;
class QSortFilterProxyModel;
class QComboBox;
class QDateTimeEdit;
#include <QDialog>
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QFormLayout>

namespace hms::ui {

class AppointmentsPage final : public QWidget {
    Q_OBJECT

public:
    explicit AppointmentsPage(services::AppointmentService& appointmentService,
                             services::PatientService& patientService,
                             services::DoctorService& doctorService,
                             QWidget* parent = nullptr);

    void setRole(core::UserRole role);
    void setCurrentUser(const core::User& user);
    void refresh();

signals:
    void appointmentSaved();
    void appointmentDeleted();
    void errorOccurred(const std::string& message);
    void registerPatientRequested();

private:
    void setupUi();
    void wireSignals();
    void clearForm();
    void onSelectionChanged();
    void onSaveClicked();
    void onUpdateClicked();
    void onDeleteClicked();
    void updateDoctorSelection();

    services::AppointmentService& appointmentService_;
    services::PatientService& patientService_;
    services::DoctorService& doctorService_;
    
    std::unique_ptr<core::User> currentUser_ {};
    core::UserRole currentRole_ = core::UserRole::Patient;

    models::AppointmentModel* appointmentModel_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    QWidget* formCard_ = nullptr;
    QPushButton* requestAppointmentButton_ = nullptr;

    QLineEdit* searchInput_ = nullptr;
    QSpinBox* idInput_ = nullptr;
    QComboBox* patientCombo_ = nullptr;
    QComboBox* specCombo_ = nullptr;
    QComboBox* doctorCombo_ = nullptr;
    QDateTimeEdit* dateTimeEdit_ = nullptr;
    QComboBox* statusCombo_ = nullptr;
    QPushButton* registerPatientButton_ = nullptr;

    QPushButton* saveButton_ = nullptr;
    QPushButton* updateButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;

    QTableView* tableView_ = nullptr;
};

}  // namespace hms::ui
