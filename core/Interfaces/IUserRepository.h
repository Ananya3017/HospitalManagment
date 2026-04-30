#pragma once

#include "IRepository.h"
#include "../User.h"

#include <optional>
#include <string>

namespace hms::core::interfaces {

class IUserRepository : public IRepository<core::User> {
public:
    ~IUserRepository() override = default;

    [[nodiscard]] virtual std::optional<core::User> getByUsername(const std::string& username) const = 0;
};

}  // namespace hms::core::interfaces
