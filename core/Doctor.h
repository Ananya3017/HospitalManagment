#pragma once

#include <string>
#include <utility>

namespace hms::core {

class Doctor final {
public:
    Doctor(int id, std::string name, std::string specialization)
        : id_(id), name_(std::move(name)), specialization_(std::move(specialization)) {}

    [[nodiscard]] int getId() const noexcept { return id_; }
    [[nodiscard]] const std::string& getName() const noexcept { return name_; }
    [[nodiscard]] const std::string& getSpecialization() const noexcept { return specialization_; }

private:
    int id_;
    std::string name_;
    std::string specialization_;
};

}  // namespace hms::core
