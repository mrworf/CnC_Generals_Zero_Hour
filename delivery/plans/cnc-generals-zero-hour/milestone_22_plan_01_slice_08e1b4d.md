# M22 08E1B4D: generated AudioEventRTS provider closure

## Boundary revalidation

The source `SinglePlayerLoadScreen` owns the ambient `AudioEventRTS` member and
its returned handle: it adds the event only near the end of `init`, removes it
in its destructor, and force-plays the selected mission briefing from
`moveWindows`.  Those calls are not independently reachable: the same owner
also requires its complete named WND graph, mapped images, video/display,
`GameInfo`, and mode lifetime.  That final composition remains 08E1B4.

This slice therefore closes only the prerequisite value/provider boundary:
the original `AudioEventRTS` constructor/copy used by generated mission and
ambient values, plus the existing project-owned `AudioManager` virtual
provider that receives bounded add/remove/force-play requests.  It does not
parse audio INI, load media, open a device, synthesize output, or claim retail
audio fidelity.

## Delivered behavior

Each of two generated campaign generations assigns a source
`AudioEventRTS("GeneratedBriefing")` to the currently published source
`Mission`, verifies its force-play request reaches the provider, and rejects
an empty briefing.  A source-generated ambient event is copy-constructed,
exercises injected add failure with no active state, retries to receive a
normal handle, rejects a special handle without mutation, and removes the
returned handle to zero active entries before re-entry.  The provider records
only generated event names and handles; it retains no caller pointers.

There is no fabricated `SinglePlayerLoadScreen` call and no direct access to
its private ambient owner.  The provider itself is unpublished by the existing
headless teardown.  The final B4 composition must prove the actual source
member call ordering after all prerequisite owners are present.

## Validation

Focused GCC/Clang source, identity, and AudioEventRTS provider-removal checks;
all six final-tree non-GPU/non-LAN suites; strict host sanitizer leak checks,
physical Vulkan, serial LAN, ledger/diff, evidence, and exact scoped commit
are required.
