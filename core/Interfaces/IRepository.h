#pragma once

#include <optional>
#include <vector>
#include <stdexcept>

namespace hms::core::interfaces {

template <typename T>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual void add(const T& entity) = 0;
    virtual void update(const T& entity) = 0;
    virtual void remove(int id) = 0;
    [[nodiscard]] virtual int getNextId() const = 0;
    [[nodiscard]] virtual std::optional<T> getById(int id) const = 0;
    [[nodiscard]] virtual std::vector<T> getAll() const = 0;
};

}  // namespace hms::core::interfaces
