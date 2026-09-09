# Day 6 Phase 3 — Component, input, and montage TDD

## Scope and current red state

These tests describe behavior, not a solution implementation. Phase 3 connects the
existing attack model and configuration to the character, input, and montage.
No traces, damage application, combos, or input buffering are required yet.

At starting commit `124033f`, the character creates the melee component and exposes
its getter, but the component remains a shell. The character header also forward
declares `UMeleeAttackComponet` instead of `UMeleeAttackComponent`; fix that typo
before interpreting compiler failures as missing gameplay behavior.

The new component test file is intentionally compile-red until these operations
exist on `UMeleeAttackComponent`:

- `RequestAttack()` returns whether a request actually starts an attack.
- `GetState()` is a const query returning `EMeleeState`.
- `CanRegisterHits()` is a const query reporting hit permission.
- One reflected, editable `FMeleeAttackData` property supplies the component's
  configuration. Its name and visibility are not prescribed. The fixture locates
  it by struct type, populates it before BeginPlay, and never modifies private
  attack state. A public configuration setter is not required just for testing.

These names are test vocabulary, not required renames for an equivalent API you
choose. Adapt calls to equivalent production entry points without weakening
assertions. Do not duplicate the model's state in the component just to satisfy
the queries, and do not implement production behavior inside the test fixture.

## Automated coverage added

Run `AshenStep.MeleeAttack.Phase3.Component`:

- `CharacterOwnership`: two spawned characters each own a distinct component;
  the getter returns the attached component and only one exists per character.
- `SafeDefaults`: configuration is editable and harmless by default; beginning
  play does not open a hit window or start an attack.
- `UnconfiguredRequestsAreHarmless`: repeated requests from a living,
  unconfigured character return false, preserve Ready, disallow hits, and do not
  change the attacker's health.
- `MissingAnimationDoesNotLatchAttack`: structurally valid configuration on a
  living bare C++ character with no animation instance cannot start an attack or
  leave the state in WindUp/Active. Repeated attempts remain safe.
- `MissingOwnerRejectsSafely`: an ownerless component rejects a request without
  crashing or opening a hit window.

