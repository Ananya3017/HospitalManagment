#include "DoctorsPage.h"

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
#include <QWidget>
#include <QItemSelectionModel>
#include <QAbstractItemView>

namespace hms::ui {

DoctorsPage::DoctorsPage(services::DoctorService& doctorService, QWidget* parent)
    : QWidget(parent), doctorService_(doctorService) {
    setupUi();
    wireSignals();
}

void DoctorsPage::setRole(core::UserRole role) {
    currentRole_ = role;
    const bool isAdmin = (role == core::UserRole::Admin);
    const bool isDoctor = (role == core::UserRole::Doctor);
    
    saveButton_->setVisible(isAdmin);
    updateButton_->setVisible(isAdmin || isDoctor);
    deleteButton_->setVisible(isAdmin);
    
    nameInput_->setEnabled(isAdmin);
    specializationInput_->setEnabled(isAdmin);
    // Doctor can only update their own record, handled in Behavioral or Service layer
}

void DoctorsPage::setCurrentUser(const core::User& user) {
    currentUser_ = std::make_unique<core::User>(user);
    refresh();
}

void DoctorsPage::refresh() {
    if (!currentUser_) return;
    try {
        doctorModel_->setDoctors(doctorService_.getAllDoctors(*currentUser_));
        clearForm();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void DoctorsPage::setupUi() {
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

    auto* formTitle = new QLabel("Doctor Profile", formCard);
    formTitle->setObjectName("cardTitle");
    formLayout->addRow(formTitle);

    idInput_ = new QSpinBox(formCard);
    idInput_->setRange(1, 999999);
    idInput_->setEnabled(false); // ID usually assigned or non-editable here
    nameInput_ = new QLineEdit(formCard);
    specializationInput_ = new QLineEdit(formCard);

    formLayout->addRow("Doctor ID", idInput_);
    formLayout->addRow("Full Name", nameInput_);
    formLayout->addRow("Specialization", specializationInput_);

    auto* btnGrid = new QGridLayout();
    saveButton_ = new QPushButton("Add Doctor", formCard);
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

    auto* tableTitle = new QLabel("Medical Staff", tableCard);
    tableTitle->setObjectName("cardTitle");
    
    searchInput_ = new QLineEdit(tableCard);
    searchInput_->setPlaceholderText("Search doctors by name or specialization...");

    tableView_ = new QTableView(tableCard);
    doctorModel_ = new models::DoctorModel(this);
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(doctorModel_);
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

void DoctorsPage::wireSignals() {
    connect(saveButton_, &QPushButton::clicked, this, &DoctorsPage::onSaveClicked);
    connect(updateButton_, &QPushButton::clicked, this, &DoctorsPage::onUpdateClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &DoctorsPage::onDeleteClicked);
    connect(clearButton_, &QPushButton::clicked, this, &DoctorsPage::clearForm);
    connect(searchInput_, &QLineEdit::textChanged, proxyModel_, &QSortFilterProxyModel::setFilterFixedString);
    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DoctorsPage::onSelectionChanged);
}

void DoctorsPage::clearForm() {
    idInput_->setValue(1);
    nameInput_->clear();
    specializationInput_->clear();
    tableView_->clearSelection();
}

void DoctorsPage::onSelectionChanged() {
    auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) return;

    int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const auto& doctor = doctorModel_->doctorAt(sourceRow);

    idInput_->setValue(doctor.getId());
    nameInput_->setText(QString::fromStdString(doctor.getName()));
    specializationInput_->setText(QString::fromStdString(doctor.getSpecialization()));
}

void DoctorsPage::onSaveClicked() {
    if (!currentUser_) return;
    try {
        doctorService_.addDoctor(
            nameInput_->text().toStdString(),
            specializationInput_->text().toStdString(),
            *currentUser_
        );
        emit doctorAdded();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void DoctorsPage::onUpdateClicked() {
    if (!currentUser_) return;
    try {
        doctorService_.updateDoctor(
            idInput_->value(),
            nameInput_->text().toStdString(),
            specializationInput_->text().toStdString(),
            *currentUser_
        );
        emit doctorUpdated();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void DoctorsPage::onDeleteClicked() {
    if (!currentUser_) return;
    try {
        doctorService_.deleteDoctor(idInput_->value(), *currentUser_);
        emit doctorDeleted();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

}  // namespace hms::ui
