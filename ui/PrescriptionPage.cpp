#include "PrescriptionPage.h"
#include <memory>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QHeaderView>
#include <QTableView>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDateTime>

namespace hms::ui {

PrescriptionPage::PrescriptionPage(QWidget* parent) : QWidget(parent) {
    setupUi();
    wireSignals();
}

void PrescriptionPage::setRole(core::UserRole role) {
    currentRole_ = role;
    const bool isDoctor = (role == core::UserRole::Doctor);
    saveButton_->setVisible(isDoctor);
    patientIdInput_->setReadOnly(!isDoctor);
    diagnosisInput_->setReadOnly(!isDoctor);
    medicinesInput_->setReadOnly(!isDoctor);
}

void PrescriptionPage::setCurrentUser(const core::User& user) {
    currentUser_ = std::make_unique<core::User>(user);
    refresh();
}

void PrescriptionPage::refresh() {
    // Model already persists in memory during the session
}

void PrescriptionPage::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(18);

    // ── Form Card ──────────────────────────────────────────────────────
    auto* formCard = new QFrame(this);
    formCard->setObjectName("pageCard");
    formCard->setFixedWidth(340);

    auto* cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(0);

    auto* formTitle = new QLabel("New Prescription", formCard);
    formTitle->setObjectName("cardTitle");
    cardLayout->addWidget(formTitle);
    cardLayout->addSpacing(20);

    auto* form = new QFormLayout();
    form->setSpacing(14);
    form->setLabelAlignment(Qt::AlignLeft);

    patientIdInput_ = new QLineEdit(formCard);
    patientIdInput_->setPlaceholderText("Enter Patient ID");

    diagnosisInput_ = new QLineEdit(formCard);
    diagnosisInput_->setPlaceholderText("e.g. Hypertension");

    medicinesInput_ = new QTextEdit(formCard);
    medicinesInput_->setPlaceholderText("List medicines and dosage...");
    medicinesInput_->setFixedHeight(90);

    form->addRow("Patient ID", patientIdInput_);
    form->addRow("Diagnosis",  diagnosisInput_);
    form->addRow("Medicines",  medicinesInput_);

    cardLayout->addLayout(form);
    cardLayout->addSpacing(20);

    saveButton_ = new QPushButton("Save Prescription", formCard);
    saveButton_->setObjectName("primaryButton");
    saveButton_->setFixedHeight(40);
    cardLayout->addWidget(saveButton_);
    cardLayout->addStretch();

    // ── Table Card ────────────────────────────────────────────────────
    auto* tableCard = new QFrame(this);
    tableCard->setObjectName("pageCard");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(24, 24, 24, 24);
    tableLayout->setSpacing(14);

    auto* tableTitle = new QLabel("Prescription History", tableCard);
    tableTitle->setObjectName("cardTitle");

    // In-memory model: Doctor | Patient ID | Diagnosis | Medicines | Date
    prescriptionModel_ = new QStandardItemModel(0, 5, this);
    prescriptionModel_->setHorizontalHeaderLabels({
        "Doctor", "Patient ID", "Diagnosis", "Medicines", "Date"
    });

    tableView_ = new QTableView(tableCard);
    tableView_->setModel(prescriptionModel_);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->verticalHeader()->setVisible(false);

    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(tableView_);

    layout->addWidget(formCard);
    layout->addWidget(tableCard, 1);
}

void PrescriptionPage::wireSignals() {
    connect(saveButton_, &QPushButton::clicked, this, [this]() {
        // Validate
        const QString patientId = patientIdInput_->text().trimmed();
        const QString diagnosis = diagnosisInput_->text().trimmed();
        const QString medicines = medicinesInput_->toPlainText().trimmed();

        if (patientId.isEmpty() || diagnosis.isEmpty() || medicines.isEmpty()) {
            QMessageBox::warning(this, "Incomplete Form",
                "Please fill in Patient ID, Diagnosis, and Medicines before saving.");
            return;
        }

        const QString doctorName = currentUser_
            ? QString::fromStdString(currentUser_->getName())
            : "Unknown";

        // Append row to in-memory model
        QList<QStandardItem*> row;
        row << new QStandardItem(doctorName)
            << new QStandardItem(patientId)
            << new QStandardItem(diagnosis)
            << new QStandardItem(medicines)
            << new QStandardItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));

        prescriptionModel_->appendRow(row);

        // Scroll to new row
        tableView_->scrollToBottom();

        // Clear form for next entry
        patientIdInput_->clear();
        diagnosisInput_->clear();
        medicinesInput_->clear();
        patientIdInput_->setFocus();

        emit prescriptionSaved();
    });
}

} // namespace hms::ui
