#include "BillingPage.h"
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
#include <QDoubleSpinBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QWidget>
#include <QItemSelectionModel>
#include <QAbstractItemView>

namespace hms::ui {

BillingPage::BillingPage(services::BillingService& billingService,
                       services::AppointmentService& appointmentService,
                       QWidget* parent)
    : QWidget(parent), 
      billingService_(billingService),
      appointmentService_(appointmentService) {
    setupUi();
    wireSignals();
}

void BillingPage::setRole(core::UserRole role) {
    currentRole_ = role;
    const bool isAdmin = (role == core::UserRole::Admin);
    const bool isReceptionist = (role == core::UserRole::Receptionist);
    const bool canEdit = (isAdmin || isReceptionist);
    
    saveButton_->setVisible(canEdit);
    updateButton_->setVisible(canEdit);
    deleteButton_->setVisible(isAdmin);
    
    appointmentCombo_->setEnabled(canEdit);
    amountInput_->setEnabled(canEdit);
    paidCombo_->setEnabled(canEdit);
}

void BillingPage::setCurrentUser(const core::User& user) {
    currentUser_ = std::make_unique<core::User>(user);
    refresh();
}

void BillingPage::refresh() {
    if (!currentUser_) return;
    try {
        auto bills = billingService_.getAllBills(*currentUser_);
        auto appointments = appointmentService_.getAllAppointments(*currentUser_);

        appointmentCombo_->clear();
        appointmentCombo_->addItem("Select Appointment", -1);
        for (const auto& a : appointments) {
             appointmentCombo_->addItem(QString("Appt #%1 (%2)").arg(a.getId()).arg(QString::fromStdString(a.getDateTime())), a.getId());
        }

        billModel_->setBills(bills);
        clearForm();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void BillingPage::setupUi() {
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

    auto* formTitle = new QLabel("Bill Processing", formCard);
    formTitle->setObjectName("cardTitle");
    formLayout->addRow(formTitle);

    idInput_ = new QSpinBox(formCard);
    idInput_->setRange(0, 999999);
    idInput_->setEnabled(false);
    
    appointmentCombo_ = new QComboBox(formCard);
    
    amountInput_ = new QDoubleSpinBox(formCard);
    amountInput_->setRange(0, 999999.99);
    amountInput_->setPrefix("$ ");
    amountInput_->setDecimals(2);
    amountInput_->setValue(services::BillingService::appointmentCharge());

    paidCombo_ = new QComboBox(formCard);
    paidCombo_->addItems({"No", "Yes"});

    formLayout->addRow("Bill ID", idInput_);
    formLayout->addRow("Appointment", appointmentCombo_);
    formLayout->addRow("Amount", amountInput_);
    formLayout->addRow("Is Paid?", paidCombo_);

    auto* btnGrid = new QGridLayout();
    saveButton_ = new QPushButton("Generate Bill", formCard);
    saveButton_->setObjectName("primaryButton");
    updateButton_ = new QPushButton("Update Bill", formCard);
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

    auto* tableTitle = new QLabel("Billing Ledger", tableCard);
    tableTitle->setObjectName("cardTitle");
    
    searchInput_ = new QLineEdit(tableCard);
    searchInput_->setPlaceholderText("Search ledger...");

    tableView_ = new QTableView(tableCard);
    billModel_ = new models::BillModel(this);
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(billModel_);
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

void BillingPage::wireSignals() {
    connect(saveButton_, &QPushButton::clicked, this, &BillingPage::onSaveClicked);
    connect(updateButton_, &QPushButton::clicked, this, &BillingPage::onUpdateClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &BillingPage::onDeleteClicked);
    connect(clearButton_, &QPushButton::clicked, this, &BillingPage::clearForm);
    connect(searchInput_, &QLineEdit::textChanged, proxyModel_, &QSortFilterProxyModel::setFilterFixedString);
    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BillingPage::onSelectionChanged);
}

void BillingPage::clearForm() {
    idInput_->setValue(0);
    appointmentCombo_->setCurrentIndex(0);
    amountInput_->setValue(services::BillingService::appointmentCharge());
    paidCombo_->setCurrentIndex(0);
    tableView_->clearSelection();
}

void BillingPage::onSelectionChanged() {
    auto indexes = tableView_->selectionModel()->selectedRows();
    if (indexes.isEmpty()) return;

    int sourceRow = proxyModel_->mapToSource(indexes.first()).row();
    const auto& bill = billModel_->billAt(sourceRow);

    idInput_->setValue(bill.getId());
    
    for (int i = 0; i < appointmentCombo_->count(); ++i) {
        if (appointmentCombo_->itemData(i).toInt() == bill.getAppointmentId()) {
            appointmentCombo_->setCurrentIndex(i);
            break;
        }
    }

    amountInput_->setValue(bill.getAmount());
    paidCombo_->setCurrentText(bill.isPaid() ? "Yes" : "No");
}

void BillingPage::onSaveClicked() {
    if (!currentUser_) return;
    int apptId = appointmentCombo_->currentData().toInt();
    if (apptId <= 0) {
        emit errorOccurred("Please select an appointment.");
        return;
    }

    try {
        billingService_.generateBill(
            apptId,
            amountInput_->value(),
            *currentUser_
        );
        emit billSaved();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void BillingPage::onUpdateClicked() {
    if (!currentUser_) return;
    int billId = idInput_->value();
    if (billId <= 0) {
        emit errorOccurred("Please select a bill to update.");
        return;
    }

    try {
        billingService_.updateBill(
            billId,
            appointmentCombo_->currentData().toInt(),
            amountInput_->value(),
            paidCombo_->currentText() == "Yes",
            *currentUser_
        );
        emit billSaved();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

void BillingPage::onDeleteClicked() {
    if (!currentUser_) return;
    int billId = idInput_->value();
    if (billId <= 0) return;

    try {
        billingService_.deleteBill(billId, *currentUser_);
        emit billDeleted();
        refresh();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }
}

}  // namespace hms::ui
