#include "RoleSelectionPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QFont>

namespace hms::ui {

RoleCard::RoleCard(const QString& title, const QIcon& icon, QWidget* parent)
    : QFrame(parent) {
    setCursor(Qt::PointingHandCursor);
    setFixedSize(240, 200);
    setObjectName("roleCard");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 24, 20, 24);
    layout->setSpacing(16);
    layout->setAlignment(Qt::AlignCenter);

    auto* iconLabel = new QLabel(this);
    iconLabel->setPixmap(icon.pixmap(64, 64));
    iconLabel->setAlignment(Qt::AlignCenter);

    auto* titleLabel = new QLabel(title, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("roleCardTitle");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setXOffset(0);
    shadow->setYOffset(6);
    shadow->setColor(QColor(0, 0, 0, 25));
    setGraphicsEffect(shadow);
}

void RoleCard::mouseReleaseEvent(QMouseEvent* event) {
    if (rect().contains(event->pos())) {
        emit clicked();
    }
    QFrame::mouseReleaseEvent(event);
}

RoleSelectionPage::RoleSelectionPage(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    wireSignals();
}

void RoleSelectionPage::setupUi() {
    setObjectName("roleSelectionPage");
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignCenter);

    auto* contentContainer = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setContentsMargins(60, 40, 60, 40);
    contentLayout->setSpacing(50);
    contentLayout->setAlignment(Qt::AlignCenter);

    auto* header = new QWidget(contentContainer);
    auto* headerLayout = new QVBoxLayout(header);
    headerLayout->setSpacing(12);
    headerLayout->setAlignment(Qt::AlignCenter);

    auto* titleLabel = new QLabel("Welcome", header);
    titleLabel->setObjectName("roleWelcomeTitle");
    QFont titleFont;
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    auto* subtitleLabel = new QLabel("Select your account type to continue", header);
    subtitleLabel->setObjectName("roleWelcomeSubtitle");
    QFont subtitleFont;
    subtitleFont.setPointSize(14);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(32);
    cardsLayout->setAlignment(Qt::AlignCenter);

    patientButton_ = new RoleCard("Patient", style()->standardIcon(QStyle::SP_DirHomeIcon), contentContainer);
    doctorButton_ = new RoleCard("Doctor", style()->standardIcon(QStyle::SP_FileDialogContentsView), contentContainer);
    receptionistButton_ = new RoleCard("Receptionist", style()->standardIcon(QStyle::SP_FileDialogDetailedView), contentContainer);
    adminButton_ = new RoleCard("Admin", style()->standardIcon(QStyle::SP_ComputerIcon), contentContainer);

    cardsLayout->addWidget(patientButton_);
    cardsLayout->addWidget(doctorButton_);
    cardsLayout->addWidget(receptionistButton_);
    cardsLayout->addWidget(adminButton_);

    auto* footerLabel = new QLabel("Choose the role that best describes your relationship with the hospital.", contentContainer);
    footerLabel->setObjectName("roleWelcomeFooter");
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setWordWrap(true);
    footerLabel->setMaximumWidth(600);

    contentLayout->addWidget(header);
    contentLayout->addLayout(cardsLayout);
    contentLayout->addWidget(footerLabel);

    mainLayout->addStretch(1);
    mainLayout->addWidget(contentContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);
}

void RoleSelectionPage::wireSignals() {
    connect(adminButton_, &RoleCard::clicked, this, [this]() {
        emit roleSelected(core::UserRole::Admin);
    });

    connect(doctorButton_, &RoleCard::clicked, this, [this]() {
        emit roleSelected(core::UserRole::Doctor);
    });

    connect(receptionistButton_, &RoleCard::clicked, this, [this]() {
        emit roleSelected(core::UserRole::Receptionist);
    });

    connect(patientButton_, &RoleCard::clicked, this, [this]() {
        emit roleSelected(core::UserRole::Patient);
    });
}

}  // namespace hms::ui
