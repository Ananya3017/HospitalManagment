#include "PatientModel.h"

#include <QString>

namespace hms::ui::models {

PatientModel::PatientModel(QObject* parent) : QAbstractTableModel(parent) {}

int PatientModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(patients_.size());
}

int PatientModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant PatientModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(patients_.size())) {
        return {};
    }

    const auto& patient = patients_[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Id: return patient.getId();
            case Name: return QString::fromStdString(patient.getName());
            case Age: return patient.getAge();
            case Contact: return QString::fromStdString(patient.getContactNumber());
            default: break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == Id || index.column() == Age) {
            return static_cast<int>(Qt::AlignCenter);
        }
    } else if (role == Qt::UserRole) {
        // Return raw ID for sorting or selection tracking
        return patient.getId();
    }

    return {};
}

QVariant PatientModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Id: return "ID";
            case Name: return "Name";
            case Age: return "Age";
            case Contact: return "Contact";
            default: break;
        }
    }
    return {};
}

void PatientModel::setPatients(std::vector<hms::core::Patient> patients) {
    beginResetModel();
    patients_ = std::move(patients);
    endResetModel();
}

const hms::core::Patient& PatientModel::patientAt(int row) const {
    return patients_.at(static_cast<size_t>(row));
}

}  // namespace hms::ui::models
