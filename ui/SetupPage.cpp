#include "SetupPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace hms::ui {

SetupPage::SetupPage(QWidget* parent) : QWidget(parent) {
    setupUi();
    wireSignals();
}

void SetupPage::setupUi() {
    setObjectName("setupPage");
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 40, 40, 40);
    root->setAlignment(Qt::AlignCenter);

    auto* card = new QFrame(this);
    card->setObjectName("pageCard");
    card->setFixedWidth(400);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    auto* title = new QLabel("System Setup", card);
    title->setObjectName("pageHeader");
    title->setAlignment(Qt::AlignCenter);
    
    auto* subtitle = new QLabel("Create the initial Administrator account to begin.", card);
    subtitle->setObjectName("pageSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);

    usernameInput_ = new QLineEdit(card);
    usernameInput_->setPlaceholderText("Admin Username");
    
    passwordInput_ = new QLineEdit(card);
    passwordInput_->setPlaceholderText("Admin Password");
    passwordInput_->setEchoMode(QLineEdit::Password);
    
    confirmPasswordInput_ = new QLineEdit(card);
    confirmPasswordInput_->setPlaceholderText("Confirm Password");
    confirmPasswordInput_->setEchoMode(QLineEdit::Password);

    errorLabel_ = new QLabel(card);
    errorLabel_->setStyleSheet("color: #e74c3c; font-size: 12px;");
    errorLabel_->setVisible(false);
    errorLabel_->setWordWrap(true);

    createButton_ = new QPushButton("Initialize System", card);
    createButton_->setObjectName("primaryButton");
    createButton_->setEnabled(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(10);
    layout->addWidget(new QLabel("Username:", card));
    layout->addWidget(usernameInput_);
    layout->addWidget(new QLabel("Password (min 8 characters):", card));
    layout->addWidget(passwordInput_);
    layout->addWidget(confirmPasswordInput_);
    layout->addWidget(errorLabel_);
    layout->addSpacing(10);
    layout->addWidget(createButton_);

    root->addWidget(card);
}

void SetupPage::wireSignals() {
    auto checkInputs = [this]() { updateCreateButton(); };
    connect(usernameInput_, &QLineEdit::textChanged, checkInputs);
    connect(passwordInput_, &QLineEdit::textChanged, checkInputs);
    connect(confirmPasswordInput_, &QLineEdit::textChanged, checkInputs);

    connect(createButton_, &QPushButton::clicked, this, [this]() {
        if (passwordInput_->text() != confirmPasswordInput_->text()) {
            errorLabel_->setText("Passwords do not match.");
            errorLabel_->setVisible(true);
            return;
        }
        if (passwordInput_->text().length() < 8) {
            errorLabel_->setText("Password must be at least 8 characters.");
            errorLabel_->setVisible(true);
            return;
        }
        emit adminCreated(usernameInput_->text(), passwordInput_->text());
    });
}

void SetupPage::updateCreateButton() {
    const bool valid = !usernameInput_->text().isEmpty() && 
                       passwordInput_->text().length() >= 8 &&
                       !confirmPasswordInput_->text().isEmpty();
    createButton_->setEnabled(valid);
}

}  // namespace hms::ui
