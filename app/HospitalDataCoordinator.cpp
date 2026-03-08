#include "HospitalDataCoordinator.h"

#include <algorithm>

namespace hms::app {

HospitalDataCoordinator::HospitalDataCoordinator(services::PatientService& patientService,
                                                 services::DoctorService& doctorService,
                                                 services::AppointmentService& appointmentService,
                                                 services::BillingService& billingService,
                                                 persistence::IDataStore& dataStore)
    : patientService_(patientService),
      doctorService_(doctorService),
      appointmentService_(appointmentService),
      billingService_(billingService),
      dataStore_(dataStore) {}

bool HospitalDataCoordinator::load(std::string& error) {
    persistence::DataSnapshot snapshot;
    if (!dataStore_.load(snapshot, error)) {
        return false;
    }

    try {
        for (const auto& patient : snapshot.patients) {
            patientService_.registerPatient(patient.id, patient.name, patient.age, patient.contact);
        }

        for (const auto& doctor : snapshot.doctors) {
            doctorService_.addDoctor(doctor.id, doctor.name, doctor.specialization);
        }

        for (const auto& appointment : snapshot.appointments) {
            appointmentService_.bookAppointment(
                appointment.id, appointment.patientId, appointment.doctorId, appointment.dateTime);
            appointmentService_.updateAppointment(
                appointment.id, appointment.patientId, appointment.doctorId, appointment.dateTime, appointment.status);
        }

        for (const auto& bill : snapshot.bills) {
            billingService_.generateBill(
                bill.id, bill.appointmentId, services::BillingService::appointmentCharge());
            billingService_.updateBill(
                bill.id, bill.appointmentId, services::BillingService::appointmentCharge(), bill.isPaid);
        }
    } catch (const std::exception& ex) {
        error = std::string("Failed to load persisted data into services: ") + ex.what();
        return false;
    }

    return true;
}

bool HospitalDataCoordinator::save(std::string& error) const {
    persistence::DataSnapshot snapshot;

    const auto patients = patientService_.getAllPatients();
    for (const auto& patient : patients) {
        snapshot.patients.push_back(
            persistence::PatientRecord{patient.getId(), patient.getName(), patient.getAge(), patient.getContactNumber()});
    }

    const auto doctors = doctorService_.getAllDoctors();
    for (const auto& doctor : doctors) {
        snapshot.doctors.push_back(
            persistence::DoctorRecord{doctor.getId(), doctor.getName(), doctor.getSpecialization()});
    }

    const auto appointments = appointmentService_.getAllAppointments();
    for (const auto& appointment : appointments) {
        snapshot.appointments.push_back(persistence::AppointmentRecord{
            appointment.getId(),
            appointment.getPatientId(),
            appointment.getDoctorId(),
            appointment.getDateTime(),
            appointment.getStatus()});
    }

    const auto bills = billingService_.getAllBills();
    for (const auto& bill : bills) {
        snapshot.bills.push_back(
            persistence::BillRecord{bill.getId(), bill.getAppointmentId(), bill.getAmount(), bill.isPaid()});
    }

    std::sort(snapshot.patients.begin(), snapshot.patients.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    std::sort(snapshot.doctors.begin(), snapshot.doctors.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    std::sort(
        snapshot.appointments.begin(), snapshot.appointments.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    std::sort(snapshot.bills.begin(), snapshot.bills.end(), [](const auto& a, const auto& b) { return a.id < b.id; });

    return dataStore_.save(snapshot, error);
}

}  // namespace hms::app
