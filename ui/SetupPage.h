#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

namespace hms::ui {

class SetupPage final : public QWidget {
    Q_OBJECT

public:
    explicit SetupPage(QWidget* parent = nullptr);

signals:
    void adminCreated(const QString& username, const QString& password);

private:
    QLineEdit* usernameInput_ = nullptr;
    QLineEdit* passwordInput_ = nullptr;
    QLineEdit* confirmPasswordInput_ = nullptr;
    QPushButton* createButton_ = nullptr;
    QLabel* errorLabel_ = nullptr;

    void setupUi();
    void wireSignals();
    void updateCreateButton();
};

}  // namespace hms::ui
