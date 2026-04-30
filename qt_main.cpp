#include "app/HospitalContext.h"
#include "ui/HospitalWindowFactory.h"

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>

#include <filesystem>
#include <memory>
#include <string>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  const std::filesystem::path databasePath =
      std::filesystem::path(QApplication::applicationDirPath().toStdString()) /
      "data" / "hospital.db";

  try {
    hms::app::HospitalContext context(databasePath);

    std::unique_ptr<QMainWindow> window = hms::ui::createHospitalWindow(
        context.patientService(), context.doctorService(),
        context.appointmentService(), context.billingService(),
        context.authService());
    window->show();

    return app.exec();
  } catch (const std::exception &e) {
    QMessageBox::critical(nullptr, "Fatal Error",
                          QString("Application failed to start:\n") + e.what());
    return 1;
  }
}
