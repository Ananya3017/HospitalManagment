#pragma once

#include <QMainWindow>
#include <memory>
#include <optional>
#include <string>

#include "../core/User.h"
#include "../services/AppointmentService.h"
#include "../services/AuthService.h"
#include "../services/BillingService.h"
#include "../services/DoctorService.h"
#include "../services/PatientService.h"

#include "SetupPage.h"
#include "DashboardPage.h"
#include "PatientsPage.h"
#include "DoctorsPage.h"
#include "AppointmentsPage.h"
#include "BillingPage.h"
#include "PrescriptionPage.h"

class QButtonGroup;
class QComboBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QSpinBox;
class QStackedWidget;
class QStandardItemModel;
class QTableView;
class QWidget;

namespace hms::ui {

class HospitalWindow final : public QMainWindow {
public:
    HospitalWindow(services::PatientService& patientService,
                   services::DoctorService& doctorService,
                   services::AppointmentService& appointmentService,
                   services::BillingService& billingService,
                   services::AuthService& authService,
                   QWidget* parent = nullptr);

private:
    enum class PageIndex {
        Setup = 0,
        RoleSelection = 1,
        Login = 2,
        Dashboard = 3,
        Patients = 4,
        Doctors = 5,
        Appointments = 6,
        Billing = 7,
        Prescriptions = 8
    };

    services::PatientService& patientService_;
    services::DoctorService& doctorService_;
    services::AppointmentService& appointmentService_;
    services::BillingService& billingService_;
    services::AuthService& authService_;

    bool darkMode_ = false;
    std::unique_ptr<core::User> loggedInUser_;

    QButtonGroup* navigationGroup_ = nullptr;
    QStackedWidget* pageStack_ = nullptr;
    QWidget* sidebar_ = nullptr;
    QWidget* topBar_ = nullptr;

    QPushButton* dashboardNavButton_ = nullptr;
    QPushButton* patientsNavButton_ = nullptr;
    QPushButton* doctorsNavButton_ = nullptr;
    QPushButton* appointmentsNavButton_ = nullptr;
    QPushButton* billingNavButton_ = nullptr;
    QPushButton* prescriptionsNavButton_ = nullptr;

    DashboardPage* dashboardPage_ = nullptr;

    QLabel* sessionLabel_ = nullptr;
    QLabel* pageTitleLabel_ = nullptr;
    QLabel* selectedRoleLabel_ = nullptr;
    QLineEdit* usernameInput_ = nullptr;
    QLineEdit* passwordInput_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QPushButton* logoutButton_ = nullptr;
    QPushButton* sidebarLogoutButton_ = nullptr;
    QPushButton* themeToggleButton_ = nullptr;
    QPushButton* dashboardPrimaryAction_ = nullptr;
    QPushButton* dashboardSecondaryAction_ = nullptr;
    QPushButton* backToRoleSelectionButton_ = nullptr;
    std::optional<core::UserRole> selectedRole_;

    PatientsPage* patientsPage_ = nullptr;

    DoctorsPage* doctorsPage_ = nullptr;

    AppointmentsPage* appointmentsPage_ = nullptr;

    BillingPage* billingPage_ = nullptr;
    PrescriptionPage* prescriptionsPage_ = nullptr;

    [[nodiscard]] bool isLoggedIn() const { return static_cast<bool>(loggedInUser_); }
    [[nodiscard]] bool canManagePatients() const;
    [[nodiscard]] bool canManageDoctors() const;
    [[nodiscard]] bool canManageAppointments() const;
    [[nodiscard]] bool canManageBilling() const;

    void applyTheme();
    void setupUi();
    void wireSignals();
    void activatePage(PageIndex pageIndex);
    void configureUIForRole(core::UserRole role);
    void refreshAllData();
    void refreshSessionUi();
    void refreshPatientData();
    void refreshDoctorData();
    void refreshAppointmentData();
    void refreshBillData();
    void refreshDashboardMetrics();
    void updateLoginButton();
    void onRoleSelected(core::UserRole role);
    void onBackToRoleSelection();
    void showError(const std::exception& error);

    [[nodiscard]] QWidget* buildSidebar(QWidget* parent);
    [[nodiscard]] QWidget* buildTopBar(QWidget* parent);
    [[nodiscard]] QWidget* buildSetupPage(QWidget* parent);
    [[nodiscard]] QWidget* buildRoleSelectionPage(QWidget* parent);
    [[nodiscard]] QWidget* buildLoginPage(QWidget* parent);
    [[nodiscard]] QWidget* buildDashboardPage(QWidget* parent);
    [[nodiscard]] QWidget* buildPatientsPage(QWidget* parent);
    [[nodiscard]] QWidget* buildDoctorsPage(QWidget* parent);
    [[nodiscard]] QWidget* buildAppointmentsPage(QWidget* parent);
    [[nodiscard]] QWidget* buildBillsPage(QWidget* parent);

    [[nodiscard]] QPushButton* createActionButton(const QString& text, const QString& role, QWidget* parent);
    [[nodiscard]] QPushButton* createSidebarButton(const QString& text,
                                                   const QIcon& icon,
                                                   PageIndex pageIndex,
                                                   QWidget* parent);

    [[nodiscard]] static int selectedId(const QComboBox* combo);
    [[nodiscard]] static int selectedTableId(const QTableView* table);
    [[nodiscard]] static int nextIdFromModel(const QStandardItemModel* model);
    [[nodiscard]] static QString tableText(const QTableView* table, int row, int column);
    [[nodiscard]] static core::AppointmentStatus statusFromString(const QString& statusText);
    static void setSelectedId(QComboBox* combo, int id);
};

}  // namespace hms::ui
