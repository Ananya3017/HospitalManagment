#pragma once

#include "../core/User.h"
#include "../services/DoctorService.h"
#include "../models/DoctorModel.h"
#include <QWidget>
#include <memory>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QTableView;
class QSortFilterProxyModel;

namespace hms::ui {

class DoctorsPage final : public QWidget {
    Q_OBJECT

public:
    explicit DoctorsPage(services::DoctorService& doctorService, QWidget* parent = nullptr);

    void setRole(core::UserRole role);
    void setCurrentUser(const core::User& user);
    void refresh();

signals:
    void doctorAdded();
    void doctorUpdated();
    void doctorDeleted();
    void errorOccurred(const std::string& message);

private:
    void setupUi();
    void wireSignals();
    void clearForm();
    void onSelectionChanged();
    void onSaveClicked();
    void onUpdateClicked();
    void onDeleteClicked();

    services::DoctorService& doctorService_;
    std::unique_ptr<core::User> currentUser_ {};
    core::UserRole currentRole_ = core::UserRole::Patient;

    models::DoctorModel* doctorModel_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;

    QLineEdit* searchInput_ = nullptr;
    QSpinBox* idInput_ = nullptr;
    QLineEdit* nameInput_ = nullptr;
    QLineEdit* specializationInput_ = nullptr;

    QPushButton* saveButton_ = nullptr;
    QPushButton* updateButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;

    QTableView* tableView_ = nullptr;
};

}  // namespace hms::ui
