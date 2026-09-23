# M22 08E1A: base load-screen presentation service order

Generated headless owners prove the source base `LoadScreen::update(Int)`
order: service Windows, update WindowManager, update Display, then draw
Display, across two generations.  Missing singleton presentation providers
fail closed; existing headless provider-removal and window/allocation cleanup
prove zero ownership.  Mode-specific layouts/data, retail input and pixels
remain deferred to E1B/E1C.

Acceptance: six non-GPU/non-LAN suites passed 213/213; focused host
`detect_leaks=1` passed 3/3 in GCC and Clang; public Vulkan probe passed 1/1;
serial LAN passed 4/4 in all six configurations; ledger and diff checks pass.
