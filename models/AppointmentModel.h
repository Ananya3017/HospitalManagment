#pragma once

#include "../core/Appointment.h"
#include <QAbstractTableModel>
#include <vector>
#include <map>
#include <string>

namespace hms::ui::models {

class AppointmentModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Id = 0,
        Patient,
        Doctor,
        DateTime,
        Status,
        ColumnCount
    };

    explicit AppointmentModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setAppointments(std::vector<hms::core::Appointment> appointments);
    void setPatientMap(std::map<int, std::string> patientMap);
    void setDoctorMap(std::map<int, std::string> doctorMap);
    
    [[nodiscard]] const hms::core::Appointment& appointmentAt(int row) const;

private:
    std::vector<hms::core::Appointment> appointments_;
    std::map<int, std::string> patientMap_;
    std::map<int, std::string> doctorMap_;
};

}  // namespace hms::ui::models
