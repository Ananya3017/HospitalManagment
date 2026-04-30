#include "StatusDelegate.h"
#include <QApplication>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPainter>

namespace hms::ui {

StatusDelegate::StatusDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void StatusDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QString status = index.data().toString();
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Draw background highlight etc if selected
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QColor bgColor;
    QColor textColor = Qt::white;

    if (status == "Pending" || status == "Scheduled") {
        bgColor = QColor("#f59e0b"); // Yellow
    } else if (status == "Confirmed") {
        bgColor = QColor("#3b82f6"); // Blue
    } else if (status == "Completed") {
        bgColor = QColor("#22c55e"); // Green
    } else if (status == "Cancelled") {
        bgColor = QColor("#ef4444"); // Red
    } else {
        QStyledItemDelegate::paint(painter, option, index);
        painter->restore();
        return;
    }

    // Badge sizing
    QRect badgeRect = option.rect.adjusted(10, 8, -10, -8);
    
    // Draw rounded rect
    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->drawRoundedRect(badgeRect, 10, 10);

    // Draw text
    painter->setPen(textColor);
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(9);
    painter->setFont(font);
    painter->drawText(badgeRect, Qt::AlignCenter, status);

    painter->restore();
}

QSize StatusDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    return QStyledItemDelegate::sizeHint(option, index).expandedTo(QSize(100, 36));
}

} // namespace hms::ui
