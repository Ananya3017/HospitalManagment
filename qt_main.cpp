#include "app/HospitalContext.h"
#include "app/HospitalDataCoordinator.h"
#include "persistence/TsvDataStore.h"
#include "ui/HospitalWindowFactory.h"

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>

#include <filesystem>
#include <memory>
#include <string>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    hms::app::HospitalContext context;

    auto dataStore = std::make_unique<hms::persistence::TsvDataStore>(
        std::filesystem::path(QApplication::applicationDirPath().toStdString()) / "data");

    hms::app::HospitalDataCoordinator dataCoordinator(
        context.patientService(),
        context.doctorService(),
        context.appointmentService(),
        context.billingService(),
        *dataStore);

    std::string loadError;
    if (!dataCoordinator.load(loadError) && !loadError.empty()) {
        QMessageBox::warning(nullptr, "Persistence Error", QString::fromStdString(loadError));
    }

    std::unique_ptr<QMainWindow> window = hms::ui::createHospitalWindow(
        context.patientService(),
        context.doctorService(),
        context.appointmentService(),
        context.billingService(),
        context.authService(),
        dataCoordinator);
    window->show();

    return app.exec();
}
