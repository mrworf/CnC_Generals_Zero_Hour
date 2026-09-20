#include "zh/foundation/numeric.h"

#include <cmath>
#include <limits>

namespace zh::foundation {

Int32 checked_float_to_i32(float value, IntegralRounding rounding)
{
    if (!std::isfinite(value)) {
        throw NumericError("cannot convert non-finite float to Int32");
    }
    double rounded = 0.0;
    switch (rounding) {
    case IntegralRounding::truncate: rounded = std::trunc(value); break;
    case IntegralRounding::floor: rounded = std::floor(value); break;
    case IntegralRounding::ceiling: rounded = std::ceil(value); break;
    case IntegralRounding::nearest:
        if (!is_round_to_nearest()) {
            throw NumericError("nearest conversion requires FE_TONEAREST");
        }
        rounded = std::nearbyint(value);
        break;
    }
    if (rounded < static_cast<double>(std::numeric_limits<Int32>::min()) ||
        rounded > static_cast<double>(std::numeric_limits<Int32>::max())) {
        throw NumericError("float is outside Int32 range");
    }
    return static_cast<Int32>(rounded);
}

bool is_round_to_nearest() noexcept
{
    return std::fegetround() == FE_TONEAREST;
}

ScopedRoundToNearest::ScopedRoundToNearest() : previous_mode_(std::fegetround())
{
    if (previous_mode_ == -1 || std::fesetround(FE_TONEAREST) != 0) {
        throw NumericError("unable to establish FE_TONEAREST");
    }
}

ScopedRoundToNearest::~ScopedRoundToNearest()
{
    if (previous_mode_ != -1) {
        std::fesetround(previous_mode_);
    }
}

} // namespace zh::foundation
