#pragma once

#include "../core/Patient.h"
#include <QAbstractTableModel>
#include <vector>

namespace hms::ui::models {

class PatientModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Id = 0,
        Name,
        Age,
        Contact,
        ColumnCount
    };

    explicit PatientModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setPatients(std::vector<hms::core::Patient> patients);
    [[nodiscard]] const hms::core::Patient& patientAt(int row) const;

private:
    std::vector<hms::core::Patient> patients_;
};

}  // namespace hms::ui::models
