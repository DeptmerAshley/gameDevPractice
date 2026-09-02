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
