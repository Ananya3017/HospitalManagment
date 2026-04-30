# Hospital Management System (C++ / Qt / SQLite)

## Overview
Hospital Management System is a desktop application built with modern C++, Qt Widgets, and SQLite-backed persistence. It provides a graphical workflow for managing users, patients, doctors, appointments, prescriptions, and billing.

The project uses a modular architecture:

- `core`: domain entities and repository interfaces
- `database`: SQLite connection, schema setup, and transaction helpers
- `services`: application logic and validation
- `infrastructure`: SQLite repository implementations
- `models`: Qt table models for UI data
- `app`: dependency wiring and application context
- `ui`: Qt windows, pages, delegates, and layout composition
- `utils`: validation and permission helpers

## Features
- Role-based login
- Patient management
- Doctor management
- Appointment management
- Prescription workflow
- Billing management
- Dashboard metrics
- Search/filter support in tables
- Edit and delete actions from the GUI
- SQLite persistence with automatic schema creation
- Prepared statements and transaction support
- Theme icons and Qt resource support

## Tech Stack
- C++17
- Qt Widgets
- Qt SQL
- SQLite
- CMake
- Ninja
- MSYS2 UCRT64 toolchain on Windows

## Project Structure
```text
HospitalManagment/
  app/
  core/
  database/
  icons/
  infrastructure/
  models/
  services/
  ui/
  utils/
  qt_main.cpp
  resources.qrc
  CMakeLists.txt
```

## Requirements
Install:

- C++17 compatible compiler
- Qt 6 or Qt 5 with Widgets and SQL modules
- CMake
- Ninja

For MSYS2 UCRT64, install the recommended packages:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base \
  mingw-w64-ucrt-x86_64-qt6-tools
```

## Build
From PowerShell with MSYS2 installed at `C:\msys64`:

```powershell
& "C:\msys64\usr\bin\bash.exe" -lc "export PATH=/ucrt64/bin:$PATH; cmake -G Ninja -S /c/Users/anany/OneDrive/Desktop/HospitalManagment -B /c/Users/anany/OneDrive/Desktop/HospitalManagment/build-qt-ucrt64 -DCMAKE_BUILD_TYPE=Release"
& "C:\msys64\usr\bin\bash.exe" -lc "export PATH=/ucrt64/bin:$PATH; cmake --build /c/Users/anany/OneDrive/Desktop/HospitalManagment/build-qt-ucrt64 -j"
```

## Run
```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
.\build-qt-ucrt64\HospitalManagmentQt.exe
```

## Default Login Users
- `admin` / `admin123`
- `reception` / `rec123`
- `doctor1` / `doc123`

## Database
- SQLite database file: `data/hospital.db` next to the executable
- Foreign keys are enabled with `PRAGMA foreign_keys = ON`
- Tables are created automatically on startup:
  - `users`
  - `patients`
  - `doctors`
  - `appointments`
  - `bills`
- Constraints include:
  - foreign keys between related tables
  - `UNIQUE(doctor_id, time)` on appointments
  - `UNIQUE(appointment_id)` on bills
  - `NOT NULL` and `CHECK` constraints where appropriate

## Notes
- If build fails with `Permission denied` on `HospitalManagmentQt.exe`, close any running app instance and rebuild.
- The app seeds default login users only when they do not already exist.
- Generated build folders and local binaries are ignored by `.gitignore`.
