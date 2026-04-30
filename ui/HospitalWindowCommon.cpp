#include "HospitalWindowView.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QIcon>
#include <QColor>

namespace hms::ui {

namespace {

QIcon drawSunIcon(const QColor& color) {
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawEllipse(QRectF(11, 11, 10, 10));
    for (int i = 0; i < 8; ++i) {
        painter.save();
        painter.translate(16, 16);
        painter.rotate(i * 45);
        painter.drawLine(0, -9, 0, -12);
        painter.restore();
    }
    return QIcon(pixmap);
}

QIcon drawMoonIcon(const QColor& color) {
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    QPainterPath path;
    path.addEllipse(QRectF(8, 8, 16, 16));
    QPainterPath sub;
    sub.addEllipse(QRectF(13, 5, 16, 16));
    painter.drawPath(path.subtracted(sub));
    return QIcon(pixmap);
}

QString lightStyleSheet() {
    return R"(
QMainWindow, QWidget#windowRoot { background: #f5f6fa; color: #2f3640; }
QFrame#sidebar { background: #2f3640; }
QLabel#brandBadge { background: #4CAF50; border-radius: 20px; color: white; font-size: 22px; font-weight: 700; min-width: 40px; max-width: 40px; min-height: 40px; max-height: 40px; qproperty-alignment: AlignCenter; }
QLabel#brandTitle { color: white; font-size: 20px; font-weight: 700; }
QLabel#brandSubtitle { color: rgba(255, 255, 255, 0.75); font-size: 11px; }
QPushButton#sidebarButton { background: transparent; border: none; border-radius: 14px; color: #dcdde1; font-size: 14px; font-weight: 600; padding: 12px 16px; text-align: left; }
QPushButton#sidebarButton:hover { background: rgba(255, 255, 255, 0.08); color: white; }
QPushButton#sidebarButton:checked { background: #4CAF50; color: white; }
QFrame#topBar, QFrame#pageCard, QFrame#metricCard { background: white; border: 1px solid #e5e9f2; border-radius: 20px; }
QWidget#roleSelectionPage { background-color: #f5f6fa; }
QLabel#roleWelcomeTitle { color: #2d3436; margin-bottom: 5px; }
QLabel#roleWelcomeSubtitle { color: #636e72; }
QLabel#roleWelcomeFooter { color: #b2bec3; font-size: 13px; }
QFrame#roleCard { background-color: white; border: 2px solid transparent; border-radius: 28px; }
QFrame#roleCard:hover { border-color: #0984e3; background-color: #f1f9ff; }
QLabel#roleCardTitle { color: #2d3436; }
QLabel#topBarTitle { color: #1f2933; font-size: 24px; font-weight: 700; }
QLabel#topBarSubtitle { color: #7f8c8d; font-size: 12px; }
QLabel#sessionLabel { color: #2f3640; font-size: 13px; font-weight: 600; }
QLabel#pageTitle { color: #1f2933; font-size: 24px; font-weight: 700; }
QLabel#pageSubtitle { color: #7f8c8d; font-size: 13px; }
QLabel#cardTitle { color: #1f2933; font-size: 16px; font-weight: 700; }
QLabel#cardSubtitle { color: #7f8c8d; font-size: 12px; }
QLabel#metricTitle { color: #7f8c8d; font-size: 12px; font-weight: 600; }
QLabel#metricValue { color: #2f3640; font-size: 28px; font-weight: 700; }
QLabel#metricMeta { color: #4CAF50; font-size: 12px; font-weight: 600; }
QLabel#sectionLabel { color: #5f6c7b; font-size: 12px; font-weight: 600; }
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit { background: white; border: 1px solid #dfe4ea; border-radius: 12px; color: #2f3640; padding: 8px 10px; min-height: 20px; }
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QDateTimeEdit:focus { border: 1px solid #4CAF50; }
QLineEdit::placeholder { color: #a4b0be; }
QPushButton#primaryButton, QPushButton#secondaryButton, QPushButton#dangerButton { border: none; border-radius: 12px; font-weight: 700; padding: 10px 14px; min-height: 20px; }
QPushButton#primaryButton { background: #4CAF50; color: white; }
QPushButton#primaryButton:hover { background: #43a047; }
QPushButton#secondaryButton { background: #edf2f7; color: #2f3640; }
QPushButton#secondaryButton:hover { background: #dfe6ec; }
QPushButton#dangerButton { background: #ff7675; color: white; }
QPushButton#dangerButton:hover { background: #e17055; }
QPushButton:disabled { background: #dfe6e9; color: #95a5a6; }
QTableView { background: white; alternate-background-color: #f8fafc; border: none; color: #2f3640; gridline-color: transparent; selection-background-color: rgba(76, 175, 80, 0.18); selection-color: #1f2933; }
QHeaderView::section { background: #f0f3f8; border: none; border-bottom: 1px solid #e5e9f2; color: #5f6c7b; font-size: 12px; font-weight: 700; padding: 10px; }
QStatusBar { background: transparent; color: #636e72; }
QPushButton#themeToggle { border-radius: 18px; background-color: #f1f2f6; }
QPushButton#themeToggle:hover { background-color: #dcdde1; }
)";
}

QString darkStyleSheet() {
    return R"(
QMainWindow, QWidget#windowRoot { background: #1e272e; color: #f5f6fa; }
QFrame#sidebar { background: #11161b; }
QLabel#brandBadge { background: #4CAF50; border-radius: 20px; color: white; font-size: 22px; font-weight: 700; min-width: 40px; max-width: 40px; min-height: 40px; max-height: 40px; qproperty-alignment: AlignCenter; }
QLabel#brandTitle { color: white; font-size: 20px; font-weight: 700; }
QLabel#brandSubtitle { color: rgba(255, 255, 255, 0.75); font-size: 11px; }
QPushButton#sidebarButton { background: transparent; border: none; border-radius: 14px; color: #d2dae2; font-size: 14px; font-weight: 600; padding: 12px 16px; text-align: left; }
QPushButton#sidebarButton:hover { background: rgba(255, 255, 255, 0.09); color: white; }
QPushButton#sidebarButton:checked { background: #4CAF50; color: white; }
QFrame#topBar, QFrame#pageCard, QFrame#metricCard { background: #2b343c; border: 1px solid #3b4650; border-radius: 20px; }
QWidget#roleSelectionPage { background-color: #1e272e; }
QLabel#roleWelcomeTitle { color: #f5f6fa; margin-bottom: 5px; }
QLabel#roleWelcomeSubtitle { color: #b2bec3; }
QLabel#roleWelcomeFooter { color: #7f8c8d; font-size: 13px; }
QFrame#roleCard { background-color: #2b343c; border: 2px solid #3b4650; border-radius: 28px; }
QFrame#roleCard:hover { border-color: #4CAF50; background-color: #323d47; }
QLabel#roleCardTitle { color: #f5f6fa; }
QLabel#topBarTitle, QLabel#pageTitle, QLabel#cardTitle, QLabel#metricValue, QLabel#sessionLabel { color: #f5f6fa; }
QLabel#topBarSubtitle, QLabel#pageSubtitle, QLabel#cardSubtitle, QLabel#metricTitle, QLabel#sectionLabel { color: #b2bec3; }
QLabel#metricMeta { color: #7bed9f; font-size: 12px; font-weight: 600; }
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit { background: #1f2930; border: 1px solid #3b4650; border-radius: 12px; color: #f5f6fa; padding: 8px 10px; min-height: 20px; }
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QDateTimeEdit:focus { border: 1px solid #4CAF50; }
QLineEdit::placeholder { color: #95a5a6; }
QPushButton#primaryButton, QPushButton#secondaryButton, QPushButton#dangerButton { border: none; border-radius: 12px; font-weight: 700; padding: 10px 14px; min-height: 20px; }
QPushButton#primaryButton { background: #4CAF50; color: white; }
QPushButton#primaryButton:hover { background: #43a047; }
QPushButton#secondaryButton { background: #3b4650; color: #f5f6fa; }
QPushButton#secondaryButton:hover { background: #46525d; }
QPushButton#dangerButton { background: #c0392b; color: white; }
QPushButton#dangerButton:hover { background: #962d22; }
QPushButton:disabled { background: #3b4650; color: #7f8c8d; }
QTableView { background: #232b33; alternate-background-color: #27313a; border: none; color: #ecf0f1; gridline-color: transparent; selection-background-color: rgba(76, 175, 80, 0.25); selection-color: #ecf0f1; }
QHeaderView::section { background: #202830; border: none; border-bottom: 1px solid #3b4650; color: #d2dae2; font-size: 12px; font-weight: 700; padding: 10px; }
QStatusBar { background: transparent; color: #d2dae2; }
QPushButton#themeToggle { border-radius: 18px; background-color: #2f3640; }
QPushButton#themeToggle:hover { background-color: #1f242b; }
)";
}

} // namespace

void HospitalWindow::applyTheme() {
    setStyleSheet(darkMode_ ? darkStyleSheet() : lightStyleSheet());
    if (themeToggleButton_) {
        if (darkMode_) {
            themeToggleButton_->setIcon(drawMoonIcon(QColor("#f5f6fa")));
        } else {
            themeToggleButton_->setIcon(drawSunIcon(QColor("#2f3640")));
        }
        themeToggleButton_->setIconSize(QSize(20, 20));
    }
}

} // namespace hms::ui
