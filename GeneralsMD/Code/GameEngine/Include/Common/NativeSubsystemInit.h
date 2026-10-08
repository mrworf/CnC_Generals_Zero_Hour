// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/SubsystemInterface.h"
#include <memory>
// Take ownership before converting a name, publishing a borrowed singleton,
// invoking init/INI callbacks or growing the final registry. Factory/name
// arguments at the call boundary must not allocate after owner acquisition.
template<class SUBSYSTEM, class LOAD>
void initOwnedSubsystem(SUBSYSTEM*& reference, const char* name, SUBSYSTEM* subsystem,
                        LOAD&& load) {
    std::unique_ptr<SUBSYSTEM> candidate(subsystem);
    if (!subsystem || !TheSubsystemList || !name) throw ERROR_BAD_ARG;
    SUBSYSTEM* prior = reference;
    const SubsystemPublication publication{&reference, prior,
        [](void* slot, void* previous) noexcept {
            *static_cast<SUBSYSTEM**>(slot) = static_cast<SUBSYSTEM*>(previous);
        }};
    reference = subsystem;
    try {
        TheSubsystemList->initializeSubsystem(subsystem, AsciiString(name),
            [&] { load(subsystem); }, publication);
    } catch (...) {
        // Original Object/Drawable cleanup calls its parent's singleton. Keep
        // that identity valid until cleanup returns, then restore without calls.
        candidate.reset();
        reference = prior;
        throw;
    }
    candidate.release(); // Registry acquired the one ownership unit.
}

template<class SUBSYSTEM>
void initSubsystem(SUBSYSTEM*& reference, const char* name, SUBSYSTEM* subsystem,
                   Xfer* xfer, const char* path1 = nullptr,
                   const char* path2 = nullptr, const char* directory = nullptr) {
    initOwnedSubsystem(reference, name, subsystem,
        [&](SUBSYSTEM*) {
            TheSubsystemList->loadDefinitions(path1, path2, directory, xfer);
        });
}
