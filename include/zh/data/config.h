#pragma once

#include "zh/foundation/platform.h"

#include <filesystem>
#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::data {

class DataError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class DataUsageError : public DataError {
public:
    using DataError::DataError;
};

struct DataArguments {
    std::optional<std::filesystem::path> zero_hour_root;
    std::optional<std::filesystem::path> generals_root;
    std::optional<std::string> language;
};

struct DataSelection {
    std::filesystem::path zero_hour_root;
    std::filesystem::path generals_root;
    std::string language;
};

DataArguments parse_data_arguments(const std::vector<std::string_view>& arguments);
DataSelection resolve_data_selection(
    const DataArguments& arguments,
    const foundation::EnvironmentLookup& environment = {});
void print_data_selection(const DataSelection& selection, std::ostream& output);

} // namespace zh::data