The fixture creates and destroys a private transient world. It does not use or
replace `GWorld`, alter the open editor level, or modify class defaults. It dispatches
actor BeginPlay so health initialization and component subscriptions are exercised.
Because `AAshenStepCharacter` is abstract, the fixture spawns the concrete
`AMeleeAttackTestCharacter` subclass. It inherits gameplay behavior unchanged and
adds no mesh, animation assets, or simulated combat logic.
World lifecycle APIs are documented in [Epic's UWorld reference](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/Engine/UWorld).

Run `AshenStep.MeleeAttack.Model` for the two additional rule-level tests:

- `BusyInputPreservesEveryPhase`: repeated start requests in WindUp, Active, and
  Recovery preserve phase and hit permission; a new press works after completion.
- `RestartAfterInterruptionInEveryPhase`: interrupt each attacking phase, repeat
  cleanup, deliver late close/completion signals while idle, then perform a full
  fresh swing. The Boolean interruption result follows the current contract:
  true when an attack ended, false when already Ready.

## What those tests do NOT prove

A bare character has no animation assets. A transient empty `UAnimMontage` is
used only to satisfy structural data validation, never as proof of playback.
The rejection tests are deliberately multi-missing-prerequisite safety checks;
they do not isolate each guard. An implementation that always rejects requests
could pass these tests. The positive-path cases below are therefore mandatory
before Phase 3 is accepted.

Model interruption tests do not prove the component bound a montage callback.
They also do not prove stale callbacks from an older swing cannot interrupt a
newer one. Real input routing, animation playback, callback wiring, and owner
death cleanup require the asset-backed cases below.

## Asset-backed red-to-green acceptance cases

These are pending editor/functional scenarios, NOT executable or passing tests.
Record a result and evidence for each. Later automate them against a dedicated
fixture map, retaining the same assertions.

### Fixture baseline

- A possessed playable character with initialized health, the melee component,
  a skeletal mesh, and a compatible animation Blueprint/slot.
- One real, nonlooping attack montage of known duration, with positive play rate,
  positive damage and radius, and usable socket names.
- A visible/readable attack state and a way to observe montage playback position.
- Once authored, the attack input action is assigned, mapped in the active input
  mapping context, and routed through the normal player-input path.
- Start with a fresh fixture or reset fully between cases. Re-establish a passing
  baseline after every deliberately invalid configuration, so failures cannot be
  explained by an unrelated missing prerequisite.

### P3-01 — Positive request and normal completion

Call the real component request entry point once with the baseline fixture.
Expect success, WindUp, no hits, and the configured montage playing at the configured
rate. Let the montage finish naturally. Expect Ready and no hits within montage
duration/play-rate plus blend-out and a small bounded frame allowance. A second
fresh request must succeed. Health must not change merely from requesting an attack.

### P3-02 — One physical press, no accidental held-input repeats

Use the mapped input, not a direct component call. One press produces one swing.
Hold the button beyond completion: no second swing should begin without another
press. Release/repress starts the next swing. Rapid extra presses while busy must
not reset montage position, start another instance, or queue an attack for later.
Test keyboard/controller bindings that are actually supplied. Do not hard-code a
particular key as the behavior requirement.

### P3-03 — Every rejection guard in isolation

First prove P3-01 passes. Then change exactly one prerequisite at a time: remove
the montage, set damage/radius/play-rate to zero or negative, clear either socket
name, remove the animation instance, or kill the owner through `UHealthComponent`.
Each request must return false, start no montage, preserve Ready, and allow no hits.
Restore the valid baseline and prove a new request succeeds (use a fresh living
character for the death case; no resurrection feature is required).

### P3-04 — Playback failure rollback

On a living, otherwise configured character with a valid animation instance,
assign a montage that cannot play, such as an empty montage with no animation.
Structural validation alone must not count as successful playback. The request
must fail, leave Ready, and allow no hits. Restore the playable montage and prove
the next request succeeds. Do not fake the playback result in a copied state model.

### P3-05 — Actual interruption and duplicate cleanup

Start the baseline montage, then stop it through the animation instance before
natural completion. Exercise both immediate stop and nonzero blend-out. Combat
must return to Ready and close the damage window when interruption is reported,
without waiting for an end notify that will never arrive. Let both blending-out
and ended callbacks run; cleanup must remain harmless. A fresh request must work.
Repeat interruption during Active and Recovery once Phase 4 notifies exist; those
later-phase checks are deferred, not already covered by the component tests.

### P3-06 — Old callback cannot cancel a newer attack

Interrupt swing A with a nonzero blend-out. Once Ready, start swing B before A's
final callback would otherwise arrive. Deliver/allow A's remaining cleanup callbacks.
They must not stop B, reset B's state, or close B's future hit window. Observe the
actual montage instance and state, not only a callback counter.

### P3-07 — Death and component teardown

During a playing attack, apply lethal damage through the real health component.
Expect cleanup to stop the attack, close hit permission, and reject further
requests while dead. In a separate run, destroy the character during playback;
allow the world to advance beyond the old montage's end. There must be no stale
callback access, crash, or leaked attack state into a newly spawned character.

### P3-08 — Regression and frame-rate checks

Repeat start, spam, completion, and interruption at 30/60/120 FPS. Confirm movement
and dash still work afterward, and the chosen attack/dash conflict rule behaves
consistently. There must be no Blueprint runtime errors. Run `AshenStep.Health`
and `AshenStep.Dash` regressions after the input integration.

## Suggested TDD order

1. Fix the character's existing forward-declaration typo.
2. Make ownership/default tests build and pass while adding the expected component
   entry point and read-only queries; preserve the single authoritative model.
3. Make asset-free rejection tests pass without manufacturing successful attacks.
4. Configure the real montage and make P3-01 pass before adding input actions.
5. Wire input and verify P3-02; then verify rejection and cleanup P3-03 through P3-07.
6. Run model, configuration, health, and dash suites, then the frame-rate cases.

No Unreal compilation or editor test run was performed on the authoring host.
The actual model implementation and all eight model test bodies were compiled and
run in a temporary C++17 harness with minimal Unreal type/assertion substitutes:
145 assertions passed. This validates model logic only, not Unreal headers,
reflection, world lifecycle, component behavior, or animation/input integration.
Missing APIs are expected TDD red; fixture/compiler errors after those APIs exist
must be fixed, not dismissed as expected red. No test is skipped or marked passing
because the necessary feature is absent.
