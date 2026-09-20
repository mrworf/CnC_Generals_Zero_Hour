#pragma once

#include <string>

namespace zh::original_support {

// Exercises the dependency-closed original support boundary and returns its runtime witness.
std::string run_ww_support_probe();

// True only when the original chunk reader rejects a truncated header without mutating output.
bool rejects_truncated_chunk();

} // namespace zh::original_support
