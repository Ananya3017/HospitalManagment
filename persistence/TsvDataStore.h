#pragma once

#include "IDataStore.h"

#include <filesystem>
#include <string>

namespace hms::persistence {

class TsvDataStore final : public IDataStore {
public:
    explicit TsvDataStore(std::filesystem::path dataDirectory);

    bool load(DataSnapshot& snapshot, std::string& error) const override;
    bool save(const DataSnapshot& snapshot, std::string& error) const override;

private:
    std::filesystem::path dataDirectory_;

    [[nodiscard]] std::filesystem::path filePath(const std::string& fileName) const;
};

}  // namespace hms::persistence
