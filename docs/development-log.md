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
Defined the dash contract with automated tests before implementation, then created an engine-independent `FDashAbilityModel` for direction capture and the Ready, Dashing, and Cooldown state transitions. Added early dash cancellation and timing logic that handles frame steps crossing multiple state boundaries. Created `UDashComponent`, attached it to `AAshenStepCharacter`, cached the owning character, updated dash behavior each frame, and added the complete dash input action, mapping, and character input handler. Final Unreal testing corrected the input trigger to fire once when the action starts and clear movement input when it completes or is canceled.

### Verification

- Build/test performed: Added six Unreal Automation tests under `AshenStep.Dash` covering defaults, direction handling, state progression, early ending, grounded/airborne rules, cooldown, and large frame steps.
- Result: The red test contract was merged through PR #17 before the implementation; the completed ability was tested in Unreal with debug logs and its final input wiring correction is recorded on `feature/dash`.
- Screenshot, recording, commit, or profiling evidence: `18ec5a4` (test contract), `5e8c425` through `7c747de` (model, component, character, and input implementation), `d0df704` (final Unreal test and input correction).

### Obstacles and decisions

Frame timing can cross the end of both the dash and cooldown in one update, so leftover time is carried across state transitions. Dash rules were kept in an engine-independent model to make behavior deterministic and straightforward to test, while the Actor Component handles Unreal character movement and input integration.

### Next action

Merge `feature/dash`, capture durable screenshot or recording evidence, and tune dash distance, duration, cooldown, and animation feel.
