#include "HospitalWindowView.h"
#include "RoleSelectionPage.h"
#include "SetupPage.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>
#include <QFrame>

namespace hms::ui {

QWidget* HospitalWindow::buildSidebar(QWidget* parent) {
    auto* sidebar = new QFrame(parent);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(240);

    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(18, 20, 18, 20);
    layout->setSpacing(8);

    auto* brandRow = new QHBoxLayout();
    brandRow->setSpacing(12);

    auto* brandBadge = new QLabel("H", sidebar);
    brandBadge->setObjectName("brandBadge");

    auto* brandText = new QWidget(sidebar);
    auto* brandTextLayout = new QVBoxLayout(brandText);
    brandTextLayout->setContentsMargins(0, 0, 0, 0);
    brandTextLayout->setSpacing(2);

    auto* brandTitle = new QLabel("NICE HOSPITAL", brandText);
    brandTitle->setObjectName("brandTitle");
    auto* brandSubtitle = new QLabel("Hospital Management", brandText);
    brandSubtitle->setObjectName("brandSubtitle");

    brandTextLayout->addWidget(brandTitle);
    brandTextLayout->addWidget(brandSubtitle);

    brandRow->addWidget(brandBadge);
    brandRow->addWidget(brandText, 1);

    navigationGroup_ = new QButtonGroup(sidebar);
    navigationGroup_->setExclusive(true);

    dashboardNavButton_ = createSidebarButton("Dashboard", style()->standardIcon(QStyle::SP_DesktopIcon), PageIndex::Dashboard, sidebar);
    patientsNavButton_ = createSidebarButton("Patients", style()->standardIcon(QStyle::SP_FileDialogContentsView), PageIndex::Patients, sidebar);
    doctorsNavButton_ = createSidebarButton("Doctors", style()->standardIcon(QStyle::SP_FileDialogDetailedView), PageIndex::Doctors, sidebar);
    appointmentsNavButton_ = createSidebarButton("Appointments", style()->standardIcon(QStyle::SP_FileDialogListView), PageIndex::Appointments, sidebar);
    billingNavButton_ = createSidebarButton("Billing", style()->standardIcon(QStyle::SP_DriveHDIcon), PageIndex::Billing, sidebar);
    prescriptionsNavButton_ = createSidebarButton("Prescriptions", style()->standardIcon(QStyle::SP_FileIcon), PageIndex::Prescriptions, sidebar);
    sidebarLogoutButton_ = createSidebarButton("Logout", style()->standardIcon(QStyle::SP_BrowserStop), PageIndex::RoleSelection, sidebar);
    sidebarLogoutButton_->setObjectName("dangerButton");

    layout->addLayout(brandRow);
    layout->addSpacing(18);
    layout->addWidget(dashboardNavButton_);
    layout->addWidget(patientsNavButton_);
    layout->addWidget(doctorsNavButton_);
    layout->addWidget(appointmentsNavButton_);
    layout->addWidget(billingNavButton_);
    layout->addWidget(prescriptionsNavButton_);
    layout->addStretch(1);
    layout->addWidget(sidebarLogoutButton_);

    return sidebar;
}

QWidget* HospitalWindow::buildTopBar(QWidget* parent) {
    auto* topBar = new QFrame(parent);
    topBar->setObjectName("topBar");

    auto* layout = new QVBoxLayout(topBar);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto* row1 = new QHBoxLayout();
    pageTitleLabel_ = new QLabel("Dashboard", topBar);
    pageTitleLabel_->setObjectName("topBarTitle");
    
    themeToggleButton_ = new QPushButton(topBar);
    themeToggleButton_->setObjectName("themeToggle");
    themeToggleButton_->setFixedSize(36, 36);

    row1->addWidget(pageTitleLabel_);
    row1->addStretch(1);
    row1->addWidget(themeToggleButton_);

    auto* row2 = new QHBoxLayout();
    auto* userStack = new QVBoxLayout();
    sessionLabel_ = new QLabel("Welcome", topBar);
    sessionLabel_->setObjectName("sessionLabel");
    userStack->addWidget(sessionLabel_);
    
    logoutButton_ = createActionButton("Logout", "secondaryButton", topBar);

    row2->addLayout(userStack, 1);
    row2->addWidget(logoutButton_);

    layout->addLayout(row1);
    layout->addLayout(row2);
    return topBar;
}

QWidget* HospitalWindow::buildSetupPage(QWidget* parent) {
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);
    layout->addWidget(new SetupPage(page), 0, Qt::AlignCenter);
    return page;
}

QWidget* HospitalWindow::buildRoleSelectionPage(QWidget* parent) {
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);
    auto* roleSelection = new RoleSelectionPage(page);
    connect(roleSelection, &RoleSelectionPage::roleSelected, this, &HospitalWindow::onRoleSelected);
    layout->addWidget(roleSelection, 0, Qt::AlignCenter);
    return page;
}

QWidget* HospitalWindow::buildLoginPage(QWidget* parent) {
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    layout->addStretch(1);
    auto* card = new QFrame(page);
    card->setObjectName("pageCard");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(15);

    selectedRoleLabel_ = new QLabel("Login", card);
    selectedRoleLabel_->setObjectName("pageSubtitle");
    
    usernameInput_ = new QLineEdit(card);
    usernameInput_->setPlaceholderText("Username");
    passwordInput_ = new QLineEdit(card);
    passwordInput_->setPlaceholderText("Password");
    passwordInput_->setEchoMode(QLineEdit::Password);
    
    loginButton_ = createActionButton("Login", "primaryButton", card);
    backToRoleSelectionButton_ = new QPushButton("Back to Role Selection", card);
    backToRoleSelectionButton_->setObjectName("secondaryButton");

    cardLayout->addWidget(selectedRoleLabel_);
    cardLayout->addWidget(usernameInput_);
    cardLayout->addWidget(passwordInput_);
    cardLayout->addWidget(loginButton_);
    cardLayout->addWidget(backToRoleSelectionButton_);
    
    layout->addWidget(card, 0, Qt::AlignCenter);
    layout->addStretch(1);
    return page;
}

