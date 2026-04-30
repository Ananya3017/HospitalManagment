#include "DoctorModel.h"

#include <QString>

namespace hms::ui::models {

DoctorModel::DoctorModel(QObject* parent) : QAbstractTableModel(parent) {}

int DoctorModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(doctors_.size());
}

int DoctorModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant DoctorModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(doctors_.size())) {
        return {};
    }

    const auto& doctor = doctors_[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Id: return doctor.getId();
            case Name: return QString::fromStdString(doctor.getName());
            case Specialization: return QString::fromStdString(doctor.getSpecialization());
            default: break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == Id) {
            return static_cast<int>(Qt::AlignCenter);
        }
    } else if (role == Qt::UserRole) {
        return doctor.getId();
    }

    return {};
}

QVariant DoctorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Id: return "ID";
            case Name: return "Name";
            case Specialization: return "Specialization";
            default: break;
        }
    }
    return {};
}

void DoctorModel::setDoctors(std::vector<hms::core::Doctor> doctors) {
    beginResetModel();
    doctors_ = std::move(doctors);
    endResetModel();
}

const hms::core::Doctor& DoctorModel::doctorAt(int row) const {
    return doctors_.at(static_cast<size_t>(row));
}

}  // namespace hms::ui::models
