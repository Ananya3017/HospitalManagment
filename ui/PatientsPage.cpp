#include "PatientsPage.h"

#include <memory>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <QAbstractItemView>

namespace hms::ui {

PatientsPage::PatientsPage(services::PatientService& patientService, QWidget* parent)
    : QWidget(parent), patientService_(patientService) {
    setupUi();
    wireSignals();
}

void PatientsPage::setRole(core::UserRole role) {
    currentRole_ = role;
    const bool canManage = (role == core::UserRole::Admin || role == core::UserRole::Receptionist);
    
    saveButton_->setVisible(canManage);
    updateButton_->setVisible(canManage);
    deleteButton_->setVisible(role == core::UserRole::Admin);
    
    idInput_->setEnabled(canManage);
    nameInput_->setEnabled(canManage);
    ageInput_->setEnabled(canManage);
    contactInput_->setEnabled(canManage);
}

void PatientsPage::setCurrentUser(const core::User& user) {
    currentUser_ = std::make_unique<core::User>(user);
    refresh();
}

void PatientsPage::refresh() {
    if (!currentUser_) return;
    try {
        patientModel_->setPatients(patientService_.getAllPatients(*currentUser_));
        clearForm();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void PatientsPage::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    // Form Side
    auto* formCard = new QFrame(this);
    formCard->setObjectName("pageCard");
    formCard->setFixedWidth(340);
    auto* formLayout = new QFormLayout(formCard);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(14);

    auto* formTitle = new QLabel("Patient Information", formCard);
    formTitle->setObjectName("cardTitle");
    formLayout->addRow(formTitle);

    idInput_ = new QSpinBox(formCard);
    idInput_->setRange(1, 999999);
    nameInput_ = new QLineEdit(formCard);
    ageInput_ = new QSpinBox(formCard);
    ageInput_->setRange(0, 150);
    contactInput_ = new QLineEdit(formCard);

    formLayout->addRow("Patient ID", idInput_);
    formLayout->addRow("Full Name", nameInput_);
    formLayout->addRow("Age", ageInput_);
    formLayout->addRow("Contact", contactInput_);

    auto* btnGrid = new QGridLayout();
    saveButton_ = new QPushButton("Save", formCard);
    saveButton_->setObjectName("primaryButton");
    updateButton_ = new QPushButton("Update", formCard);
    updateButton_->setObjectName("secondaryButton");
    deleteButton_ = new QPushButton("Delete", formCard);
    deleteButton_->setObjectName("dangerButton");
    clearButton_ = new QPushButton("Clear", formCard);
    clearButton_->setObjectName("secondaryButton");

    btnGrid->addWidget(saveButton_, 0, 0);
    btnGrid->addWidget(updateButton_, 0, 1);
    btnGrid->addWidget(deleteButton_, 1, 0);
    btnGrid->addWidget(clearButton_, 1, 1);
    formLayout->addRow(btnGrid);

    // Table Side
    auto* tableCard = new QFrame(this);
    tableCard->setObjectName("pageCard");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(20, 20, 20, 20);
    tableLayout->setSpacing(14);

    auto* tableTitle = new QLabel("Patient Records", tableCard);
    tableTitle->setObjectName("cardTitle");
    
    searchInput_ = new QLineEdit(tableCard);
    searchInput_->setPlaceholderText("Search patients...");

    tableView_ = new QTableView(tableCard);
    patientModel_ = new models::PatientModel(this);
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(patientModel_);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel_->setFilterKeyColumn(-1);
    
    tableView_->setModel(proxyModel_);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(searchInput_);
    tableLayout->addWidget(tableView_);

    layout->addWidget(formCard);
    layout->addWidget(tableCard, 1);
}

void PatientsPage::wireSignals() {
    connect(saveButton_, &QPushButton::clicked, this, &PatientsPage::onSaveClicked);
    connect(updateButton_, &QPushButton::clicked, this, &PatientsPage::onUpdateClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &PatientsPage::onDeleteClicked);
    connect(clearButton_, &QPushButton::clicked, this, &PatientsPage::clearForm);
    connect(searchInput_, &QLineEdit::textChanged, proxyModel_, &QSortFilterProxyModel::setFilterFixedString);
    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PatientsPage::onSelectionChanged);
}

void PatientsPage::clearForm() {
    idInput_->setValue(1);
    nameInput_->clear();
    ageInput_->setValue(0);
    contactInput_->clear();
    tableView_->clearSelection();
}

void PatientsPage::onSelectionChanged() {
    auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) return;

    int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const auto& patient = patientModel_->patientAt(sourceRow);

    idInput_->setValue(patient.getId());
    nameInput_->setText(QString::fromStdString(patient.getName()));
    ageInput_->setValue(patient.getAge());
    contactInput_->setText(QString::fromStdString(patient.getContactNumber()));
}

void PatientsPage::onSaveClicked() {
    if (!currentUser_) return;
    try {
        patientService_.registerPatient(
            nameInput_->text().toStdString(),
            ageInput_->value(),
            contactInput_->text().toStdString(),
            *currentUser_
        );
        emit patientAdded();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void PatientsPage::onUpdateClicked() {
    if (!currentUser_) return;
    try {
        patientService_.updatePatient(
            idInput_->value(),
            nameInput_->text().toStdString(),
            ageInput_->value(),
            contactInput_->text().toStdString(),
            *currentUser_
        );
        emit patientUpdated();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void PatientsPage::onDeleteClicked() {
    if (!currentUser_) return;
    try {
        patientService_.deletePatient(idInput_->value(), *currentUser_);
        emit patientDeleted();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

}  // namespace hms::ui