QWidget* HospitalWindow::buildDashboardPage(QWidget* parent) {
    dashboardPage_ = new DashboardPage(parent);
    connect(dashboardPage_, &DashboardPage::navigateRequested, this, [this](const QString& page) {
        if (page == "Patients") activatePage(PageIndex::Patients);
        else if (page == "Doctors") activatePage(PageIndex::Doctors);
        else if (page == "Appointments") activatePage(PageIndex::Appointments);
        else if (page == "Billing") activatePage(PageIndex::Billing);
    });
    return dashboardPage_;
}

QWidget* HospitalWindow::buildPatientsPage(QWidget* parent) {
    patientsPage_ = new PatientsPage(patientService_, parent);
    connect(patientsPage_, &PatientsPage::patientAdded, this, &HospitalWindow::refreshPatientData);
    connect(patientsPage_, &PatientsPage::patientUpdated, this, &HospitalWindow::refreshPatientData);
    connect(patientsPage_, &PatientsPage::patientDeleted, this, &HospitalWindow::refreshPatientData);
    connect(patientsPage_, &PatientsPage::errorOccurred, this, [this](const std::string& message) {
        QMessageBox::warning(this, "Patient Error", QString::fromStdString(message));
    });
    return patientsPage_;
}

QWidget* HospitalWindow::buildDoctorsPage(QWidget* parent) {
    doctorsPage_ = new DoctorsPage(doctorService_, parent);
    connect(doctorsPage_, &DoctorsPage::doctorAdded, this, &HospitalWindow::refreshDoctorData);
    connect(doctorsPage_, &DoctorsPage::doctorUpdated, this, &HospitalWindow::refreshDoctorData);
    connect(doctorsPage_, &DoctorsPage::doctorDeleted, this, &HospitalWindow::refreshDoctorData);
    return doctorsPage_;
}

QWidget* HospitalWindow::buildAppointmentsPage(QWidget* parent) {
    appointmentsPage_ = new AppointmentsPage(appointmentService_, patientService_, doctorService_, parent);
    connect(appointmentsPage_, &AppointmentsPage::appointmentSaved, this, &HospitalWindow::refreshAppointmentData);
    connect(appointmentsPage_, &AppointmentsPage::appointmentDeleted, this, &HospitalWindow::refreshAppointmentData);
    connect(appointmentsPage_, &AppointmentsPage::errorOccurred, this, [this](const std::string& message) {
        QMessageBox::warning(this, "Appointment Error", QString::fromStdString(message));
    });
    connect(appointmentsPage_, &AppointmentsPage::registerPatientRequested, this, [this]() {
        activatePage(PageIndex::Patients);
    });
    return appointmentsPage_;
}

QWidget* HospitalWindow::buildBillsPage(QWidget* parent) {
    billingPage_ = new BillingPage(billingService_, appointmentService_, parent);
    connect(billingPage_, &BillingPage::billSaved, this, &HospitalWindow::refreshBillData);
    connect(billingPage_, &BillingPage::billDeleted, this, &HospitalWindow::refreshBillData);
    return billingPage_;
}

void HospitalWindow::setupUi() {
    setWindowTitle("Hospital Management System");
    resize(1200, 800);

    auto* root = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(root);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    sidebar_ = buildSidebar(root);
    mainLayout->addWidget(sidebar_);

    auto* contentBox = new QWidget(root);
    auto* contentLayout = new QVBoxLayout(contentBox);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    topBar_ = buildTopBar(contentBox);
    contentLayout->addWidget(topBar_);

    pageStack_ = new QStackedWidget(contentBox);
    pageStack_->addWidget(buildSetupPage(pageStack_));           // index 0
    pageStack_->addWidget(buildRoleSelectionPage(pageStack_));   // index 1
    pageStack_->addWidget(buildLoginPage(pageStack_));           // index 2
    pageStack_->addWidget(buildDashboardPage(pageStack_));       // index 3
    pageStack_->addWidget(buildPatientsPage(pageStack_));        // index 4
    pageStack_->addWidget(buildDoctorsPage(pageStack_));         // index 5
    pageStack_->addWidget(buildAppointmentsPage(pageStack_));    // index 6
    pageStack_->addWidget(buildBillsPage(pageStack_));          // index 7
    prescriptionsPage_ = new PrescriptionPage(pageStack_);
    pageStack_->addWidget(prescriptionsPage_);                  // index 8

    contentLayout->addWidget(pageStack_, 1);
    mainLayout->addWidget(contentBox, 1);

    setCentralWidget(root);
}

QPushButton* HospitalWindow::createActionButton(const QString& text, const QString& role, QWidget* parent) {
    auto* btn = new QPushButton(text, parent);
    btn->setObjectName(role);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QPushButton* HospitalWindow::createSidebarButton(const QString& text, const QIcon& icon, PageIndex pageIndex, QWidget* parent) {
    auto* btn = new QPushButton(icon, text, parent);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    navigationGroup_->addButton(btn, static_cast<int>(pageIndex));
    return btn;
}

}  // namespace hms::ui
