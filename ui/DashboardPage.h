#pragma once

#include "../core/User.h"
#include <QWidget>
#include <memory>
#include <map>
#include <string>

class QLabel;
class QPushButton;

namespace hms::ui {

class DashboardPage final : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);

    void setRole(core::UserRole role);
    void setCurrentUser(const core::User& user);
    void updateMetrics(const std::map<std::string, int>& metrics);

signals:
    void navigateRequested(const QString& pageName);

private:
    void setupUi();
    
    std::unique_ptr<core::User> currentUser_ {};
    core::UserRole currentRole_ = core::UserRole::Patient;

    QLabel* welcomeLabel_ = nullptr;
    QLabel* patientTitleLabel_ = nullptr;
    QLabel* patientCountLabel_ = nullptr;
    QLabel* doctorTitleLabel_ = nullptr;
    QLabel* doctorCountLabel_ = nullptr;
    QLabel* appointmentTitleLabel_ = nullptr;
    QLabel* appointmentCountLabel_ = nullptr;
    QLabel* billTitleLabel_ = nullptr;
    QLabel* billCountLabel_ = nullptr;

    QPushButton* patientBtn_ = nullptr;
    QPushButton* doctorBtn_ = nullptr;
    QPushButton* appointmentBtn_ = nullptr;
    QPushButton* billingBtn_ = nullptr;
};

}  // namespace hms::ui
