#include "HospitalWindowView.h"
#include "../utils/PermissionManager.h"
#include <map>
#include <string>
#include <QButtonGroup>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>

namespace hms::ui {

HospitalWindow::HospitalWindow(services::PatientService& patientService,
                               services::DoctorService& doctorService,
                               services::AppointmentService& appointmentService,
                               services::BillingService& billingService,
                               services::AuthService& authService,
                               QWidget* parent)
    : QMainWindow(parent),
      patientService_(patientService),
      doctorService_(doctorService),
      appointmentService_(appointmentService),
      billingService_(billingService),
      authService_(authService) {
    applyTheme();
    setupUi();
    wireSignals();
    refreshAllData();
    refreshSessionUi();
    activatePage(PageIndex::RoleSelection);
    statusBar()->showMessage("Ready", 2500);
}

void HospitalWindow::activatePage(PageIndex pageIndex) {
    pageStack_->setCurrentIndex(static_cast<int>(pageIndex));
    
    const bool isNavPage = (pageIndex >= PageIndex::Dashboard);
    sidebar_->setVisible(isNavPage);
    topBar_->setVisible(isNavPage);

    if (navigationGroup_) {
        auto* btn = navigationGroup_->button(static_cast<int>(pageIndex));
        if (btn) btn->setChecked(true);
    }

    if (pageIndex == PageIndex::Dashboard) refreshDashboardMetrics();
    else if (pageIndex == PageIndex::Patients) refreshPatientData();
    else if (pageIndex == PageIndex::Doctors) refreshDoctorData();
    else if (pageIndex == PageIndex::Appointments) refreshAppointmentData();
    else if (pageIndex == PageIndex::Billing) refreshBillData();

    if (pageTitleLabel_) {
        switch (pageIndex) {
            case PageIndex::Dashboard: pageTitleLabel_->setText("Dashboard"); break;
            case PageIndex::Patients: pageTitleLabel_->setText("Patients"); break;
            case PageIndex::Doctors: pageTitleLabel_->setText("Doctors"); break;
            case PageIndex::Appointments: pageTitleLabel_->setText("Appointments"); break;
            case PageIndex::Billing: pageTitleLabel_->setText("Billing"); break;
            case PageIndex::Prescriptions: pageTitleLabel_->setText("Prescriptions"); break;
            default: pageTitleLabel_->setText("Hospital Management System"); break;
        }
    }
}

void HospitalWindow::wireSignals() {
    connect(navigationGroup_, &QButtonGroup::idClicked, this, [this](int id) {
        activatePage(static_cast<PageIndex>(id));
    });

    connect(themeToggleButton_, &QPushButton::clicked, this, [this]() {
        darkMode_ = !darkMode_;
        applyTheme();
    });



    connect(loginButton_, &QPushButton::clicked, this, [this]() {
        try {
            if (!selectedRole_.has_value()) {
                QMessageBox::warning(this, "Role Required", "Please select a role first.");
                activatePage(PageIndex::RoleSelection);
                return;
            }

            loggedInUser_ = authService_.loginWithRole(
                usernameInput_->text().trimmed().toStdString(),
                passwordInput_->text().toStdString(),
                selectedRole_.value());
            passwordInput_->clear();
            
            refreshSessionUi();
            refreshAllData();
            activatePage(PageIndex::Dashboard);
            statusBar()->showMessage("Login successful.", 3000);
        } catch (const std::exception& error) {
            showError(error);
        }
    });

    connect(passwordInput_, &QLineEdit::returnPressed, loginButton_, &QPushButton::click);
    connect(usernameInput_, &QLineEdit::textChanged, this, [this]() { updateLoginButton(); });
    connect(passwordInput_, &QLineEdit::textChanged, this, [this]() { updateLoginButton(); });
    connect(backToRoleSelectionButton_, &QPushButton::clicked, this, [this]() { onBackToRoleSelection(); });

    connect(logoutButton_, &QPushButton::clicked, this, [this]() {
        loggedInUser_.reset();
        selectedRole_ = std::nullopt;
        refreshSessionUi();
        activatePage(PageIndex::RoleSelection);
        statusBar()->showMessage("Logged out.", 2000);
    });

    connect(sidebarLogoutButton_, &QPushButton::clicked, logoutButton_, &QPushButton::click);
}

void HospitalWindow::refreshAllData() {
    refreshDashboardMetrics();
    refreshPatientData();
    refreshDoctorData();
    refreshAppointmentData();
    refreshBillData();
}

void HospitalWindow::refreshPatientData() {
    if (!loggedInUser_ || !patientsPage_) return;
    patientsPage_->refresh();
    refreshDashboardMetrics();
}

void HospitalWindow::refreshDoctorData() {
    if (!loggedInUser_ || !doctorsPage_) return;
    doctorsPage_->refresh();
    refreshDashboardMetrics();
}

void HospitalWindow::refreshAppointmentData() {
    if (!loggedInUser_ || !appointmentsPage_) return;
    appointmentsPage_->refresh();
    refreshDashboardMetrics();
}

void HospitalWindow::refreshBillData() {
    if (!loggedInUser_ || !billingPage_) return;
    billingPage_->refresh();
    refreshDashboardMetrics();
}

void HospitalWindow::refreshDashboardMetrics() {
    if (!loggedInUser_ || !dashboardPage_) return;
    
    std::map<std::string, int> metrics;
    try {
        metrics["patients"] = static_cast<int>(patientService_.getAllPatients(*loggedInUser_).size());
        metrics["doctors"] = static_cast<int>(doctorService_.getAllDoctors(*loggedInUser_).size());
        metrics["appointments"] = static_cast<int>(appointmentService_.getAllAppointments(*loggedInUser_).size());
        metrics["bills"] = static_cast<int>(billingService_.getAllBills(*loggedInUser_).size());
        dashboardPage_->updateMetrics(metrics);
    } catch (...) {}
}

void HospitalWindow::refreshSessionUi() {
    if (loggedInUser_) {
        sessionLabel_->setText(QString::fromStdString(loggedInUser_->getName()));
        
        dashboardPage_->setCurrentUser(*loggedInUser_);
        patientsPage_->setCurrentUser(*loggedInUser_);
        doctorsPage_->setCurrentUser(*loggedInUser_);
        appointmentsPage_->setCurrentUser(*loggedInUser_);
        billingPage_->setCurrentUser(*loggedInUser_);
        
        configureUIForRole(loggedInUser_->getRole());
    } else {
        sessionLabel_->setText("No active session");
    }
}

void HospitalWindow::configureUIForRole(core::UserRole role) {
    const bool isAdmin = (role == core::UserRole::Admin);
    const bool isReceptionist = (role == core::UserRole::Receptionist);
    const bool isDoctor = (role == core::UserRole::Doctor);
    const bool isPatient = (role == core::UserRole::Patient);
    
    patientsNavButton_->setVisible(isAdmin || isReceptionist);
    doctorsNavButton_->setVisible(true); // All can see doctors
    appointmentsNavButton_->setVisible(true); // All can see appointments (filtered)
    billingNavButton_->setVisible(isAdmin || isReceptionist || isPatient);
    prescriptionsNavButton_->setVisible(isDoctor || isPatient);

    dashboardPage_->setRole(role);
    patientsPage_->setRole(role);
    doctorsPage_->setRole(role);
    appointmentsPage_->setRole(role);
    billingPage_->setRole(role);
    if (prescriptionsPage_) prescriptionsPage_->setRole(role);
}

void HospitalWindow::updateLoginButton() {
    const bool hasCreds = !usernameInput_->text().trimmed().isEmpty() && !passwordInput_->text().isEmpty();
    loginButton_->setEnabled(hasCreds && selectedRole_.has_value());
}

void HospitalWindow::onRoleSelected(core::UserRole role) {
    selectedRole_ = role;
    selectedRoleLabel_->setText(QString("Login as %1").arg(QString::fromStdString(hms::core::toString(role))));
    updateLoginButton();
    activatePage(PageIndex::Login);
}

void HospitalWindow::onBackToRoleSelection() {
    selectedRole_ = std::nullopt;
    updateLoginButton();
    activatePage(PageIndex::RoleSelection);
}

void HospitalWindow::showError(const std::exception& error) {
    QMessageBox::critical(this, "Error", QString::fromStdString(error.what()));
}

}  // namespace hms::ui
