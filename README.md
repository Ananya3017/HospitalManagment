# 🏥 Hospital Management System (C++17 + Qt Widgets)

A **desktop Hospital Management System** built using **C++17 and Qt Widgets**.
The application provides a clean graphical interface for managing **patients, doctors, appointments, and billing** with a modular architecture and SOLID-inspired design.

This project demonstrates **modern C++ architecture, object-oriented design, GUI development with Qt, and persistent data storage**.

---

# 🚀 Features

### 🔐 Authentication

* Role-based login system
* Multiple user roles supported

### 👩‍⚕️ Patient Management

* Create, update, delete patients
* List all patients
* Search and filter records

### 🧑‍⚕️ Doctor Management

* Register and manage doctors
* Update doctor information
* Search and filter doctors

### 📅 Appointment Management

* Book appointments
* Link appointments to patients and doctors
* Edit or cancel appointments

### 💳 Billing System

* Generate appointment bills
* Billing linked to appointment records
* Service-layer billing rules

### 📊 Dashboard

* KPI metrics display:

  * Total Patients
  * Total Doctors
  * Total Appointments
  * Total Bills

### 🖥 User Interface

* Qt Widgets GUI
* Table-based data display
* Tab-based navigation
* Status bar feedback
* Row selection autofill
* Search filtering per tab

### 💾 Persistence

* Data stored using **TSV (Tab Separated Values) files**
* Automatically loaded at application startup
* Saved after create/update/delete operations

---

# 🧠 Architecture

The system follows a **layered modular architecture** inspired by **Clean Architecture and SOLID principles**.

```
core → domain models and shared interfaces
services → business logic and validation
infrastructure → repository implementations
app → application orchestration
persistence → file storage layer
ui → Qt GUI layer
```

Each layer has a clear responsibility which makes the system **maintainable and scalable**.

---

# 📂 Project Structure

```
HospitalManagement
│
├── core
│   ├── User.h
│   ├── Patient.h
│   ├── Doctor.h
│   ├── Appointment.h
│   └── Billing.h
│
├── services
│   ├── PatientService.h
│   ├── DoctorService.h
│   ├── AppointmentService.h
│   ├── BillingService.h
│   └── AuthService.h
│
├── infrastructure
│   └── repositories
│       └── InMemoryRepository.h
│
├── app
│   ├── HospitalContext.h
│   └── HospitalDataCoordinator.h
│
├── persistence
│   ├── IDataStore.h
│   └── TsvDataStore.h
│
├── ui
│   ├── HospitalWindowFactory.h
│   └── HospitalWindow.cpp
│
├── qt_main.cpp
├── main.cpp
└── CMakeLists.txt
```

---

# ⚙️ Entry Points

### GUI Application

```
qt_main.cpp
```

Starts the **Qt graphical application**.

### Legacy Console Mode

```
main.cpp
```

Provides the original **console-based interface**.

---

# 🧩 Business Rules

### Role Permissions

| Feature                | Roles               |
| ---------------------- | ------------------- |
| Patient Management     | Admin, Receptionist |
| Doctor Management      | Logged-in user      |
| Appointment Management | Logged-in user      |
| Billing                | Logged-in user      |

### Billing Rules

* Bills are generated based on **appointment records**
* Fixed charge logic handled in **BillingService**

### Validation Rules

* IDs must be positive
* Required fields cannot be empty
* Age and billing amounts validated
* Cross-entity references validated
  (e.g. appointments must reference existing patients and doctors)

---

# 💾 Data Persistence

Storage format:

```
TSV (Tab Separated Values)
```

Stored entities:

* Patients
* Doctors
* Appointments
* Bills

### Data Location

```
data/
```

The folder is created **next to the executable** at runtime.

Data is:

* Loaded during application startup
* Saved after create/update/delete operations

---

# 🛠 Build System

The project uses:

* **CMake**
* **Ninja**
* **Qt Widgets**
* **C++17**

---

# 🔧 Requirements

Install the following tools:

* C++17 compatible compiler
* Qt 6
* CMake
* Ninja

Example tools:

* Qt Creator
* MSYS2 Qt
* Visual Studio Code

---

# ▶️ Build Instructions

### 1. Clone the Repository

```
git clone https://github.com/yourusername/hospital-management-system.git
cd hospital-management-system
```

### 2. Configure Build

```
cmake -G Ninja -S . -B build
```

### 3. Build Project

```
cmake --build build
```

### 4. Run Application

```
./build/HospitalManagementQt.exe
```

---

# 🔑 Default Login Accounts

| Username  | Password |
| --------- | -------- |
| admin     | admin123 |
| reception | rec123   |
| doctor1   | doc123   |

---

# 🧪 Development Environment

The project includes **VS Code configuration** for Qt development:

```
.vscode/settings.json
.vscode/c_cpp_properties.json
```

Configured for:

* MSYS2 Qt toolchain
* C++ IntelliSense
* Include paths for Qt libraries

---

# 🎨 UI Layout

Main interface includes:

* **Session/Login panel**
* **KPI metric cards**
* **Tabbed management interface**

Tabs:

```
Patients
Doctors
Appointments
Billing
```

Each tab supports:

* Table view
* Search filtering
* Row selection editing
* CRUD operations

---

# 📈 Future Improvements

Possible enhancements:

* Database integration (SQLite/PostgreSQL)
* REST API backend
* Web dashboard
* Role-based permission expansion
* Reporting and analytics
* Export to PDF invoices
* Unit testing suite

---

# 📚 Learning Objectives

This project demonstrates:

* C++17 application architecture
* Object-Oriented Programming
* SOLID design principles
* Qt GUI development
* Layered system architecture
* Data persistence design
* CMake build configuration

---

# 👨‍💻 Author

Developed as a **C++ + Qt architecture project** demonstrating a full modular desktop system.

---

# 📄 License

This project is released under the **MIT License**.
