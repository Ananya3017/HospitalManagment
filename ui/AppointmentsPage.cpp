#include "AppointmentsPage.h"
#include <memory>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QWidget>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QDialog>
#include "StatusDelegate.h"
#include <set>

namespace hms::ui {

namespace {

QDateTime nextAppointmentSlot() {
    QDateTime next = QDateTime::currentDateTime().addSecs(15 * 60);
    next.setTime(QTime(next.time().hour(), next.time().minute(), 0));

    const int remainder = next.time().minute() % 15;
    if (remainder != 0) {
        next = next.addSecs((15 - remainder) * 60);
    }

    return next;
}

class RequestAppointmentDialog : public QDialog {
public:
    RequestAppointmentDialog(services::DoctorService& doctorService, const core::User& user, QWidget* parent = nullptr)
        : QDialog(parent), doctorService_(doctorService), currentUser_(user) {
        setWindowTitle("Request New Appointment");
        setMinimumWidth(400);

        auto* layout = new QVBoxLayout(this);
        auto* form = new QFormLayout();

        specCombo = new QComboBox(this);
        doctorCombo = new QComboBox(this);
        dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), this);
        dateTimeEdit->setCalendarPopup(true);

        auto doctors = doctorService_.getAllDoctors(currentUser_);
        std::set<std::string> specs;
        for (const auto& d : doctors) specs.insert(d.getSpecialization());
        
        specCombo->addItem("Select Specialization", "");
        for (const auto& s : specs) specCombo->addItem(QString::fromStdString(s));

        form->addRow("Specialization", specCombo);
        form->addRow("Doctor", doctorCombo);
        form->addRow("Preferred Time", dateTimeEdit);

        auto* btnBox = new QHBoxLayout();
        auto* submitBtn = new QPushButton("Submit Request", this);
        submitBtn->setObjectName("primaryButton");
        auto* cancelBtn = new QPushButton("Cancel", this);
        
        btnBox->addStretch();
        btnBox->addWidget(cancelBtn);
        btnBox->addWidget(submitBtn);

        layout->addLayout(form);
        layout->addLayout(btnBox);

        connect(specCombo, &QComboBox::currentTextChanged, [this, doctors](const QString& spec) {
            doctorCombo->clear();
            for (const auto& d : doctors) {
                if (spec.isEmpty() || QString::fromStdString(d.getSpecialization()) == spec) {
                    doctorCombo->addItem(QString::fromStdString(d.getName()), d.getId());
                }
            }
        });

        connect(submitBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    }

    int getDoctorId() const { return doctorCombo->currentData().toInt(); }
    QDateTime getDateTime() const { return dateTimeEdit->dateTime(); }

private:
    services::DoctorService& doctorService_;
    const core::User& currentUser_;
    QComboBox* specCombo;
    QComboBox* doctorCombo;
    QDateTimeEdit* dateTimeEdit;
};

}  // namespace

AppointmentsPage::AppointmentsPage(services::AppointmentService& appointmentService,
                                 services::PatientService& patientService,
                                 services::DoctorService& doctorService,
                                 QWidget* parent)
    : QWidget(parent), 
      appointmentService_(appointmentService),
      patientService_(patientService),
      doctorService_(doctorService) {
    setupUi();
    wireSignals();
}

void AppointmentsPage::setRole(core::UserRole role) {
    currentRole_ = role;
    const bool isPatient = (role == core::UserRole::Patient);
    const bool isDoctor = (role == core::UserRole::Doctor);
    const bool isAdmin = (role == core::UserRole::Admin);
    const bool isReceptionist = (role == core::UserRole::Receptionist);
    
    // Patients see NO management form, only a "Request" button
    formCard_->setVisible(!isPatient);
    requestAppointmentButton_->setVisible(isPatient);

    if (!isPatient) {
        // Doctors can only update status to Completed
        const bool fullManagement = isAdmin || isReceptionist;
        patientCombo_->setEnabled(fullManagement);
        specCombo_->setEnabled(fullManagement);
        doctorCombo_->setEnabled(fullManagement);
        dateTimeEdit_->setEnabled(fullManagement);
        statusCombo_->setEnabled(true); // Doctors CAN change status
        
        saveButton_->setVisible(fullManagement);
        deleteButton_->setVisible(isAdmin);
        clearButton_->setVisible(fullManagement);
        registerPatientButton_->setVisible(fullManagement);
    }
}

