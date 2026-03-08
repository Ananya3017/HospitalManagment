#include "TsvDataStore.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace hms::persistence {

namespace {

std::string sanitizeField(std::string value) {
    std::replace(value.begin(), value.end(), '\t', ' ');
    std::replace(value.begin(), value.end(), '\r', ' ');
    std::replace(value.begin(), value.end(), '\n', ' ');
    return value;
}

std::vector<std::string> splitTab(const std::string& line) {
    std::vector<std::string> output;
    std::stringstream stream(line);
    std::string part;
    while (std::getline(stream, part, '\t')) {
        output.push_back(part);
    }
    return output;
}

std::string statusToString(core::AppointmentStatus status) {
    switch (status) {
        case core::AppointmentStatus::Scheduled:
            return "Scheduled";
        case core::AppointmentStatus::Completed:
            return "Completed";
        case core::AppointmentStatus::Cancelled:
            return "Cancelled";
    }
    return "Scheduled";
}

core::AppointmentStatus statusFromString(const std::string& value) {
    if (value == "Completed") {
        return core::AppointmentStatus::Completed;
    }
    if (value == "Cancelled") {
        return core::AppointmentStatus::Cancelled;
    }
    return core::AppointmentStatus::Scheduled;
}

template <typename Writer>
bool writeLines(const std::filesystem::path& path, Writer writer, std::string& error) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        error = "Unable to open file for writing: " + path.string();
        return false;
    }
    writer(out);
    if (!out.good()) {
        error = "Failed while writing file: " + path.string();
        return false;
    }
    return true;
}

}  // namespace

TsvDataStore::TsvDataStore(std::filesystem::path dataDirectory) : dataDirectory_(std::move(dataDirectory)) {}

std::filesystem::path TsvDataStore::filePath(const std::string& fileName) const { return dataDirectory_ / fileName; }

bool TsvDataStore::save(const DataSnapshot& snapshot, std::string& error) const {
    std::error_code ec;
    std::filesystem::create_directories(dataDirectory_, ec);
    if (ec) {
        error = "Unable to create data directory: " + dataDirectory_.string();
        return false;
    }

    if (!writeLines(filePath("patients.tsv"), [&](std::ofstream& out) {
            for (const auto& patient : snapshot.patients) {
                out << patient.id << '\t' << sanitizeField(patient.name) << '\t' << patient.age << '\t'
                    << sanitizeField(patient.contact) << '\n';
            }
        }, error)) {
        return false;
    }

    if (!writeLines(filePath("doctors.tsv"), [&](std::ofstream& out) {
            for (const auto& doctor : snapshot.doctors) {
                out << doctor.id << '\t' << sanitizeField(doctor.name) << '\t'
                    << sanitizeField(doctor.specialization) << '\n';
            }
        }, error)) {
        return false;
    }

    if (!writeLines(filePath("appointments.tsv"), [&](std::ofstream& out) {
            for (const auto& appointment : snapshot.appointments) {
                out << appointment.id << '\t' << appointment.patientId << '\t' << appointment.doctorId << '\t'
                    << sanitizeField(appointment.dateTime) << '\t' << statusToString(appointment.status) << '\n';
            }
        }, error)) {
        return false;
    }

    if (!writeLines(filePath("bills.tsv"), [&](std::ofstream& out) {
            for (const auto& bill : snapshot.bills) {
                out << bill.id << '\t' << bill.appointmentId << '\t' << bill.amount << '\t'
                    << (bill.isPaid ? 1 : 0) << '\n';
            }
        }, error)) {
        return false;
    }

    return true;
}

bool TsvDataStore::load(DataSnapshot& snapshot, std::string& error) const {
    snapshot = DataSnapshot{};
    if (!std::filesystem::exists(dataDirectory_)) {
        return true;
    }

    const auto loadFile = [&](const std::filesystem::path& path, std::vector<std::string>& lines) -> bool {
        if (!std::filesystem::exists(path)) {
            return true;
        }
        std::ifstream in(path);
        if (!in.is_open()) {
            error = "Unable to open file for reading: " + path.string();
            return false;
        }
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        return true;
    };

    std::vector<std::string> patientLines;
    std::vector<std::string> doctorLines;
    std::vector<std::string> appointmentLines;
    std::vector<std::string> billLines;

    if (!loadFile(filePath("patients.tsv"), patientLines) ||
        !loadFile(filePath("doctors.tsv"), doctorLines) ||
        !loadFile(filePath("appointments.tsv"), appointmentLines) ||
        !loadFile(filePath("bills.tsv"), billLines)) {
        return false;
    }

    try {
        for (const auto& line : patientLines) {
            const auto parts = splitTab(line);
            if (parts.size() != 4) {
                continue;
            }
            snapshot.patients.push_back(
                PatientRecord{std::stoi(parts[0]), parts[1], std::stoi(parts[2]), parts[3]});
        }

        for (const auto& line : doctorLines) {
            const auto parts = splitTab(line);
            if (parts.size() != 3) {
                continue;
            }
            snapshot.doctors.push_back(DoctorRecord{std::stoi(parts[0]), parts[1], parts[2]});
        }

        for (const auto& line : appointmentLines) {
            const auto parts = splitTab(line);
            if (parts.size() != 5) {
                continue;
            }
            snapshot.appointments.push_back(AppointmentRecord{
                std::stoi(parts[0]),
                std::stoi(parts[1]),
                std::stoi(parts[2]),
                parts[3],
                statusFromString(parts[4])});
        }

        for (const auto& line : billLines) {
            const auto parts = splitTab(line);
            if (parts.size() != 4) {
                continue;
            }
            snapshot.bills.push_back(
                BillRecord{std::stoi(parts[0]), std::stoi(parts[1]), std::stod(parts[2]), parts[3] == "1"});
        }
    } catch (const std::exception& ex) {
        error = std::string("Failed to parse persisted data: ") + ex.what();
        return false;
    }

    return true;
}

}  // namespace hms::persistence
