# Development Log

Use one entry per development session. Keep entries short and evidence-based.

## 2026-8-29 — Add Health Component

### Goal
Create a Health Component Object that is attached to a player. This will allow for editable fields in unreal that allow the user to manipulate max and current health.

### Result
New Actor component UHealthComponent was created and successfully added to a player component. It is a sub object that appears under components in the player blueprint.

### Verification

- Build/test performed:
- Result:
- Screenshot, recording, commit, or profiling evidence:

### Obstacles and decisions

Learning to code in Unreal but thats it.

### Next action

Create a health bar, damage, gain health, etc.

## 2026-8-30 — Day 3: Damage, Healing, and Health UI

### Goal
Turn the health foundation into a usable gameplay system by adding damage, healing, change notifications, and a visible health bar.

### Result
Expanded `UHealthComponent` with damage and healing behavior, including health clamping, death handling, and a health-change broadcast for dependent systems. Added a Blueprint-accessible health component getter to the player character and connected a UI health bar to the component. Added automated coverage for initialization, damage, overkill, healing, overhealing, post-death behavior, broadcasts, and character-component access.

### Verification

- Build/test performed: Unreal Automation tests were added under `AshenStep.Health`.
- Result: Test coverage was merged through PR #16; the health gameplay work was merged through PR #15.
- Screenshot, recording, commit, or profiling evidence: `6c99af4` (tests), `b50c0b3` through `c24331f` (implementation and UI), merge commits `f604a8c` and `3599939`.

### Obstacles and decisions

Restored an accidental Blueprint change before continuing. Health changes are broadcast from the component so UI and future gameplay systems can react without owning the health rules.

### Next action

Build the first expressive movement ability: a directional dash with clear timing, cooldown, and input behavior.

## 2026-8-31 — Day 4: Directional Dash

### Goal
Implement a testable directional dash and connect it to the playable character and Enhanced Input system.

### Result
Defined the dash contract with automated tests before implementation, then created an engine-independent `FDashAbilityModel` for camera-relative direction capture, fallback-forward behavior, and the Ready, Dashing, and Cooldown state transitions. Added early dash cancellation and timing logic that carries leftover frame time across state boundaries.

Created `UDashComponent`, attached it to `AAshenStepCharacter`, and connected a dedicated Enhanced Input action. The component now performs swept physical movement, terminates on blocking collisions, exposes distance, duration, cooldown, airborne use, momentum contribution, maximum momentum bonus, and exit-speed retention as Blueprint defaults, and suppresses normal movement application while retaining player input intent. Pre-dash horizontal velocity contributes a directionally projected and capped momentum bonus. Normal dash completion restores a capped exit velocity when movement input initiated the dash, while collision termination skips forward exit velocity.

### Verification

- Build/test performed: Added six Unreal Automation tests under `AshenStep.Dash` covering defaults, direction resolution, state progression, early termination, grounded/airborne rules, cooldown boundaries, repeated requests, and a large frame spanning dash and cooldown. Rebuilt and manually tested the physical dash in Unreal during implementation.
- Result: Directional and fallback dashes move the player; downhill slopes work; blocking geometry and small ledges terminate the dash; momentum and maximum bonus values were tuned; movement-to-dash-to-movement smoothing was implemented with retained exit velocity. Uphill slopes remain unsupported, and final frame-rate, visualization, and environmental verification is backlogged.
- Screenshot, recording, commit, or profiling evidence: `18ec5a4` (test contract), `5e8c425` through `7c747de` (model, component, character, and input), `d493419` through `5e572bb` (physical movement and momentum), `6d7948c` (movement-input integration), `9f6515b` through `9b9e153` (exit-velocity transition and fixes).

### Obstacles and decisions

Dash rules were kept in an engine-independent model so state and direction behavior remain deterministic, while the Actor Component owns Unreal movement, collision, momentum, and exit-velocity integration. Direct swept movement made collision termination straightforward but does not reproduce Character Movement's complete walking step-up behavior; small ledges currently count as blocking collisions. Uphill slope support requires projecting grounded dash direction onto the walkable floor plane or integrating more of Character Movement's walking behavior.

