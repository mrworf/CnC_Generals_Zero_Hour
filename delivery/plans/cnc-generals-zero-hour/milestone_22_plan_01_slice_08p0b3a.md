# M22 plan 01 slice 08P0B3A: exact client RNG transaction prerequisite

## Goal and boundary

Before tree geometry publication, expose a bounded preview/commit/discard
transaction for the existing six-word GameClient random stream. Native
`W3DTreeBuffer::addTree` consumes that stream for random scale (when
requested) and sway type before fallible vertex/index and device work. A
failed tree admission must not consume those values or change the next
successful tree's scale/sway sequence. The existing GameLogic and audio
streams are separate and must remain untouched.

Preserve the exact native client random algorithm, integer and real result
order, seed words and `InitRandom` behavior. Add no new entropy source,
tree-specific pseudo-random substitute, persistent gameplay owner or
threaded RNG route. Copy the six-word state; preview integer/real draws by
advancing only that local copy; discard it on failure or commit it with a
no-throw six-word assignment after every fallible asset, geometry, wrapper,
Recording and registry-storage step has succeeded. No global RNG mutation
is permitted during preview. The owner must exclude intervening client RNG
draws between copy and commit. Do not publish a tree or open the physical
factory here.

## Validation and commit

Generated seed controls compare uninterrupted client draws with a
copy/preview/commit or discard/retry sequence for integer and real values,
including source tree draw order. Verify a failed speculative transaction leaves the
next successful client sequence identical to a fresh equivalent generation,
while GameLogic/audio results and seed state are unchanged. Run six complete
builds and canonical nonretail suites, focused strict host LSan, physical
Vulkan display/bootstrap/map controls, serial LAN 4/4 all six, ledger and
diff checks on final source. Commit one slice:
`delivery: M22 08P0B3A preview client tree RNG`.
