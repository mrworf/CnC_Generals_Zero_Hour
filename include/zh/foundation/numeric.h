#pragma once

#include "zh/foundation/types.h"

#include <cfenv>
#include <stdexcept>

namespace zh::foundation {

class NumericError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class IntegralRounding { truncate, floor, ceiling, nearest };

Int32 checked_float_to_i32(float value, IntegralRounding rounding);
bool is_round_to_nearest() noexcept;

class ScopedRoundToNearest {
public:
    ScopedRoundToNearest();
    ~ScopedRoundToNearest();
    ScopedRoundToNearest(const ScopedRoundToNearest&) = delete;
    ScopedRoundToNearest& operator=(const ScopedRoundToNearest&) = delete;

private:
    int previous_mode_;
};

} // namespace zh::foundation
