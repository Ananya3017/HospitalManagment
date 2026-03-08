#pragma once 
//repository for storing
#include "../../core/Interfaces/IRepository.h" // repo interface

#include <functional> //id extractor - lambda
#include <mutex> // used for thread safety
#include <optional> // returns nullable values
#include <stdexcept> // runtime exceptions
#include <unordered_map> // store entity using ID as key
#include <utility> // move 
#include <vector> // returns list of entities

namespace hms::infrastructure::repositories {
// class -> genric (same code for all) -> patient , doctor ,user etc 
template <typename T>
class InMemoryRepository final : public core::interfaces::IRepository<T> {
public: // takes a function to extract id from object 
    explicit InMemoryRepository(std::function<int(const T&)> idExtractor) : idExtractor_(std::move(idExtractor)) {
        if (!idExtractor_) {
            throw std::invalid_argument("idExtractor cannot be empty.");
        }
    }

    void add(const T& entity) override {
        const int id = idExtractor_(entity);
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_.find(id) != data_.end()) {
            throw std::runtime_error("Entity with the same ID already exists.");
        }
        data_.emplace(id, entity); // hash map
    }
    // to update existing
    void update(const T& entity) override { 
        const int id = idExtractor_(entity);
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find(id);
        if (it == data_.end()) {
            throw std::runtime_error("Entity not found for update.");
        }
        it->second = entity;
    }

    void remove(int id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto erased = data_.erase(id);
        if (erased == 0U) { // if nothing removed-> entity didn't exist
            throw std::runtime_error("Entity not found for deletion.");
        }
    }
    //     nodiscard -> shows a Compiler warning - if return value is ignored
    //     optional<T> - return type - value of T/ nothing(May be found or not )
    [[nodiscard]] std::optional<T> getById(int id) const override { 
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find(id);
        if (it == data_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] std::vector<T> getAll() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<T> output;
        output.reserve(data_.size());
        for (const auto& [id, entity] : data_) {
            (void)id;
            output.push_back(entity);
        }
        return output;
    }

private:
    std::function<int(const T&)> idExtractor_;
    mutable std::mutex mutex_;
    std::unordered_map<int, T> data_;
};

}  // namespace hms::infrastructure::repositories
