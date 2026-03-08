#include "core/Appointment.h"
#include "core/Billing.h"
#include "core/Doctor.h"
#include "core/Patient.h"
#include "core/User.h"
#include "infrastructure/repositories/InMemoryRepository.h" // stores objects temporary db
#include "services/AppointmentService.h"
#include "services/AuthService.h"
#include "services/BillingService.h"
#include "services/DoctorService.h"
#include "services/PatientService.h"
#include "utils/Logger.h" // logs class and log errors

#include <iostream>
#include <memory> // smart pointers
#include <string>

using namespace std;

int main() {
    using hms::core::Appointment;
    using hms::core::Billing;
    using hms::core::Doctor;
    using hms::core::Patient;
    using hms::core::User;
    using hms::core::UserRole;
    using hms::infrastructure::repositories::InMemoryRepository;
    using hms::services::AppointmentService;
    using hms::services::AuthService;
    using hms::services::BillingService;
    using hms::services::DoctorService;
    using hms::services::PatientService;

    auto logger = make_shared<hms::utils::Logger>(); // creates a smart pointer
   // creating Repo
    auto patientRepo = make_shared<InMemoryRepository<Patient>>(
        [](const Patient& p) { return p.getId(); }); // lamba func-> tells repo to get id

    auto doctorRepo = make_shared<InMemoryRepository<Doctor>>(
        [](const Doctor& d) { return d.getId(); });

    auto appointmentRepo = make_shared<InMemoryRepository<Appointment>>(
        [](const Appointment& a) { return a.getId(); });

    auto billingRepo = make_shared<InMemoryRepository<Billing>>(
        [](const Billing& b) { return b.getId(); });

    auto userRepo = make_shared<InMemoryRepository<User>>(
        [](const User& u) { return u.getId(); });

    //creating Services  - using DI - does not create but recieves from outside 
    PatientService patientService(patientRepo, logger);
    DoctorService doctorService(doctorRepo, logger);
    AppointmentService appointmentService(appointmentRepo, patientRepo, doctorRepo, logger);
    BillingService billingService(billingRepo, appointmentRepo, logger);
    AuthService authService(userRepo, logger);

    // Default users
    authService.registerUser(1, "admin", "admin123", UserRole::Admin);
    authService.registerUser(2, "reception", "rec123", UserRole::Receptionist);
    authService.registerUser(3, "doctor1", "doc123", UserRole::Doctor);

    unique_ptr<User> loggedInUser; // stores logged in user - one user only
    bool running = true;

    while (running) {
        cout << "\n===== Hospital Management System =====\n";

        if (loggedInUser)
            cout << "Logged in as: " << loggedInUser->getUsername() << "\n";
        else
            cout << "Logged in as: [none]\n";

        cout << "1. Login\n";
        cout << "2. Register Patient\n";
        cout << "3. Add Doctor\n";
        cout << "4. Book Appointment\n";
        cout << "5. Generate Bill\n";
        cout << "6. Exit\n";

        int choice;
        cout << "Select option: ";
        cin >> choice;
        cin.ignore();

        try {
            switch (choice) {
            //login
            case 1: {
                string username, password;
                cout << "Username: ";
                getline(cin, username);
                cout << "Password: ";
                getline(cin, password);

                loggedInUser = authService.login(username, password);
                cout << "Login successful.\n";
                break;
            }
            // Register Patient
            case 2: {
                if (!loggedInUser ||
                   (loggedInUser->getRole() != UserRole::Admin &&
                    loggedInUser->getRole() != UserRole::Receptionist)) {
                    cout << "Permission denied.\n";
                    break;
                }

                int id, age;
                string name, contact;

                cout << "Patient ID: ";
                cin >> id;
                cin.ignore();

                cout << "Name: ";
                getline(cin, name);

                cout << "Age: ";
                cin >> age;
                cin.ignore();

                cout << "Contact: ";
                getline(cin, contact);

                patientService.registerPatient(id, name, age, contact);
                cout << "Patient registered successfully.\n";
                break;
            }
            //add doctor
            case 3: {
                if (!loggedInUser) {
                    cout << "Login required.\n";
                    break;
                }

                int id;
                string name, specialization;

                cout << "Doctor ID: ";
                cin >> id;
                cin.ignore();

                cout << "Name: ";
                getline(cin, name);

                cout << "Specialization: ";
                getline(cin, specialization);

                doctorService.addDoctor(id, name, specialization);
                cout << "Doctor added successfully.\n";
                break;
            }
            // book appointment
            case 4: {
                if (!loggedInUser) {
                    cout << "Login required.\n";
                    break;
                }

                int id, patientId, doctorId;
                string dateTime;

                cout << "Appointment ID: ";
                cin >> id;
                cout << "Patient ID: ";
                cin >> patientId;
                cout << "Doctor ID: ";
                cin >> doctorId;
                cin.ignore();

                cout << "Date & Time: ";
                getline(cin, dateTime);

                appointmentService.bookAppointment(id, patientId, doctorId, dateTime);
                cout << "Appointment booked successfully.\n";
                break;
            }
            // generate bill
            case 5: {
                if (!loggedInUser) {
                    cout << "Login required.\n";
                    break;
                }

                int id, appointmentId;
                double amount;

                cout << "Bill ID: ";
                cin >> id;
                cout << "Appointment ID: ";
                cin >> appointmentId;
                cout << "Amount: ";
                cin >> amount;
                cin.ignore();

                billingService.generateBill(id, appointmentId, amount);
                cout << "Bill generated successfully.\n";
                break;
            }

            case 6:
                running = false;
                cout << "Exiting system...\n";
                break;

            default:
                cout << "Invalid option.\n";
            }
        }
        catch (const exception& e) {
            logger->error(e.what());
            cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}