### Backlog

- Project grounded dash movement onto walkable uphill slopes.
- Decide whether small walkable ledges should terminate the dash or use step-up handling.
- Prevent physical-distance overshoot when a frame crosses the dash-duration boundary.
- Add a displacement or speed curve for tunable ease-in/ease-out while preserving total distance.
- Add debug direction, path, state, and collision visualization.
- Verify physical distance and transition behavior at 30, 60, and 120 FPS.
- Add automated coverage for physical displacement, momentum contribution/capping, collision termination, and exit velocity.
- Capture durable video or screenshot evidence.
- Clean remaining whitespace and minor Boolean/accessor style issues.
- Add invulnerability frames later if required by the combat design; excluded from the current Day 4 completion scope.

### Next action

Move to Day 5 and extend the existing health foundation with structured damage context, damage-received and death events, temporary damage targets, and the remaining zero, negative, lethal, repeated, and post-death verification. Return to the Day 4 backlog during the next movement-polish pass.

## 2026-9-3 — Day 5: Structured Damage and Practice Target

### Goal
Extend the reusable health system with structured damage information, gameplay events, complete boundary-case coverage, and a temporary in-world target for manually verifying the damage loop.

### Result
Added `FDamageContext` with requested damage amount, instigator, source, world-space hit location, and damage type. Updated `UHealthComponent` to accept structured damage, return the amount actually applied, and broadcast health-changed, damage-received, and death events while keeping health rules independent from animation and presentation.

Added `ADamageTestTarget` and `BP_PracticeDummy` with a visible mesh, reusable health component, world-space health bar, and a temporary keyboard-triggered damage action. Updated the player health-bar integration so both the player and practice dummy supply their own health values to the reusable widget.

### Verification

- Build/test performed: Expanded the `AshenStep.Health` automation contract to cover structured and amount-only damage, event payloads and order, environmental damage, zero and negative damage, repeated hits, lethal overkill, and post-death rejection. Manually exercised the practice dummy and player health displays in Play mode.
- Result: The practice dummy accepts test damage and updates its overhead health bar; the player health bar initializes and responds to health changes. Valid hits report applied damage, lethal damage reaches zero and broadcasts death once, and post-death damage is rejected.
- Screenshot, recording, commit, or profiling evidence: `439037f` through `e61493b` (damage context, gameplay events, and automated coverage), `f450903` through `47cb6b7` (practice target, world placement, health-bar integration, and manual damage setup).

### Obstacles and decisions

The shared health widget originally attempted to read an unassigned health-component reference when instantiated for the practice dummy. Responsibility for obtaining health values and responding to health events was moved to each owning actor, leaving the widget focused on display. The damage-context fields were made Blueprint-writable so Blueprint test callers can construct the complete context.

### Next action

Run the complete `AshenStep.Health` automation suite once more, perform the final branch review, and merge Day 5. Then begin Day 6 with one data-driven melee attack using trace-based hit detection and the structured damage contract.

## 2026-09-10 — Day 6: Core Data-Driven Melee Attack

### Goal

Connect one configured attack from player input through animation-authored hit timing, swept collision detection, per-attack duplicate prevention, and the existing structured health/damage contract.

### Result

Implemented `FMeleeAttackModel` with authoritative `Ready -> WindUp -> Active -> Recovery -> Ready` transitions, request rejection, interruption, and permission queries. `FMeleeAttackData` holds editable montage, play rate, damage amount/type, trace radius, and socket names with harmless default damage and basic configuration validation.

Attached `UMeleeAttackComponent` to the player and bound the melee Enhanced Input action using `Started`. Attack requests validate the owner, living health component, mesh, sockets, animation instance, and configuration before starting the model and montage. Playback failure resets the model; montage-end handling completes recovery or performs fallback cleanup. A custom Notify State requests the active window's begin/end transitions.

Added active-only sphere sweeps at the weapon tip, base, and midpoint using previous/current socket positions. The queries ignore the attacker and currently use `ECC_Visibility`. History initializes at the accepted window opening and is invalidated on window close, sampling failure, or montage cleanup. Toggleable socket markers and a connecting line remain independent of collision detection.

