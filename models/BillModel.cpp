#include "BillModel.h"

#include <QString>

namespace hms::ui::models {

BillModel::BillModel(QObject* parent) : QAbstractTableModel(parent) {}

int BillModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(bills_.size());
}

int BillModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant BillModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(bills_.size())) {
        return {};
    }

    const auto& bill = bills_[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Id: return bill.getId();
            case AppointmentId: return bill.getAppointmentId();
            case Amount: return QString::number(bill.getAmount(), 'f', 2);
            case Paid: return bill.isPaid() ? "Yes" : "No";
            default: break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == Id || index.column() == AppointmentId || index.column() == Paid) {
            return static_cast<int>(Qt::AlignCenter);
        } else if (index.column() == Amount) {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    } else if (role == Qt::UserRole) {
        return bill.getId();
    }

    return {};
}

QVariant BillModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Id: return "Bill ID";
            case AppointmentId: return "Appt ID";
            case Amount: return "Amount ($)";
            case Paid: return "Paid Status";
            default: break;
        }
    }
    return {};
}

void BillModel::setBills(std::vector<hms::core::Billing> bills) {
    beginResetModel();
    bills_ = std::move(bills);
    endResetModel();
}

const hms::core::Billing& BillModel::billAt(int row) const {
    return bills_.at(static_cast<size_t>(row));
}

}  // namespace hms::ui::models
