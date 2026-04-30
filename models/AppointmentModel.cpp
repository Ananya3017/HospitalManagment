#include "AppointmentModel.h"

#include <QString>
#include <QBrush>
#include <QColor>
#include <QDateTime>

namespace hms::ui::models {

AppointmentModel::AppointmentModel(QObject* parent) : QAbstractTableModel(parent) {}

int AppointmentModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(appointments_.size());
}

int AppointmentModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant AppointmentModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(appointments_.size())) {
        return {};
    }

    const auto& appt = appointments_[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Id: return appt.getId();
            case Patient: {
                auto it = patientMap_.find(appt.getPatientId());
                return (it != patientMap_.end()) ? QString::fromStdString(it->second) : QString::number(appt.getPatientId());
            }
            case Doctor: {
                auto it = doctorMap_.find(appt.getDoctorId());
                return (it != doctorMap_.end()) ? QString::fromStdString(it->second) : QString::number(appt.getDoctorId());
            }
            case DateTime: return QString::fromStdString(appt.getDateTime());
            case Status: return QString::fromStdString(core::toString(appt.getStatus()));
            default: break;
        }
    } else if (role == Qt::ForegroundRole && index.column() == Status) {
        switch (appt.getStatus()) {
            case core::AppointmentStatus::Scheduled: return QBrush(QColor("#0f766e"));
            case core::AppointmentStatus::Completed: return QBrush(QColor("#1d4ed8"));
            case core::AppointmentStatus::Cancelled: return QBrush(QColor("#dc2626"));
        }
    } else if (role == Qt::BackgroundRole) {
        const QDateTime apptDate = QDateTime::fromString(QString::fromStdString(appt.getDateTime()), "yyyy-MM-dd HH:mm");
        if (apptDate.isValid() && apptDate.date() == QDate::currentDate()) {
            return QBrush(QColor(220, 245, 229)); // Today's appointments highlight
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == Id || index.column() == DateTime || index.column() == Status) {
            return static_cast<int>(Qt::AlignCenter);
        }
    } else if (role == Qt::UserRole) {
        return appt.getId();
    }

    return {};
}

QVariant AppointmentModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Id: return "ID";
            case Patient: return "Patient";
            case Doctor: return "Doctor";
            case DateTime: return "Date & Time";
            case Status: return "Status";
            default: break;
        }
    }
    return {};
}

void AppointmentModel::setAppointments(std::vector<hms::core::Appointment> appointments) {
    beginResetModel();
    appointments_ = std::move(appointments);
    endResetModel();
}

void AppointmentModel::setPatientMap(std::map<int, std::string> patientMap) {
    beginResetModel();
    patientMap_ = std::move(patientMap);
    endResetModel();
}

void AppointmentModel::setDoctorMap(std::map<int, std::string> doctorMap) {
    beginResetModel();
    doctorMap_ = std::move(doctorMap);
    endResetModel();
}

const hms::core::Appointment& AppointmentModel::appointmentAt(int row) const {
    return appointments_.at(static_cast<size_t>(row));
}

}  // namespace hms::ui::models
