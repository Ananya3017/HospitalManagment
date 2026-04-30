#pragma once

#include "../core/User.h"

#include <QWidget>
#include <QFrame>
#include <QIcon>

namespace hms::ui {

class RoleCard final : public QFrame {
    Q_OBJECT
public:
    RoleCard(const QString& title, const QIcon& icon, QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
};

class RoleSelectionPage final : public QWidget {
    Q_OBJECT

public:
    explicit RoleSelectionPage(QWidget* parent = nullptr);

signals:
    void roleSelected(core::UserRole role);

private:
    RoleCard* adminButton_ = nullptr;
    RoleCard* doctorButton_ = nullptr;
    RoleCard* receptionistButton_ = nullptr;
    RoleCard* patientButton_ = nullptr;

    void setupUi();
    void wireSignals();
};

}  // namespace hms::ui
