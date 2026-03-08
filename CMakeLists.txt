cmake_minimum_required(VERSION 3.16)
project(HospitalManagmentQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(HospitalManagmentQt
    app/HospitalContext.cpp
    app/HospitalDataCoordinator.cpp
    persistence/TsvDataStore.cpp
    qt_main.cpp
    ui/HospitalWindow.cpp
)

target_include_directories(HospitalManagmentQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HospitalManagmentQt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