void AppointmentsPage::setCurrentUser(const core::User& user) {
    currentUser_ = std::make_unique<core::User>(user);
    refresh();
}

void AppointmentsPage::refresh() {
    if (!currentUser_) return;
    try {
        auto appointments = appointmentService_.getAllAppointments(*currentUser_);
        auto patients = patientService_.getAllPatients(*currentUser_);
        auto doctors = doctorService_.getAllDoctors(*currentUser_);

        std::map<int, std::string> patientNames;
        std::map<int, std::string> doctorNames;
        
        patientCombo_->clear();
        patientCombo_->addItem("Select Patient", -1);
        for (const auto& p : patients) {
            patientNames[p.getId()] = p.getName();
            patientCombo_->addItem(QString::fromStdString(p.getName()), p.getId());
        }

        std::set<std::string> specializations;
        for (const auto& d : doctors) {
            doctorNames[d.getId()] = d.getName();
            specializations.insert(d.getSpecialization());
        }

        specCombo_->clear();
        specCombo_->addItem("All Specializations", "");
        for (const auto& spec : specializations) {
            specCombo_->addItem(QString::fromStdString(spec));
        }

        appointmentModel_->setPatientMap(patientNames);
        appointmentModel_->setDoctorMap(doctorNames);
        appointmentModel_->setAppointments(appointments);

        if (currentRole_ == core::UserRole::Patient) {
            for (int i = 0; i < patientCombo_->count(); ++i) {
                if (patientCombo_->itemData(i).toInt() == currentUser_->getId()) {
                    patientCombo_->setCurrentIndex(i);
                    break;
                }
            }
        }
        
        updateDoctorSelection();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void AppointmentsPage::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    // Form Side
    formCard_ = new QFrame(this);
    formCard_->setObjectName("pageCard");
    formCard_->setFixedWidth(340);
    auto* formLayout = new QFormLayout(formCard_);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(14);

    auto* formTitle = new QLabel("Appointment Management", formCard_);
    formTitle->setObjectName("cardTitle");
    formLayout->addRow(formTitle);

    idInput_ = new QSpinBox(formCard_);
    idInput_->setRange(0, 999999);
    idInput_->setEnabled(false);
    
    patientCombo_ = new QComboBox(formCard_);
    registerPatientButton_ = new QPushButton("Register New Patient", formCard_);
    registerPatientButton_->setObjectName("secondaryButton");
    specCombo_ = new QComboBox(formCard_);
    doctorCombo_ = new QComboBox(formCard_);
    dateTimeEdit_ = new QDateTimeEdit(nextAppointmentSlot(), formCard_);
    dateTimeEdit_->setDisplayFormat("yyyy-MM-dd HH:mm");
    dateTimeEdit_->setCalendarPopup(true);
    dateTimeEdit_->setMinimumDateTime(nextAppointmentSlot());
    
    statusCombo_ = new QComboBox(formCard_);
    statusCombo_->addItems({"Scheduled", "Completed", "Cancelled"});

    formLayout->addRow("Appt ID", idInput_);
    formLayout->addRow("Patient", patientCombo_);
    formLayout->addRow("", registerPatientButton_);
    formLayout->addRow("Spec.", specCombo_);
    formLayout->addRow("Doctor", doctorCombo_);
    formLayout->addRow("Date/Time", dateTimeEdit_);
    formLayout->addRow("Status", statusCombo_);

    auto* btnGrid = new QGridLayout();
    saveButton_ = new QPushButton("Save", formCard_);
    saveButton_->setObjectName("primaryButton");
    updateButton_ = new QPushButton("Update", formCard_);
    updateButton_->setObjectName("secondaryButton");
    deleteButton_ = new QPushButton("Delete", formCard_);
    deleteButton_->setObjectName("dangerButton");
    clearButton_ = new QPushButton("Clear", formCard_);
    clearButton_->setObjectName("secondaryButton");

    btnGrid->addWidget(saveButton_, 0, 0);
    btnGrid->addWidget(updateButton_, 0, 1);
    btnGrid->addWidget(deleteButton_, 1, 0);
    btnGrid->addWidget(clearButton_, 1, 1);
    formLayout->addRow(btnGrid);

    // Request Button (for Patients)
    requestAppointmentButton_ = new QPushButton("Request New Appointment", this);
    requestAppointmentButton_->setObjectName("primaryButton");
    requestAppointmentButton_->setFixedHeight(50);
    requestAppointmentButton_->setVisible(false);

    // Table Side
    auto* tableCard = new QFrame(this);
    tableCard->setObjectName("pageCard");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(20, 20, 20, 20);
    tableLayout->setSpacing(14);

    auto* tableTitle = new QLabel("Appointments Schedule", tableCard);
    tableTitle->setObjectName("cardTitle");
    
    searchInput_ = new QLineEdit(tableCard);
    searchInput_->setPlaceholderText("Search appointments...");

    tableView_ = new QTableView(tableCard);
    appointmentModel_ = new models::AppointmentModel(this);
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(appointmentModel_);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel_->setFilterKeyColumn(-1);
    
    tableView_->setModel(proxyModel_);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(requestAppointmentButton_);
    tableLayout->addWidget(searchInput_);
    tableLayout->addWidget(tableView_);
    
    tableView_->setItemDelegateForColumn(4, new StatusDelegate(this)); // Status column

    layout->addWidget(formCard_);
    layout->addWidget(tableCard, 1);
}

void AppointmentsPage::wireSignals() {
    connect(saveButton_, &QPushButton::clicked, this, &AppointmentsPage::onSaveClicked);
    connect(updateButton_, &QPushButton::clicked, this, &AppointmentsPage::onUpdateClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &AppointmentsPage::onDeleteClicked);
    connect(clearButton_, &QPushButton::clicked, this, &AppointmentsPage::clearForm);
    connect(searchInput_, &QLineEdit::textChanged, proxyModel_, &QSortFilterProxyModel::setFilterFixedString);
    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AppointmentsPage::onSelectionChanged);
    connect(specCombo_, &QComboBox::currentTextChanged, this, &AppointmentsPage::updateDoctorSelection);
    connect(registerPatientButton_, &QPushButton::clicked, this, &AppointmentsPage::registerPatientRequested);
    
    connect(requestAppointmentButton_, &QPushButton::clicked, this, [this]() {
        if (!currentUser_) return;
        RequestAppointmentDialog dialog(doctorService_, *currentUser_, this);
        if (dialog.exec() == QDialog::Accepted) {
            try {
                appointmentService_.requestAppointment(
                    dialog.getDoctorId(),
                    dialog.getDateTime().toString("yyyy-MM-dd HH:mm").toStdString(),
                    *currentUser_
                );
                emit appointmentSaved();
                refresh();
            } catch (const std::exception& e) {
                emit errorOccurred(e.what());
            }
        }
    });
}

