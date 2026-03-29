# Cortex Command - Work Intake Playbook

Manager-focused template for opening tasks that are easy to implement and verify.

## 1) Ticket template (copy/paste)

Use this format when assigning work:

```md
Title:

Player-visible outcome:
- What should the player notice after this is done?

Current behavior:
- What happens now?

Scope:
- In scope:
- Out of scope:

Likely subsystem(s):
- (pick from AGENTS_FEATURE_OWNERSHIP.md)

Repro / validation path:
1.
2.
3.

Acceptance criteria:
- [ ]
- [ ]

Risk notes:
- Save compatibility?
- Mod compatibility?
- Performance risk?
```

## 2) How to choose subsystem owner quickly

- "Cannot launch / black screen / crash on boot" -> Boot/Window/Frame owners.
- "My mod item does not load" -> Preset/DataModule owners (+ content author).
- "Weapon/actor behavior wrong" -> Entity/Activity owners.
- "Input binding or controller issue" -> UInput/InputMapping owners.
- "Menu/settings flow issue" -> MenuMan/Settings GUI owners.
- "Script callback not firing" -> LuaMan/Bindings owners.
- "Visual artifact only" -> Renderer/PostProcess owners.

## 3) Scoping rules that prevent churn

- Define one player-visible behavior per ticket when possible.
- Do not combine rendering + gameplay + content edits in one task unless required.
- If touching `Data/*.rte`, call out whether change is balance/content or engine logic.
- If touching engine core (`Main.cpp`, managers), require a short regression checklist.

## 4) Required validation checklist per PR

Minimum checks:

1. Build passes (`ninja -C build-dev`).
2. Game boots to menu (`./build-dev/CortexCommand`).
3. One in-game scenario/activity run for the changed area.
4. No new warnings in relevant logs (`LogConsole.txt`, `LogLoading.txt`).

Additional checks by area:

- Input changes: keyboard + mouse + at least one gamepad path.
- Module/content changes: official module path + one mod path.
- Activity/gameplay changes: pause/resume/restart flow.
- Render/window changes: windowed + fullscreen and resolution change.

## 5) Priority model

- P0: crash/data loss/hard lock/startup failure.
- P1: major gameplay blocker or severe regression in common path.
- P2: functional bug with workaround, or moderate UX degradation.
- P3: polish/refactor/non-blocking improvement.

## 6) Definition of done (manager view)

A task is done when all are true:

- behavior matches the "player-visible outcome"
- acceptance criteria are checked
- validation steps are reproducible by another person
- no unexplained side effects in logs
- change list is scoped to declared subsystem(s)

## 7) Suggested task size

- Small: 1-3 files, one subsystem, same-day.
- Medium: 4-10 files, one major subsystem plus UI/content touches.
- Large: multi-subsystem architecture or behavior overhaul; split into milestones.

For large work, request an implementation plan first, then approve milestone by milestone.
