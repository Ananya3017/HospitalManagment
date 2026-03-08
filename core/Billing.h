#pragma once

namespace hms::core {

class Billing final {
public:
    Billing(int id, int appointmentId, double amount, bool isPaid)
        : id_(id), appointmentId_(appointmentId), amount_(amount), isPaid_(isPaid) {}

    [[nodiscard]] int getId() const noexcept { return id_; }
    [[nodiscard]] int getAppointmentId() const noexcept { return appointmentId_; }
    [[nodiscard]] double getAmount() const noexcept { return amount_; }
    [[nodiscard]] bool isPaid() const noexcept { return isPaid_; }
    void setPaid(bool paid) noexcept { isPaid_ = paid; }

private:
    int id_;
    int appointmentId_;
    double amount_;
    bool isPaid_;
};

}  // namespace hms::core
