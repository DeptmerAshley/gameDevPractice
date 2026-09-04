# Day 6 melee acceptance specification

These scenarios define the remaining black-box integration and functional tests. They are intentionally expressed in terms of observable gameplay behavior rather than component internals. Convert each scenario into an automated map-backed test when its required animation and test assets exist.

## Input and montage lifecycle

- Given a living player with a valid configured attack, one attack input starts the configured montage and enters wind-up.
- Given an attack in wind-up, active, or recovery, repeated input does not restart or stack the montage.
- Given a dead player, attack input is rejected and no montage begins.
- Given invalid attack configuration or an unavailable animation instance, attack input is rejected without changing authoritative state.
- Given an interrupted or blending-out montage in any attacking phase, cleanup returns the attack to ready and closes the damage window.

## Authored hit-window timing

- Before the begin-window notify, traces cannot register gameplay hits.
- The begin-window notify transitions wind-up to active and initializes trace history.
- During active, trace sampling may register gameplay hits.
- The end-window notify transitions active to recovery and stops trace sampling immediately.
- Notify signals received twice or out of order do not create an invalid attack state.
- Completion, interruption, and owner death all restore ready state and clear swing-scoped data.

## Swept hit detection

- During active, the trace sweeps between previous and current weapon socket positions rather than testing only the current point.
- The attacker is ignored by its own trace.
- A valid target crossing the swept weapon path is detected.
- A target outside the swept path is not detected.
- Actors that do not expose the damage-receiver contract are ignored safely.
- Debug visualization changes presentation only and never changes hit results.

## Per-swing deduplication

- Multiple active trace frames overlapping the same actor apply damage exactly once.
- Multiple hit components belonging to one actor still apply damage exactly once.
- Two different valid actors can each receive damage once during the same swing.
- Completion or interruption clears the hit set so the same target can be damaged by the next swing.
- A rejected or unsuccessful damage application does not produce successful-hit feedback.

## Damage contract

- A successful melee hit sends the configured requested damage, damage type, instigator, source, and world-space impact point through the target's health component.
- Damage is applied through the reusable health contract without requiring a practice-dummy type.
- The health component's returned applied amount determines whether the hit counts as successful.
- Overkill, dead targets, and invalid damage continue to obey the Day 5 health rules.
- A lethal melee hit preserves the established health-change-then-death event order.

## Feedback separation

- A successful applied hit may trigger target reaction, attacker camera response, sound, and visual feedback exactly once.
- A trace contact that applies no damage does not trigger full successful-hit confirmation.
- Feedback does not mutate health independently or determine authoritative attack state.

## End-to-end acceptance

- One valid input produces one complete ready-to-wind-up-to-active-to-recovery-to-ready attack.
- The attack damages the practice dummy once, updates its health bar, and preserves Day 5 damage and death events.
- The same configured component can attack another actor exposing the health contract without changing the target-specific implementation.
