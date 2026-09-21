| Observation/source | PO feedback item | Finding | Root requirement/rule | Source change | Affected definition domains | Potentially stale artifacts | Remediation | Decision | Trace status |
|---|---|---|---|---|---|---|---|---|---|
| M22 06C3C blocked source camera clear and SDL_GPU API | None | REC-001 | Port plan §9; migration RB-01–RB-04 | REC-CHANGE-001 (prior committed decision, no mutation here) | All false after targeted architecture review | None in definition; implementation mappings/tests need rerun | REC-RM-001 | None | TRACED |
| RTX bgfx color/depth/stencil probe exit 0 | None | REC-001 | Migration RB-01–RB-04 | REC-CHANGE-001 | All false | None in definition | REC-RM-001 | None | TRACED |
| M24 retail-first-tick upstream dependency | None | REC-001 | M22 source draw provider and port-plan original behavior | REC-CHANGE-001 | All false | None in definition | REC-RM-001 prerequisite; existing M24 acceptance preserved | None | TRACED |

The three observations share one renderer root cause. Original retail scene rendering remains unverified until M22; that is an implementation acceptance limit, not missing product authority.
