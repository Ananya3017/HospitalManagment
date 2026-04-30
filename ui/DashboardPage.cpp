#include "DashboardPage.h"
#include <memory>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

namespace hms::ui {

DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void DashboardPage::setRole(core::UserRole role) {
    currentRole_ = role;
    switch (role) {
        case core::UserRole::Patient:
            if (patientTitleLabel_) patientTitleLabel_->setText("MY PROFILE");
            if (doctorTitleLabel_) doctorTitleLabel_->setText("AVAILABLE DOCTORS");
            if (appointmentTitleLabel_) appointmentTitleLabel_->setText("MY APPOINTMENTS");
            if (billTitleLabel_) billTitleLabel_->setText("MY BILLS");
            break;
        case core::UserRole::Doctor:
            if (patientTitleLabel_) patientTitleLabel_->setText("MY PATIENTS");
            if (doctorTitleLabel_) doctorTitleLabel_->setText("SPECIALIZATIONS");
            if (appointmentTitleLabel_) appointmentTitleLabel_->setText("MY SCHEDULE");
            if (billTitleLabel_) billTitleLabel_->setText("---");
            break;
        case core::UserRole::Receptionist:
            if (patientTitleLabel_) patientTitleLabel_->setText("PATIENTS");
            if (doctorTitleLabel_) doctorTitleLabel_->setText("DOCTORS");
            if (appointmentTitleLabel_) appointmentTitleLabel_->setText("APPOINTMENTS");
            if (billTitleLabel_) billTitleLabel_->setText("BILLING");
            break;
        case core::UserRole::Admin:
            if (patientTitleLabel_) patientTitleLabel_->setText("TOTAL PATIENTS");
            if (doctorTitleLabel_) doctorTitleLabel_->setText("ACTIVE DOCTORS");
            if (appointmentTitleLabel_) appointmentTitleLabel_->setText("SYSTEM APPOINTMENTS");
            if (billTitleLabel_) billTitleLabel_->setText("REVENUE OVERVIEW");
            break;
    }
}

void DashboardPage::setCurrentUser(const core::User& user) {
    currentUser_ = std::make_unique<core::User>(user);
    welcomeLabel_->setText(QString("Welcome back, %1!").arg(QString::fromStdString(user.getName())));
}

void DashboardPage::updateMetrics(const std::map<std::string, int>& metrics) {
    if (metrics.count("patients")) patientCountLabel_->setText(QString::number(metrics.at("patients")));
    if (metrics.count("doctors")) doctorCountLabel_->setText(QString::number(metrics.at("doctors")));
    if (metrics.count("appointments")) appointmentCountLabel_->setText(QString::number(metrics.at("appointments")));
    if (metrics.count("bills")) billCountLabel_->setText(QString::number(metrics.at("bills")));
}

void DashboardPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);

    welcomeLabel_ = new QLabel("Welcome back!", this);
    welcomeLabel_->setObjectName("dashboardWelcome");
    welcomeLabel_->setStyleSheet("font-size: 32px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(welcomeLabel_);

    auto* gridLayout = new QGridLayout();
    gridLayout->setSpacing(20);

    auto createMetricCard = [&](const QString& title, QLabel*& titleLabel, QLabel*& countLabel, const QString& icon, const QString& page) {
        auto* card = new QFrame(this);
        card->setObjectName("metricCard");
        card->setStyleSheet("QFrame#metricCard { background: white; border-radius: 12px; border: 1px solid #e2e8f0; }");
        
        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(20, 20, 20, 20);
        
        titleLabel = new QLabel(title, card);
        titleLabel->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; text-transform: uppercase;");
        
        countLabel = new QLabel("0", card);
        countLabel->setStyleSheet("color: #0f172a; font-size: 36px; font-weight: 700;");
        
        auto* btn = new QPushButton("View Details →", card);
        btn->setStyleSheet("QPushButton { border: none; color: #2563eb; font-weight: 600; text-align: left; } QPushButton:hover { color: #1d4ed8; text-decoration: underline; }");
        connect(btn, &QPushButton::clicked, this, [this, page]() { emit navigateRequested(page); });

        layout->addWidget(titleLabel);
        layout->addWidget(countLabel);
        layout->addStretch();
        layout->addWidget(btn);
        
        return card;
    };

    gridLayout->addWidget(createMetricCard("Total Patients", patientTitleLabel_, patientCountLabel_, "", "Patients"), 0, 0);
    gridLayout->addWidget(createMetricCard("Active Doctors", doctorTitleLabel_, doctorCountLabel_, "", "Doctors"), 0, 1);
    gridLayout->addWidget(createMetricCard("Upcoming Appointments", appointmentTitleLabel_, appointmentCountLabel_, "", "Appointments"), 1, 0);
    gridLayout->addWidget(createMetricCard("Unpaid Invoices", billTitleLabel_, billCountLabel_, "", "Billing"), 1, 1);

    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
}

}  // namespace hms::ui
