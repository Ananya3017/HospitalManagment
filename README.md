# Hospital Management System (C++ / Qt)

## Overview
This project is a Hospital Management System built in modern C++ with a Qt Widgets GUI.  
It is organized with a modular OOP architecture:

- `core`: domain models and interfaces
- `services`: application/business logic
- `infrastructure`: repository implementations
- `app`: application composition and orchestration
- `persistence`: persistence abstractions and implementations
- `ui`: Qt window composition layer
- `qt_main.cpp`: thin composition root (startup/bootstrap)
- `main.cpp`: console entry point

## Features
- User login with role-based access
- Patient management
- Doctor management
- Appointment management
- Billing management
- Search/filter in tables
- Edit/Delete operations from GUI
- Local data persistence between app restarts

## Tech Stack
- C++17
- Qt Widgets (Qt6/Qt5 via CMake)
- CMake + Ninja
- MSYS2 UCRT64 toolchain (recommended on Windows)

## Project Structure
```text
HospitalManagment/
  app/
  core/
  infrastructure/
  persistence/
  services/
  ui/
  utils/
  main.cpp
  qt_main.cpp
  CMakeLists.txt
```

## Prerequisites (Windows + MSYS2)
Install in **MSYS2 UCRT64**:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base \
  mingw-w64-ucrt-x86_64-qt6-tools
```

## Build
From PowerShell:

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

## Data Persistence
The GUI stores data files in a local `data` folder near the executable.  
Data is loaded at startup and saved after create/update/delete operations.

## Notes
- If build fails with `Permission denied` on `HospitalManagmentQt.exe`, close any running app instance and rebuild.
- If text is not visible in inputs, ensure you are running the latest rebuilt executable.