Centralized sample sweeping/logging and debug drawing in helpers, made position history private, and delegated hit permission to the model. A private `ActorsHit` collection is shared across samples and ticks and resets after an accepted new attack request. Eligible targets are living actors with `UHealthComponent`, without a practice-dummy-specific cast. Targets are recorded before calling damage. Each context carries configured damage/type, the attacker as instigator and source, and the sweep impact point.

### Verification

- Martin previously reported a successful Unreal build and passing Phase 3 checks/tests. Captured transition logs showed accepted `Ready -> WindUp -> Active -> Recovery -> Ready`, hits permitted only in `Active`, and normal `TryEndRecovery` cleanup.
- During this session, Martin reported working active-window debug markers, collision detection of `BP_PracticeDummy`, and duplicate-suppression behavior. Tip-first logs are expected from sample order and do not independently verify base/midpoint-only coverage.
- After fixing the inverted health-validity guard, Martin reported that melee hits reduce dummy health, display damage taken, and trigger the existing death announcement. These are user-reported Play-mode observations, not an independently executed integration test run.
- Assistant checks: source/diff reviews; earlier position-tracking work passed eight mocked tick scenarios. No Unreal toolchain was available on this host, and the final melee/health regression suites have not been rerun here.
- Evidence: `bc94bfa` (transition diagnostics), `ce78177` (safe position history/debug toggle), `b9ccbb1` (three-point tracing), `bfa068c` (refactor), `84f8be1` (deduplication), `49a8cc5` (damage context), and `bf94018` (health-guard correction).

### Obstacles and decisions

- Missing Blueprint melee defaults were resolved after closing Unreal and rebuilding the Editor target. Montage input was initially blocked by the unassigned melee action in player Class Defaults.
- Corrected shadowed socket variables, a tip/base endpoint mix-up, and an empty sweep `if`. Sweep results are processed regardless of the Boolean blocking-hit return so overlap results are not discarded.
- Invalid/dead receivers use `continue`, not `return`, so one ineligible result does not abort processing the remaining results. The health guard rejects invalid components before dereferencing them.
- Martin chose to wrap Day 6 at the core-melee milestone and prioritize fundamentals. Sound, camera shake, hit-stop, and additional audiovisual feedback are deferred unless needed later; the existing damage display and death events remain in use. This supersedes the original plan's immediate presentation requirement.
- Interruption/blend-out hardening and its final active-window verification were separately deferred earlier. Existing montage-end fallback is implemented, but that is not proof of immediate hit shutdown during interrupted blend-out.

### Backlog and remaining verification

- Rerun `AshenStep.MeleeAttack.Model`, `AshenStep.MeleeAttack.Configuration`, `AshenStep.MeleeAttack.Phase3.Component`, and `AshenStep.Health` in Unreal against the final branch. Add executable deduplication and damage-context integration coverage; the acceptance specification is not itself an executable test.
- Confirm exactly one damage event per target per attack, a second attack hitting the same target again, two targets each hit once, misses, scenery, dead targets, lethal overkill, and no repeated death announcements. Verify damage with debug drawing disabled and inspect the context event payloads.
- Independently test base/midpoint coverage and tracing at varied frame rates. Three samples and straight inter-frame sweeps are the current approximation, not proven complete coverage for every weapon length or fast rotation. Revisit a dedicated combat collision channel and blocker behavior.
- Finish interruption/owner-death cleanup, immediate hit-window shutdown, and clearing swing-scoped hit tracking during cleanup (currently reset on the next accepted attack). Confirm movement/dash interactions and no stale callbacks affecting later attacks.
- Use the returned applied-damage amount for success diagnostics and any future feedback. It is currently captured but unused; the melee log still reports detection, not confirmed applied damage.

### Next action

Wrap this core-melee session with the working input-to-damage loop documented. Keep optional presentation work deferred. Before treating the original full Day 6 acceptance checklist as complete or merging the feature, run the remaining core regression checks and address the recorded cleanup gaps. No merge is performed by this documentation update.
