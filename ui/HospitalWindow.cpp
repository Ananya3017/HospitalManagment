#include "HospitalWindowFactory.h"
#include "../core/Appointment.h"
#include "../core/Billing.h"
#include "../core/Doctor.h"
#include "../core/Patient.h"
#include "../core/User.h"
#include "../services/AppointmentService.h"
#include "../services/AuthService.h"
#include "../services/BillingService.h"
#include "../services/DoctorService.h"
#include "../services/PatientService.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace {

QString appointmentStatusToQString(hms::core::AppointmentStatus status) {
    switch (status) {
        case hms::core::AppointmentStatus::Scheduled:
            return "Scheduled";
        case hms::core::AppointmentStatus::Completed:
            return "Completed";
        case hms::core::AppointmentStatus::Cancelled:
            return "Cancelled";
    }
    return "Unknown";
}

void setReadOnlyItem(QTableWidget* table, int row, int col, const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    table->setItem(row, col, item);
}

void applyTableFilter(QTableWidget* table, const QString& rawQuery) {
    const QString query = rawQuery.trimmed();
    for (int row = 0; row < table->rowCount(); ++row) {
        bool match = query.isEmpty();
        if (!match) {
            for (int col = 0; col < table->columnCount(); ++col) {
                const auto* item = table->item(row, col);
                if (item && item->text().contains(query, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        table->setRowHidden(row, !match);
    }
}

}  // namespace

namespace hms::ui {

class HospitalWindow final : public QMainWindow {
public:
    HospitalWindow(services::PatientService& patientService,
                   services::DoctorService& doctorService,
                   services::AppointmentService& appointmentService,
                   services::BillingService& billingService,
                   services::AuthService& authService,
                   app::HospitalDataCoordinator& dataCoordinator,
                   QWidget* parent = nullptr)
        : QMainWindow(parent),
          patientService_(patientService),
          doctorService_(doctorService),
          appointmentService_(appointmentService),
          billingService_(billingService),
          authService_(authService),
          dataCoordinator_(dataCoordinator) {
        applyTheme();
        setupUi();
        wireSignals();
        refreshAllData();
        clearPatientForm();
        clearDoctorForm();
        clearAppointmentForm();
        clearBillForm();
        refreshSessionUi();
        statusBar()->showMessage("Ready", 2500);
    }

private:
    services::PatientService& patientService_;
    services::DoctorService& doctorService_;
    services::AppointmentService& appointmentService_;
    services::BillingService& billingService_;
    services::AuthService& authService_;
    app::HospitalDataCoordinator& dataCoordinator_;

    std::unique_ptr<hms::core::User> loggedInUser_;

    QLabel* patientCount_ = nullptr;
    QLabel* doctorCount_ = nullptr;
    QLabel* appointmentCount_ = nullptr;
    QLabel* billCount_ = nullptr;

    QLabel* sessionLabel_ = nullptr;
    QLineEdit* usernameInput_ = nullptr;
    QLineEdit* passwordInput_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QPushButton* logoutButton_ = nullptr;

    QLineEdit* patientSearch_ = nullptr;
    QSpinBox* patientIdInput_ = nullptr;
    QLineEdit* patientNameInput_ = nullptr;
    QSpinBox* patientAgeInput_ = nullptr;
    QLineEdit* patientContactInput_ = nullptr;
    QPushButton* savePatientButton_ = nullptr;
    QPushButton* updatePatientButton_ = nullptr;
    QPushButton* deletePatientButton_ = nullptr;
    QPushButton* clearPatientButton_ = nullptr;
    QTableWidget* patientTable_ = nullptr;

    QLineEdit* doctorSearch_ = nullptr;
    QSpinBox* doctorIdInput_ = nullptr;
    QLineEdit* doctorNameInput_ = nullptr;
    QLineEdit* doctorSpecInput_ = nullptr;
    QPushButton* saveDoctorButton_ = nullptr;
    QPushButton* updateDoctorButton_ = nullptr;
    QPushButton* deleteDoctorButton_ = nullptr;
    QPushButton* clearDoctorButton_ = nullptr;
    QTableWidget* doctorTable_ = nullptr;

    QLineEdit* appointmentSearch_ = nullptr;
    QSpinBox* appointmentIdInput_ = nullptr;
    QComboBox* appointmentPatientInput_ = nullptr;
    QComboBox* appointmentDoctorInput_ = nullptr;
    QDateTimeEdit* appointmentDateTimeInput_ = nullptr;
    QComboBox* appointmentStatusInput_ = nullptr;
    QPushButton* saveAppointmentButton_ = nullptr;
    QPushButton* updateAppointmentButton_ = nullptr;
    QPushButton* deleteAppointmentButton_ = nullptr;
    QPushButton* clearAppointmentButton_ = nullptr;
    QTableWidget* appointmentTable_ = nullptr;

    QLineEdit* billSearch_ = nullptr;
    QSpinBox* billIdInput_ = nullptr;
    QComboBox* billAppointmentInput_ = nullptr;
    QDoubleSpinBox* billAmountInput_ = nullptr;
    QComboBox* billPaidInput_ = nullptr;
    QPushButton* saveBillButton_ = nullptr;
    QPushButton* updateBillButton_ = nullptr;
    QPushButton* deleteBillButton_ = nullptr;
    QPushButton* clearBillButton_ = nullptr;
    QTableWidget* billTable_ = nullptr;

    [[nodiscard]] bool isLoggedIn() const { return static_cast<bool>(loggedInUser_); }

    [[nodiscard]] bool canManagePatients() const {
        if (!loggedInUser_) {
            return false;
        }
        const auto role = loggedInUser_->getRole();
        return role == hms::core::UserRole::Admin || role == hms::core::UserRole::Receptionist;
    }

    [[nodiscard]] static int selectedId(const QComboBox* combo) {
        bool ok = false;
        const int value = combo->currentData().toInt(&ok);
        return ok ? value : -1;
    }

    [[nodiscard]] static int tableIdAt(const QTableWidget* table, int row) {
        bool ok = false;
        const int id = table->item(row, 0)->text().toInt(&ok);
        return ok ? id : -1;
    }

    [[nodiscard]] static int selectedTableId(const QTableWidget* table) {
        const auto indexes = table->selectionModel()->selectedRows();
        if (indexes.isEmpty()) {
            return -1;
        }
        return tableIdAt(table, indexes.first().row());
    }

    [[nodiscard]] static int nextIdFromTable(const QTableWidget* table) {
        int maxId = 0;
        for (int row = 0; row < table->rowCount(); ++row) {
            bool ok = false;
            const int id = table->item(row, 0)->text().toInt(&ok);
            if (ok && id > maxId) {
                maxId = id;
            }
        }
        return maxId + 1;
    }

    [[nodiscard]] static hms::core::AppointmentStatus statusFromString(const QString& statusText) {
        if (statusText == "Completed") {
            return hms::core::AppointmentStatus::Completed;
        }
        if (statusText == "Cancelled") {
            return hms::core::AppointmentStatus::Cancelled;
        }
        return hms::core::AppointmentStatus::Scheduled;
    }

    void saveDataWithFeedback() {
        std::string error;
        if (!dataCoordinator_.save(error)) {
            QMessageBox::warning(this,
                                 "Persistence Error",
                                 "Data changed in memory, but saving to disk failed.\n"
                                     + QString::fromStdString(error));
        }
    }

    static void setSelectedId(QComboBox* combo, int id) {
        const int idx = combo->findData(id);
        combo->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    static QWidget* buttonRow(QWidget* parent, QPushButton* primary, QPushButton* secondary) {
        auto* row = new QWidget(parent);
        auto* layout = new QHBoxLayout(row);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);
        layout->addWidget(primary);
        layout->addWidget(secondary);
        return row;
    }

    static QWidget* actionRow(QWidget* parent, QPushButton* first, QPushButton* second, QPushButton* third, QPushButton* fourth) {
        auto* row = new QWidget(parent);
        auto* layout = new QHBoxLayout(row);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);
        layout->addWidget(first);
        layout->addWidget(second);
        layout->addWidget(third);
        layout->addWidget(fourth);
        return row;
    }

    static QFrame* metricCard(const QString& title, QLabel*& valueLabel, QWidget* parent) {
        auto* card = new QFrame(parent);
        card->setObjectName("metricCard");
        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(12, 10, 12, 10);
        auto* titleLabel = new QLabel(title, card);
        titleLabel->setObjectName("metricTitle");
        valueLabel = new QLabel("0", card);
        valueLabel->setObjectName("metricValue");
        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        return card;
    }

    void applyTheme() {
        setStyleSheet(R"(
QMainWindow { background: #f3f7fb; }
QGroupBox { border: 1px solid #cbd5e1; border-radius: 10px; margin-top: 10px; padding: 10px; font-weight: 600; background: #fff; }
QGroupBox::title { left: 10px; padding: 0 4px; color: #334155; }
QFrame#metricCard { border-radius: 12px; background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #0f766e, stop:1 #0e7490); }
QLabel#metricTitle { color: #d1fae5; font-size: 12px; }
QLabel#metricValue { color: #fff; font-size: 24px; font-weight: 700; }
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit { border: 1px solid #cbd5e1; border-radius: 8px; padding: 6px 8px; background: #fff; color: #0f172a; selection-background-color: #99f6e4; selection-color: #0f172a; }
QLineEdit::placeholder { color: #64748b; }
QPushButton { border: none; border-radius: 8px; padding: 7px 14px; background: #0ea5a4; color: #fff; font-weight: 600; }
QPushButton:hover { background: #0f766e; }
QPushButton:disabled { background: #cbd5e1; color: #64748b; }
QTableWidget { border: 1px solid #cbd5e1; border-radius: 8px; background: #fff; color: #0f172a; gridline-color: #e2e8f0; selection-background-color: #99f6e4; selection-color: #0f172a; }
QHeaderView::section { background: #e2e8f0; border: none; border-right: 1px solid #cbd5e1; border-bottom: 1px solid #cbd5e1; padding: 6px; font-weight: 600; color: #334155; }
QLabel { color: #0f172a; }
)");
    }

    void setupUi() {
        setWindowTitle("Hospital Management System");
        resize(1100, 720);

        auto* central = new QWidget(this);
        auto* root = new QVBoxLayout(central);
        root->setSpacing(10);
        root->setContentsMargins(10, 10, 10, 10);

        auto* session = new QGroupBox("Session", central);
        auto* sessionLayout = new QGridLayout(session);
        sessionLabel_ = new QLabel("Logged in as: [none]", session);
        usernameInput_ = new QLineEdit(session);
        passwordInput_ = new QLineEdit(session);
        loginButton_ = new QPushButton("Login", session);
        logoutButton_ = new QPushButton("Logout", session);
        auto* hint = new QLabel("Default users: admin/admin123 | reception/rec123 | doctor1/doc123", session);

        passwordInput_->setEchoMode(QLineEdit::Password);
        usernameInput_->setPlaceholderText("username");
        passwordInput_->setPlaceholderText("password");

        sessionLayout->addWidget(sessionLabel_, 0, 0, 1, 4);
        sessionLayout->addWidget(new QLabel("Username", session), 1, 0);
        sessionLayout->addWidget(usernameInput_, 1, 1);
        sessionLayout->addWidget(new QLabel("Password", session), 1, 2);
        sessionLayout->addWidget(passwordInput_, 1, 3);
        sessionLayout->addWidget(hint, 2, 0, 1, 2);
        sessionLayout->addWidget(loginButton_, 2, 2);
        sessionLayout->addWidget(logoutButton_, 2, 3);

        auto* metrics = new QWidget(central);
        auto* metricsLayout = new QHBoxLayout(metrics);
        metricsLayout->setContentsMargins(0, 0, 0, 0);
        metricsLayout->setSpacing(10);
        metricsLayout->addWidget(metricCard("Patients", patientCount_, metrics), 1);
        metricsLayout->addWidget(metricCard("Doctors", doctorCount_, metrics), 1);
        metricsLayout->addWidget(metricCard("Appointments", appointmentCount_, metrics), 1);
        metricsLayout->addWidget(metricCard("Bills", billCount_, metrics), 1);

        auto* tabs = new QTabWidget(central);
        tabs->addTab(buildPatientsTab(tabs), "Patients");
        tabs->addTab(buildDoctorsTab(tabs), "Doctors");
        tabs->addTab(buildAppointmentsTab(tabs), "Appointments");
        tabs->addTab(buildBillsTab(tabs), "Billing");

        root->addWidget(session);
        root->addWidget(metrics);
        root->addWidget(tabs, 1);
        setCentralWidget(central);
    }

    QWidget* buildPatientsTab(QWidget* parent) {
        auto* tab = new QWidget(parent);
        auto* layout = new QVBoxLayout(tab);
        auto* searchRow = new QHBoxLayout();
        searchRow->addWidget(new QLabel("Search", tab));
        patientSearch_ = new QLineEdit(tab);
        patientSearch_->setPlaceholderText("Filter patients...");
        searchRow->addWidget(patientSearch_, 1);

        auto* formGroup = new QGroupBox("Register Patient", tab);
        auto* form = new QFormLayout(formGroup);
        patientIdInput_ = new QSpinBox(formGroup);
        patientIdInput_->setRange(1, 999999);
        patientNameInput_ = new QLineEdit(formGroup);
        patientAgeInput_ = new QSpinBox(formGroup);
        patientAgeInput_->setRange(0, 130);
        patientContactInput_ = new QLineEdit(formGroup);
        savePatientButton_ = new QPushButton("Save Patient", formGroup);
        updatePatientButton_ = new QPushButton("Update Selected", formGroup);
        deletePatientButton_ = new QPushButton("Delete Selected", formGroup);
        clearPatientButton_ = new QPushButton("Clear", formGroup);
        form->addRow("Patient ID", patientIdInput_);
        form->addRow("Name", patientNameInput_);
        form->addRow("Age", patientAgeInput_);
        form->addRow("Contact", patientContactInput_);
        form->addRow("", actionRow(formGroup, savePatientButton_, updatePatientButton_, deletePatientButton_, clearPatientButton_));

        patientTable_ = new QTableWidget(tab);
        patientTable_->setColumnCount(4);
        patientTable_->setHorizontalHeaderLabels(QStringList({"ID", "Name", "Age", "Contact"}));
        patientTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        patientTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        patientTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        layout->addLayout(searchRow);
        layout->addWidget(formGroup);
        layout->addWidget(patientTable_, 1);
        return tab;
    }

    QWidget* buildDoctorsTab(QWidget* parent) {
        auto* tab = new QWidget(parent);
        auto* layout = new QVBoxLayout(tab);
        auto* searchRow = new QHBoxLayout();
        searchRow->addWidget(new QLabel("Search", tab));
        doctorSearch_ = new QLineEdit(tab);
        doctorSearch_->setPlaceholderText("Filter doctors...");
        searchRow->addWidget(doctorSearch_, 1);

        auto* formGroup = new QGroupBox("Add Doctor", tab);
        auto* form = new QFormLayout(formGroup);
        doctorIdInput_ = new QSpinBox(formGroup);
        doctorIdInput_->setRange(1, 999999);
        doctorNameInput_ = new QLineEdit(formGroup);
        doctorSpecInput_ = new QLineEdit(formGroup);
        saveDoctorButton_ = new QPushButton("Save Doctor", formGroup);
        updateDoctorButton_ = new QPushButton("Update Selected", formGroup);
        deleteDoctorButton_ = new QPushButton("Delete Selected", formGroup);
        clearDoctorButton_ = new QPushButton("Clear", formGroup);
        form->addRow("Doctor ID", doctorIdInput_);
        form->addRow("Name", doctorNameInput_);
        form->addRow("Specialization", doctorSpecInput_);
        form->addRow("", actionRow(formGroup, saveDoctorButton_, updateDoctorButton_, deleteDoctorButton_, clearDoctorButton_));

        doctorTable_ = new QTableWidget(tab);
        doctorTable_->setColumnCount(3);
        doctorTable_->setHorizontalHeaderLabels(QStringList({"ID", "Name", "Specialization"}));
        doctorTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        doctorTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        doctorTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        layout->addLayout(searchRow);
        layout->addWidget(formGroup);
        layout->addWidget(doctorTable_, 1);
        return tab;
    }

    QWidget* buildAppointmentsTab(QWidget* parent) {
        auto* tab = new QWidget(parent);
        auto* layout = new QVBoxLayout(tab);
        auto* searchRow = new QHBoxLayout();
        searchRow->addWidget(new QLabel("Search", tab));
        appointmentSearch_ = new QLineEdit(tab);
        appointmentSearch_->setPlaceholderText("Filter appointments...");
        searchRow->addWidget(appointmentSearch_, 1);

        auto* formGroup = new QGroupBox("Book Appointment", tab);
        auto* form = new QFormLayout(formGroup);
        appointmentIdInput_ = new QSpinBox(formGroup);
        appointmentIdInput_->setRange(1, 999999);
        appointmentPatientInput_ = new QComboBox(formGroup);
        appointmentDoctorInput_ = new QComboBox(formGroup);
        appointmentDateTimeInput_ = new QDateTimeEdit(QDateTime::currentDateTime(), formGroup);
        appointmentDateTimeInput_->setDisplayFormat("yyyy-MM-dd HH:mm");
        appointmentDateTimeInput_->setCalendarPopup(true);
        appointmentStatusInput_ = new QComboBox(formGroup);
        appointmentStatusInput_->addItems(QStringList({"Scheduled", "Completed", "Cancelled"}));
        saveAppointmentButton_ = new QPushButton("Save Appointment", formGroup);
        updateAppointmentButton_ = new QPushButton("Update Selected", formGroup);
        deleteAppointmentButton_ = new QPushButton("Delete Selected", formGroup);
        clearAppointmentButton_ = new QPushButton("Clear", formGroup);
        form->addRow("Appointment ID", appointmentIdInput_);
        form->addRow("Patient", appointmentPatientInput_);
        form->addRow("Doctor", appointmentDoctorInput_);
        form->addRow("Date & Time", appointmentDateTimeInput_);
        form->addRow("Status", appointmentStatusInput_);
        form->addRow("", actionRow(formGroup, saveAppointmentButton_, updateAppointmentButton_, deleteAppointmentButton_, clearAppointmentButton_));

        appointmentTable_ = new QTableWidget(tab);
        appointmentTable_->setColumnCount(5);
        appointmentTable_->setHorizontalHeaderLabels(QStringList({"ID", "Patient ID", "Doctor ID", "Date & Time", "Status"}));
        appointmentTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        appointmentTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        appointmentTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        layout->addLayout(searchRow);
        layout->addWidget(formGroup);
        layout->addWidget(appointmentTable_, 1);
        return tab;
    }

    QWidget* buildBillsTab(QWidget* parent) {
        auto* tab = new QWidget(parent);
        auto* layout = new QVBoxLayout(tab);
        auto* searchRow = new QHBoxLayout();
        searchRow->addWidget(new QLabel("Search", tab));
        billSearch_ = new QLineEdit(tab);
        billSearch_->setPlaceholderText("Filter bills...");
        searchRow->addWidget(billSearch_, 1);

        auto* formGroup = new QGroupBox("Generate Bill", tab);
        auto* form = new QFormLayout(formGroup);
        billIdInput_ = new QSpinBox(formGroup);
        billIdInput_->setRange(1, 999999);
        billAppointmentInput_ = new QComboBox(formGroup);
        billAmountInput_ = new QDoubleSpinBox(formGroup);
        billAmountInput_->setRange(hms::services::BillingService::appointmentCharge(),
                                   hms::services::BillingService::appointmentCharge());
        billAmountInput_->setDecimals(2);
        billAmountInput_->setReadOnly(true);
        billAmountInput_->setValue(hms::services::BillingService::appointmentCharge());
        billPaidInput_ = new QComboBox(formGroup);
        billPaidInput_->addItems(QStringList({"No", "Yes"}));
        saveBillButton_ = new QPushButton("Save Bill", formGroup);
        updateBillButton_ = new QPushButton("Update Selected", formGroup);
        deleteBillButton_ = new QPushButton("Delete Selected", formGroup);
        clearBillButton_ = new QPushButton("Clear", formGroup);
        form->addRow("Bill ID", billIdInput_);
        form->addRow("Appointment", billAppointmentInput_);
        form->addRow("Amount (Fixed)", billAmountInput_);
        form->addRow("Paid", billPaidInput_);
        form->addRow("", actionRow(formGroup, saveBillButton_, updateBillButton_, deleteBillButton_, clearBillButton_));

        billTable_ = new QTableWidget(tab);
        billTable_->setColumnCount(4);
        billTable_->setHorizontalHeaderLabels(QStringList({"ID", "Appointment ID", "Amount", "Paid"}));
        billTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        billTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        billTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        layout->addLayout(searchRow);
        layout->addWidget(formGroup);
        layout->addWidget(billTable_, 1);
        return tab;
    }

    void wireSignals() {
        connect(loginButton_, &QPushButton::clicked, this, [this]() {
            try {
                loggedInUser_ = authService_.login(usernameInput_->text().toStdString(), passwordInput_->text().toStdString());
                passwordInput_->clear();
                refreshSessionUi();
                statusBar()->showMessage("Login successful.", 3000);
            } catch (const std::exception& e) {
                showError(e);
            }
        });
        connect(passwordInput_, &QLineEdit::returnPressed, loginButton_, &QPushButton::click);

        connect(logoutButton_, &QPushButton::clicked, this, [this]() {
            loggedInUser_.reset();
            refreshSessionUi();
            statusBar()->showMessage("Logged out.", 2000);
        });

        connect(savePatientButton_, &QPushButton::clicked, this, [this]() {
            if (!canManagePatients()) {
                QMessageBox::warning(this, "Permission Denied", "Only Admin or Receptionist can add patients.");
                return;
            }
            try {
                patientService_.registerPatient(patientIdInput_->value(),
                                                patientNameInput_->text().toStdString(),
                                                patientAgeInput_->value(),
                                                patientContactInput_->text().toStdString());
                patientSearch_->clear();
                refreshAllData();
                clearPatientForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Patient saved.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(updatePatientButton_, &QPushButton::clicked, this, [this]() {
            if (!canManagePatients()) {
                QMessageBox::warning(this, "Permission Denied", "Only Admin or Receptionist can update patients.");
                return;
            }
            try {
                patientService_.updatePatient(patientIdInput_->value(),
                                              patientNameInput_->text().toStdString(),
                                              patientAgeInput_->value(),
                                              patientContactInput_->text().toStdString());
                patientSearch_->clear();
                refreshAllData();
                saveDataWithFeedback();
                statusBar()->showMessage("Patient updated.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(deletePatientButton_, &QPushButton::clicked, this, [this]() {
            if (!canManagePatients()) {
                QMessageBox::warning(this, "Permission Denied", "Only Admin or Receptionist can delete patients.");
                return;
            }
            const int id = selectedTableId(patientTable_);
            if (id <= 0) {
                QMessageBox::information(this, "Select Row", "Select a patient row to delete.");
                return;
            }
            try {
                patientService_.deletePatient(id);
                patientSearch_->clear();
                refreshAllData();
                clearPatientForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Patient deleted.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(saveDoctorButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            try {
                doctorService_.addDoctor(
                    doctorIdInput_->value(), doctorNameInput_->text().toStdString(), doctorSpecInput_->text().toStdString());
                doctorSearch_->clear();
                refreshAllData();
                clearDoctorForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Doctor saved.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(updateDoctorButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            try {
                doctorService_.updateDoctor(
                    doctorIdInput_->value(), doctorNameInput_->text().toStdString(), doctorSpecInput_->text().toStdString());
                doctorSearch_->clear();
                refreshAllData();
                saveDataWithFeedback();
                statusBar()->showMessage("Doctor updated.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(deleteDoctorButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int id = selectedTableId(doctorTable_);
            if (id <= 0) {
                QMessageBox::information(this, "Select Row", "Select a doctor row to delete.");
                return;
            }
            try {
                doctorService_.deleteDoctor(id);
                doctorSearch_->clear();
                refreshAllData();
                clearDoctorForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Doctor deleted.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(saveAppointmentButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int patientId = selectedId(appointmentPatientInput_);
            const int doctorId = selectedId(appointmentDoctorInput_);
            if (patientId <= 0 || doctorId <= 0) {
                QMessageBox::warning(this, "Missing Data", "Select patient and doctor.");
                return;
            }
            try {
                appointmentService_.bookAppointment(appointmentIdInput_->value(),
                                                    patientId,
                                                    doctorId,
                                                    appointmentDateTimeInput_->dateTime().toString("yyyy-MM-dd HH:mm").toStdString());
                appointmentSearch_->clear();
                refreshAllData();
                clearAppointmentForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Appointment saved.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(updateAppointmentButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int patientId = selectedId(appointmentPatientInput_);
            const int doctorId = selectedId(appointmentDoctorInput_);
            if (patientId <= 0 || doctorId <= 0) {
                QMessageBox::warning(this, "Missing Data", "Select patient and doctor.");
                return;
            }
            try {
                appointmentService_.updateAppointment(
                    appointmentIdInput_->value(),
                    patientId,
                    doctorId,
                    appointmentDateTimeInput_->dateTime().toString("yyyy-MM-dd HH:mm").toStdString(),
                    statusFromString(appointmentStatusInput_->currentText()));
                appointmentSearch_->clear();
                refreshAllData();
                saveDataWithFeedback();
                statusBar()->showMessage("Appointment updated.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(deleteAppointmentButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int id = selectedTableId(appointmentTable_);
            if (id <= 0) {
                QMessageBox::information(this, "Select Row", "Select an appointment row to delete.");
                return;
            }
            try {
                appointmentService_.deleteAppointment(id);
                appointmentSearch_->clear();
                refreshAllData();
                clearAppointmentForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Appointment deleted.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(saveBillButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int appointmentId = selectedId(billAppointmentInput_);
            if (appointmentId <= 0) {
                QMessageBox::warning(this, "Missing Data", "Select an appointment.");
                return;
            }
            try {
                billingService_.generateBill(
                    billIdInput_->value(), appointmentId, hms::services::BillingService::appointmentCharge());
                billSearch_->clear();
                refreshAllData();
                clearBillForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Bill saved.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(updateBillButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int appointmentId = selectedId(billAppointmentInput_);
            if (appointmentId <= 0) {
                QMessageBox::warning(this, "Missing Data", "Select an appointment.");
                return;
            }
            try {
                billingService_.updateBill(
                    billIdInput_->value(),
                    appointmentId,
                    hms::services::BillingService::appointmentCharge(),
                    billPaidInput_->currentText() == "Yes");
                billSearch_->clear();
                refreshAllData();
                saveDataWithFeedback();
                statusBar()->showMessage("Bill updated.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(deleteBillButton_, &QPushButton::clicked, this, [this]() {
            if (!isLoggedIn()) {
                QMessageBox::warning(this, "Login Required", "Login first.");
                return;
            }
            const int id = selectedTableId(billTable_);
            if (id <= 0) {
                QMessageBox::information(this, "Select Row", "Select a bill row to delete.");
                return;
            }
            try {
                billingService_.deleteBill(id);
                billSearch_->clear();
                refreshAllData();
                clearBillForm();
                saveDataWithFeedback();
                statusBar()->showMessage("Bill deleted.", 2500);
            } catch (const std::exception& e) {
                showError(e);
            }
        });

        connect(clearPatientButton_, &QPushButton::clicked, this, [this]() { clearPatientForm(); });
        connect(clearDoctorButton_, &QPushButton::clicked, this, [this]() { clearDoctorForm(); });
        connect(clearAppointmentButton_, &QPushButton::clicked, this, [this]() { clearAppointmentForm(); });
        connect(clearBillButton_, &QPushButton::clicked, this, [this]() { clearBillForm(); });

        connect(patientTable_, &QTableWidget::itemSelectionChanged, this, [this]() {
            const auto rows = patientTable_->selectionModel()->selectedRows();
            if (rows.isEmpty()) {
                return;
            }
            const int row = rows.first().row();
            patientIdInput_->setValue(tableIdAt(patientTable_, row));
            patientNameInput_->setText(patientTable_->item(row, 1)->text());
            patientAgeInput_->setValue(patientTable_->item(row, 2)->text().toInt());
            patientContactInput_->setText(patientTable_->item(row, 3)->text());
        });

        connect(doctorTable_, &QTableWidget::itemSelectionChanged, this, [this]() {
            const auto rows = doctorTable_->selectionModel()->selectedRows();
            if (rows.isEmpty()) {
                return;
            }
            const int row = rows.first().row();
            doctorIdInput_->setValue(tableIdAt(doctorTable_, row));
            doctorNameInput_->setText(doctorTable_->item(row, 1)->text());
            doctorSpecInput_->setText(doctorTable_->item(row, 2)->text());
        });

        connect(appointmentTable_, &QTableWidget::itemSelectionChanged, this, [this]() {
            const auto rows = appointmentTable_->selectionModel()->selectedRows();
            if (rows.isEmpty()) {
                return;
            }
            const int row = rows.first().row();
            appointmentIdInput_->setValue(tableIdAt(appointmentTable_, row));
            setSelectedId(appointmentPatientInput_, appointmentTable_->item(row, 1)->text().toInt());
            setSelectedId(appointmentDoctorInput_, appointmentTable_->item(row, 2)->text().toInt());
            const QDateTime dt = QDateTime::fromString(appointmentTable_->item(row, 3)->text(), "yyyy-MM-dd HH:mm");
            if (dt.isValid()) {
                appointmentDateTimeInput_->setDateTime(dt);
            }
            appointmentStatusInput_->setCurrentText(appointmentTable_->item(row, 4)->text());
        });

        connect(billTable_, &QTableWidget::itemSelectionChanged, this, [this]() {
            const auto rows = billTable_->selectionModel()->selectedRows();
            if (rows.isEmpty()) {
                return;
            }
            const int row = rows.first().row();
            billIdInput_->setValue(tableIdAt(billTable_, row));
            setSelectedId(billAppointmentInput_, billTable_->item(row, 1)->text().toInt());
            billAmountInput_->setValue(billTable_->item(row, 2)->text().toDouble());
            billPaidInput_->setCurrentText(billTable_->item(row, 3)->text());
        });

        connect(patientSearch_, &QLineEdit::textChanged, this, [this](const QString& text) { applyTableFilter(patientTable_, text); });
        connect(doctorSearch_, &QLineEdit::textChanged, this, [this](const QString& text) { applyTableFilter(doctorTable_, text); });
        connect(appointmentSearch_, &QLineEdit::textChanged, this, [this](const QString& text) { applyTableFilter(appointmentTable_, text); });
        connect(billSearch_, &QLineEdit::textChanged, this, [this](const QString& text) { applyTableFilter(billTable_, text); });
    }

    void refreshSessionUi() {
        if (loggedInUser_) {
            sessionLabel_->setText(QString("Logged in as: %1 (%2)")
                                       .arg(QString::fromStdString(loggedInUser_->getUsername()))
                                       .arg(QString::fromStdString(hms::core::toString(loggedInUser_->getRole()))));
        } else {
            sessionLabel_->setText("Logged in as: [none]");
        }

        const bool loggedIn = isLoggedIn();
        logoutButton_->setEnabled(loggedIn);
        savePatientButton_->setEnabled(canManagePatients());
        updatePatientButton_->setEnabled(canManagePatients());
        deletePatientButton_->setEnabled(canManagePatients());
        saveDoctorButton_->setEnabled(loggedIn);
        updateDoctorButton_->setEnabled(loggedIn);
        deleteDoctorButton_->setEnabled(loggedIn);
        saveAppointmentButton_->setEnabled(loggedIn);
        updateAppointmentButton_->setEnabled(loggedIn);
        deleteAppointmentButton_->setEnabled(loggedIn);
        saveBillButton_->setEnabled(loggedIn);
        updateBillButton_->setEnabled(loggedIn);
        deleteBillButton_->setEnabled(loggedIn);
        appointmentStatusInput_->setEnabled(loggedIn);
        billPaidInput_->setEnabled(loggedIn);
    }

    void refreshAllData() {
        refreshPatientTable();
        refreshDoctorTable();
        refreshAppointmentTable();
        refreshBillTable();
        refreshSelectors();
        patientCount_->setText(QString::number(patientTable_->rowCount()));
        doctorCount_->setText(QString::number(doctorTable_->rowCount()));
        appointmentCount_->setText(QString::number(appointmentTable_->rowCount()));
        billCount_->setText(QString::number(billTable_->rowCount()));
        applyTableFilter(patientTable_, patientSearch_->text());
        applyTableFilter(doctorTable_, doctorSearch_->text());
        applyTableFilter(appointmentTable_, appointmentSearch_->text());
        applyTableFilter(billTable_, billSearch_->text());
    }

    void refreshSelectors() {
        const int patientSel = selectedId(appointmentPatientInput_);
        const int doctorSel = selectedId(appointmentDoctorInput_);
        const int appointmentSel = selectedId(billAppointmentInput_);

        std::vector<hms::core::Patient> patients = patientService_.getAllPatients();
        std::sort(patients.begin(), patients.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        {
            QSignalBlocker blocker(appointmentPatientInput_);
            appointmentPatientInput_->clear();
            appointmentPatientInput_->addItem("Select patient", -1);
            for (const auto& p : patients) {
                appointmentPatientInput_->addItem(QString("%1 - %2").arg(p.getId()).arg(QString::fromStdString(p.getName())), p.getId());
            }
            setSelectedId(appointmentPatientInput_, patientSel);
        }

        std::vector<hms::core::Doctor> doctors = doctorService_.getAllDoctors();
        std::sort(doctors.begin(), doctors.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        {
            QSignalBlocker blocker(appointmentDoctorInput_);
            appointmentDoctorInput_->clear();
            appointmentDoctorInput_->addItem("Select doctor", -1);
            for (const auto& d : doctors) {
                appointmentDoctorInput_->addItem(QString("%1 - %2").arg(d.getId()).arg(QString::fromStdString(d.getName())), d.getId());
            }
            setSelectedId(appointmentDoctorInput_, doctorSel);
        }

        std::vector<hms::core::Appointment> appointments = appointmentService_.getAllAppointments();
        std::sort(appointments.begin(), appointments.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        {
            QSignalBlocker blocker(billAppointmentInput_);
            billAppointmentInput_->clear();
            billAppointmentInput_->addItem("Select appointment", -1);
            for (const auto& a : appointments) {
                billAppointmentInput_->addItem(
                    QString("%1 - Patient %2 / Doctor %3").arg(a.getId()).arg(a.getPatientId()).arg(a.getDoctorId()),
                    a.getId());
            }
            setSelectedId(billAppointmentInput_, appointmentSel);
        }
    }

    void refreshPatientTable() {
        std::vector<hms::core::Patient> patients = patientService_.getAllPatients();
        std::sort(patients.begin(), patients.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        patientTable_->setRowCount(0);
        for (const auto& p : patients) {
            const int row = patientTable_->rowCount();
            patientTable_->insertRow(row);
            setReadOnlyItem(patientTable_, row, 0, QString::number(p.getId()));
            setReadOnlyItem(patientTable_, row, 1, QString::fromStdString(p.getName()));
            setReadOnlyItem(patientTable_, row, 2, QString::number(p.getAge()));
            setReadOnlyItem(patientTable_, row, 3, QString::fromStdString(p.getContactNumber()));
        }
    }

    void refreshDoctorTable() {
        std::vector<hms::core::Doctor> doctors = doctorService_.getAllDoctors();
        std::sort(doctors.begin(), doctors.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        doctorTable_->setRowCount(0);
        for (const auto& d : doctors) {
            const int row = doctorTable_->rowCount();
            doctorTable_->insertRow(row);
            setReadOnlyItem(doctorTable_, row, 0, QString::number(d.getId()));
            setReadOnlyItem(doctorTable_, row, 1, QString::fromStdString(d.getName()));
            setReadOnlyItem(doctorTable_, row, 2, QString::fromStdString(d.getSpecialization()));
        }
    }

    void refreshAppointmentTable() {
        std::vector<hms::core::Appointment> appointments = appointmentService_.getAllAppointments();
        std::sort(appointments.begin(), appointments.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        appointmentTable_->setRowCount(0);
        for (const auto& a : appointments) {
            const int row = appointmentTable_->rowCount();
            appointmentTable_->insertRow(row);
            setReadOnlyItem(appointmentTable_, row, 0, QString::number(a.getId()));
            setReadOnlyItem(appointmentTable_, row, 1, QString::number(a.getPatientId()));
            setReadOnlyItem(appointmentTable_, row, 2, QString::number(a.getDoctorId()));
            setReadOnlyItem(appointmentTable_, row, 3, QString::fromStdString(a.getDateTime()));
            setReadOnlyItem(appointmentTable_, row, 4, appointmentStatusToQString(a.getStatus()));
        }
    }

    void refreshBillTable() {
        std::vector<hms::core::Billing> bills = billingService_.getAllBills();
        std::sort(bills.begin(), bills.end(), [](const auto& a, const auto& b) { return a.getId() < b.getId(); });
        billTable_->setRowCount(0);
        for (const auto& b : bills) {
            const int row = billTable_->rowCount();
            billTable_->insertRow(row);
            setReadOnlyItem(billTable_, row, 0, QString::number(b.getId()));
            setReadOnlyItem(billTable_, row, 1, QString::number(b.getAppointmentId()));
            setReadOnlyItem(billTable_, row, 2, QString::number(b.getAmount(), 'f', 2));
            setReadOnlyItem(billTable_, row, 3, b.isPaid() ? "Yes" : "No");
        }
    }

    void clearPatientForm() {
        patientIdInput_->setValue(nextIdFromTable(patientTable_));
        patientNameInput_->clear();
        patientAgeInput_->setValue(0);
        patientContactInput_->clear();
        patientTable_->clearSelection();
    }

    void clearDoctorForm() {
        doctorIdInput_->setValue(nextIdFromTable(doctorTable_));
        doctorNameInput_->clear();
        doctorSpecInput_->clear();
        doctorTable_->clearSelection();
    }

    void clearAppointmentForm() {
        appointmentIdInput_->setValue(nextIdFromTable(appointmentTable_));
        setSelectedId(appointmentPatientInput_, -1);
        setSelectedId(appointmentDoctorInput_, -1);
        appointmentDateTimeInput_->setDateTime(QDateTime::currentDateTime());
        appointmentStatusInput_->setCurrentText("Scheduled");
        appointmentTable_->clearSelection();
    }

    void clearBillForm() {
        billIdInput_->setValue(nextIdFromTable(billTable_));
        setSelectedId(billAppointmentInput_, -1);
        billAmountInput_->setValue(hms::services::BillingService::appointmentCharge());
        billPaidInput_->setCurrentText("No");
        billTable_->clearSelection();
    }

    void showError(const std::exception& e) { QMessageBox::critical(this, "Error", e.what()); }
};

std::unique_ptr<QMainWindow> createHospitalWindow(
    services::PatientService& patientService,
    services::DoctorService& doctorService,
    services::AppointmentService& appointmentService,
    services::BillingService& billingService,
    services::AuthService& authService,
    app::HospitalDataCoordinator& dataCoordinator,
    QWidget* parent) {
    return std::make_unique<HospitalWindow>(
        patientService, doctorService, appointmentService, billingService, authService, dataCoordinator, parent);
}

}  // namespace hms::ui
