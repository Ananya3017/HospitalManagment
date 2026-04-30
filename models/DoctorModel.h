#pragma once

#include "../core/Doctor.h"
#include <QAbstractTableModel>
#include <vector>

namespace hms::ui::models {

class DoctorModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Id = 0,
        Name,
        Specialization,
        Contact,
        ColumnCount
    };

    explicit DoctorModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setDoctors(std::vector<hms::core::Doctor> doctors);
    [[nodiscard]] const hms::core::Doctor& doctorAt(int row) const;

private:
    std::vector<hms::core::Doctor> doctors_;
};

}  // namespace hms::ui::models
