#pragma once

#include <QStyledItemDelegate>
#include <QPainter>

namespace hms::ui {

class StatusDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit StatusDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

} // namespace hms::ui
