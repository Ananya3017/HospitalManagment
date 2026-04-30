#pragma once

#include "../core/Billing.h"
#include <QAbstractTableModel>
#include <vector>

namespace hms::ui::models {

class BillModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Id = 0,
        AppointmentId,
        Amount,
        Paid,
        ColumnCount
    };

    explicit BillModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setBills(std::vector<hms::core::Billing> bills);
    [[nodiscard]] const hms::core::Billing& billAt(int row) const;

private:
    std::vector<hms::core::Billing> bills_;
};

}  // namespace hms::ui::models