void AppointmentsPage::updateDoctorSelection() {
    if (!currentUser_) return;
    try {
        QString selectedSpec = specCombo_->currentText();
        auto doctors = doctorService_.getAllDoctors(*currentUser_);
        
        int currentDoctorId = doctorCombo_->currentData().toInt();
        doctorCombo_->clear();
        doctorCombo_->addItem("Select Doctor", -1);
        
        for (const auto& d : doctors) {
            if (selectedSpec == "All Specializations" || selectedSpec == "" || QString::fromStdString(d.getSpecialization()) == selectedSpec) {
                doctorCombo_->addItem(QString::fromStdString(d.getName()), d.getId());
            }
        }
        
        // Try to restore previous selection
        for (int i = 0; i < doctorCombo_->count(); ++i) {
            if (doctorCombo_->itemData(i).toInt() == currentDoctorId) {
                doctorCombo_->setCurrentIndex(i);
                break;
            }
        }
    } catch (...) {}
}

void AppointmentsPage::clearForm() {
    idInput_->setValue(0);
    if (currentRole_ != core::UserRole::Patient) {
        patientCombo_->setCurrentIndex(0);
    }
    specCombo_->setCurrentIndex(0);
    doctorCombo_->setCurrentIndex(0);
    dateTimeEdit_->setMinimumDateTime(nextAppointmentSlot());
    dateTimeEdit_->setDateTime(nextAppointmentSlot());
    statusCombo_->setCurrentIndex(0);
    tableView_->clearSelection();
}

