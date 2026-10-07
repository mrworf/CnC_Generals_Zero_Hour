# Milestone N6: Linux LAN matches

## Objective

Two Linux instances discover/join and complete synchronized matches.

## User/System Outcome

Two Linux instances discover/join and complete synchronized matches.

## Scope

IPv4 discovery/direct connection, session lifecycle, map transfer, original protocol and deterministic multiplayer.

## Explicit Exclusions

No GameSpy/Internet matchmaking, mixed Windows/Linux promise or new game protocol.

## Source Requirements

R11 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N5 accepted before implementation consumes its output.
- PRE-08: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Port socket/time services while preserving source-established wire encodings.
- Run two-process join/map-transfer/start/play/finish and disconnect/retry paths.
- Reject malformed/oversized packets and safely handle duplicate/lost messages.
- Demonstrate determinism with synchronized simulation checkpoints.

## UX Constraints

Preserve source-established UI/gameplay behavior. Render loading/error states truthfully;
never claim a scene or operation succeeded when required content or output is missing.
N0/N1 have developer-facing diagnostics; they do not redefine player-facing UX.

## Architecture / Security Constraints

Stock dependencies/public APIs only. Retail roots are read-only, diagnostics redacted,
no proprietary distributions. No extra privileges or network accounts are introduced.
Validate externally supplied lengths/paths and preserve allocator/thread ownership.

## Interfaces and Compatibility

Zero Hour internal interfaces may change; no legacy MSVC/internal Windows ABI mandate.
Preserve source asset formats, externally meaningful encodings and intended behavior.
External dependencies use their supported interfaces; do not publish custom SDK APIs.

## Acceptance Criteria

- [ ] Two local Linux processes complete a match with matching checkpoints.
- [ ] Discovery/direct connection and map transfer work without corrupting supplied assets.
- [ ] Disconnect, invalid packets and repeated session shutdown remain safe.

## Required Validation

Local UDP integration and failure fixtures, deterministic two-process harness and sanitizer checks. Physical-subnet discovery is separately evidenced if available.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

Local loopback discovery may need a deterministic discovery harness; real direct connection and match completion remain mandatory.
