#pragma once

#include <string>
#include <utility>

namespace hms::core {

class Patient final {
public:
    Patient(int id, std::string name, int age, std::string contactNumber)
        : id_(id), name_(std::move(name)), age_(age), contactNumber_(std::move(contactNumber)) {}

    [[nodiscard]] int getId() const noexcept { return id_; } //  ⚠ compiler warning 
    [[nodiscard]] const std::string& getName() const noexcept { return name_; } // reference not copy , const prevents modification
    [[nodiscard]] int getAge() const noexcept { return age_; }
    [[nodiscard]] const std::string& getContactNumber() const noexcept { return contactNumber_; }

private:
    int id_;
    std::string name_;
    int age_;
    std::string contactNumber_;
};

} 
// immmutable 