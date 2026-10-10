# Security Review Report — headless qualification

## Metadata

Project: cnc-generals-zero-hour. Inspected base: c5e1fcb34f5c25151f583f6fa9e3b88833c82ab7.
Review time: 2026-10-10T17:16:04.985114+00:00. Role: application security definition review.
Scope: HQ-01–07, unchanged R02/R04/R05 storage/input/privacy boundaries, generated runner and
retail startup probe, and corresponding nonvisual UX. Commands: rg/sed source inspection,
Git provenance, YAML and document integrity checks. No security tests or private input opened.
Existing dirty implementation is preserved and not certified. Review runs in this chat.

## Executive Summary

No material security-definition conflict is introduced by the finite headless qualification.
The contract keeps input parsing, read-only assets, separated write authority, explicit failure,
owner retirement and private-diagnostic redaction mandatory. No confirmed vulnerability is
established by this bounded review. This PASS is not a whole-game security certificate.

## Scope and Methodology

Inspect source and tests only enough to assess the amendment's trust boundaries. Relevant
implementation evidence: NativeUserStorage.cpp::validateRootOwnership/validateWriteTarget and
no-follow directory access; tools/run_data_audit.py::snapshot/main; GameEngine.cpp native startup;
original typed parsing and generated fault fixtures referenced by HQ-05. Correlate these with
contract/UX requirements. Existing implementation coverage and exploitability are not inferred
from normative wording or historical tests.

## Threat Model

- Exposed interfaces: local developer CLI/options, generated fixtures, locally supplied original
  data and writable user/cache paths. H1 creates no remote listener or multiplayer session.
- Sensitive assets: original proprietary content, private root/selectors, user data, and process
  ownership/state. Generated test data is safe to retain; retail bytes/derivatives are not.
- Boundaries: caller-selected paths → immutable asset mounts; untrusted file lengths/chunks →
  original parsers; local user storage → OS filesystem; private probe output → public evidence.
- Attacker position: someone able to supply malformed local content or manipulate local paths.
  No new remote account, privilege, tenant, credential or Internet-service model is introduced.

## Findings Summary

No confirmed or likely vulnerability demonstrated within scope. No CVSS score or exploit PoC
is assigned without such a finding. Unexecuted tests are limits, not proof of security defects.

## Detailed Findings

No vulnerability entry. Definition observation SEC-HQ-OBS-01: the existing data audit wrapper
accepts only the earlier stage0–4 indexing result shape. A future original-startup probe must
supply a reviewed bounded/redacted status contract; it cannot reuse that old PASS as proof of
post-init reachability. Evidence: tools/run_data_audit.py::main allowlist and N2-original-data.md.
HQ-06 explicitly demands a different semantic outcome, with no private raw output. This is
pending integration already covered by R07, not an unresolved security-product decision.

## Exploit Chains

None established. No active exploitation or destructive filesystem tests were performed.

## Hardening Recommendations

No new generic hardening obligations are added. Required existing controls remain: reject
malformed lengths/paths before unsafe allocation or traversal; use separate explicit user
storage; preserve every allocation failure/retry pair and safe ownership; redact private
errors and outputs; reject incomplete execution. Runtime tests must cover those controls.

## Positive Security Observations

- NativeUserStorage validates prospective write roots against asset ownership and uses no-follow
  access for directory traversal. This supports the definition's separate write boundary; a
  complete race/symlink audit was not performed and is not implied.
- The existing audit wrapper compares snapshots and allowlists public output rather than
  relaying stderr/exception text. HQ-06 preserves that privacy rule for the new native probe.
- H1 is generated and bounded: no proprietary fixture distribution and no new network exposure.
- HQ-05 reports failed ticks as failures and retires failed worlds; no silent partial success.
- Unwritable disposable caches fall back to memory, while required writable user-data failure
  remains explicit. The contract does not blur cache availability with permission to alter assets.

## Assumptions and Limitations

Normal local OS-user permissions are the established deployment authority. Availability of the
user's private data remains an external acceptance prerequisite. This review does not inspect
those roots, scan current dependency vulnerabilities, certify all map/network parsing, or prove
runtime cleanup. Later LAN and release-security obligations remain unchanged.

## Appendix

Definition validation: HQ-01–07 contain positive, malformed/missing-input, privacy and recovery
rules; UX-HQ-01–05 preserve public-only diagnostics; no authority grants retail writes or vendor
patches. Source and exact artifact hashes are retained in results/security.yaml. No security
exceptions or risk waivers were accepted.
