#pragma once

#include "DataSnapshot.h"

#include <string>

namespace hms::persistence {

class IDataStore {
public:
    virtual ~IDataStore() = default;

    virtual bool load(DataSnapshot& snapshot, std::string& error) const = 0;
    virtual bool save(const DataSnapshot& snapshot, std::string& error) const = 0;
};

}  // namespace hms::persistence
