cmake_minimum_required(VERSION 3.16)
project(HospitalManagmentQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set Qt6 path explicitly
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(SQLite3 QUIET)

if(NOT SQLite3_FOUND)
    find_path(SQLite3_INCLUDE_DIR
        NAMES sqlite3.h
        PATHS
            C:/msys64/mingw64/include
            C:/msys64/ucrt64/include
    )

    find_library(SQLite3_LIBRARY
        NAMES sqlite3 libsqlite3.dll.a
        PATHS
            C:/msys64/mingw64/lib
            C:/msys64/ucrt64/lib
    )

    if(SQLite3_INCLUDE_DIR AND SQLite3_LIBRARY)
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLite3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}"
        )
        set(SQLite3_FOUND TRUE)
    else()
        message(FATAL_ERROR "SQLite3 not found. Install the SQLite development package or update the fallback paths in CMakeLists.txt.")
    endif()
endif()

add_executable(HospitalManagmentQt
    app/HospitalContext.cpp
    database/DatabaseManager.cpp
    database/Transaction.cpp
    infrastructure/repositories/SqliteAppointmentRepository.cpp
    infrastructure/repositories/SqliteBillingRepository.cpp
    infrastructure/repositories/SqliteDoctorRepository.cpp
    infrastructure/repositories/SqlitePatientRepository.cpp
    infrastructure/repositories/SqliteUserRepository.cpp
    qt_main.cpp
    ui/HospitalWindowBehavior.cpp
    ui/HospitalWindow.cpp
    ui/HospitalWindowCommon.cpp
    ui/HospitalWindowLayout.cpp
    ui/RoleSelectionPage.cpp
    ui/SetupPage.cpp
    ui/DashboardPage.cpp
    ui/PatientsPage.cpp
    ui/DoctorsPage.cpp
    ui/AppointmentsPage.cpp
    ui/BillingPage.cpp
    ui/StatusDelegate.cpp
    ui/PrescriptionPage.cpp
    models/PatientModel.cpp
    models/DoctorModel.cpp
    models/AppointmentModel.cpp
    models/BillModel.cpp

    resources.qrc
)

target_include_directories(HospitalManagmentQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HospitalManagmentQt PRIVATE
    Qt6::Widgets
    SQLite::SQLite3
)
