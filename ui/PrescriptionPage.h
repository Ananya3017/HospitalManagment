#pragma once

#include "../core/User.h"
#include <QWidget>
#include <memory>

class QLabel;
class QTableView;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QStandardItemModel;

namespace hms::ui {

class PrescriptionPage final : public QWidget {
    Q_OBJECT

public:
    explicit PrescriptionPage(QWidget* parent = nullptr);

    void setRole(core::UserRole role);
    void setCurrentUser(const core::User& user);
    void refresh();

signals:
    void errorOccurred(const std::string& message);
    void prescriptionSaved();

private:
    void setupUi();
    void wireSignals();

    std::unique_ptr<core::User> currentUser_ {};
    core::UserRole currentRole_ = core::UserRole::Doctor;

    QLineEdit* patientIdInput_ = nullptr;
    QLineEdit* diagnosisInput_ = nullptr;
    QTextEdit* medicinesInput_ = nullptr;
    QPushButton* saveButton_ = nullptr;
    QTableView* tableView_ = nullptr;
    QStandardItemModel* prescriptionModel_ = nullptr;
};

} // namespace hms::ui