void AppointmentsPage::onSelectionChanged() {
    auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) return;

    int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const auto& appt = appointmentModel_->appointmentAt(sourceRow);

    idInput_->setValue(appt.getId());
    
    for (int i = 0; i < patientCombo_->count(); ++i) {
        if (patientCombo_->itemData(i).toInt() == appt.getPatientId()) {
            patientCombo_->setCurrentIndex(i);
            break;
        }
    }
    
    // We might need to find the doctor's specialization to select it first
    try {
        auto doctors = doctorService_.getAllDoctors(*currentUser_);
        for (const auto& d : doctors) {
            if (d.getId() == appt.getDoctorId()) {
                specCombo_->setCurrentText(QString::fromStdString(d.getSpecialization()));
                break;
            }
        }
    } catch (...) {}

    for (int i = 0; i < doctorCombo_->count(); ++i) {
        if (doctorCombo_->itemData(i).toInt() == appt.getDoctorId()) {
            doctorCombo_->setCurrentIndex(i);
            break;
        }
    }

    dateTimeEdit_->setDateTime(QDateTime::fromString(QString::fromStdString(appt.getDateTime()), "yyyy-MM-dd HH:mm"));
    statusCombo_->setCurrentText(QString::fromStdString(core::toString(appt.getStatus())));
}

void AppointmentsPage::onSaveClicked() {
    if (!currentUser_) return;
    const bool isPatient = (currentRole_ == core::UserRole::Patient);
    const int patientId = isPatient ? currentUser_->getId() : patientCombo_->currentData().toInt();
    const int doctorId = doctorCombo_->currentData().toInt();
    
    if (doctorId <= 0) {
        emit errorOccurred("Please select a doctor.");
        return;
    }

    if (!isPatient && patientId <= 0) {
        emit errorOccurred("Please select a patient.");
        return;
    }

    if (dateTimeEdit_->dateTime() <= QDateTime::currentDateTime()) {
        emit errorOccurred("Please choose a future appointment date and time.");
        return;
    }

    try {
        if (isPatient) {
             appointmentService_.requestAppointment(
                doctorId,
                dateTimeEdit_->dateTime().toString("yyyy-MM-dd HH:mm").toStdString(),
                *currentUser_
            );
        } else {
            appointmentService_.bookAppointment(
                patientId,
                doctorId,
                dateTimeEdit_->dateTime().toString("yyyy-MM-dd HH:mm").toStdString(),
                *currentUser_
            );
        }
        emit appointmentSaved();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void AppointmentsPage::onUpdateClicked() {
    if (!currentUser_) return;
    int apptId = idInput_->value();
    if (apptId <= 0) {
        emit errorOccurred("Please select an appointment to update.");
        return;
    }

    try {
        appointmentService_.updateAppointment(
            apptId,
            patientCombo_->currentData().toInt(),
            doctorCombo_->currentData().toInt(),
            dateTimeEdit_->dateTime().toString("yyyy-MM-dd HH:mm").toStdString(),
            core::appointmentStatusFromString(statusCombo_->currentText().toStdString()),
            *currentUser_
        );
        emit appointmentSaved();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void AppointmentsPage::onDeleteClicked() {
    if (!currentUser_) return;
    int apptId = idInput_->value();
    if (apptId <= 0) return;

    try {
        appointmentService_.deleteAppointment(apptId, *currentUser_);
        emit appointmentDeleted();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

}  // namespace hms::ui
