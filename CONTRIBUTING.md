# Contributing to Hydra-ESP

Thanks for wanting to put time into this. Firmware for a niche board with two radios and a homemade web UI has a lot of edge cases, so any help is genuinely useful — code, docs, testing on hardware I don't own, whatever.

## Before you do anything

Scope check first: this is a **security research / education** firmware. PRs that make it easier to point at networks you don't own, or that remove/weaken the legal notice, consent prompts, or anything like that, will get closed. Everything else is fair game.

## Where things go

- **Discussions** → questions, "does this work on ESP32-WROVER", feature ideas you're not sure about yet, general chat. Not everything needs to be an issue.
- **Issues** → actual bugs, crashes, a specific attack module not working the way the README says it should.
- **Pull Requests** → code changes. Small and focused beats one giant PR touching six modules.

If you're not sure which one, Discussions is the safe default — I'll move it to an issue if it turns out to be one.

## Reporting a bug

Tell me:
- board you're on (DevKit V1, WROOM-32, whatever)
- ESP-IDF version
- what you did, what you expected, what actually happened
- serial monitor output if it crashed/panicked — this is the single most useful thing you can paste in

Vague "it doesn't work" reports are hard to act on. A panic backtrace or a screenshot of the web UI stuck in a weird state does a lot more.

## Sending a PR

1. Fork it, branch off `main`.
2. Keep the change focused — one feature/fix per PR.
3. Match the existing code style (see below). Don't reformat files you're not otherwise touching, it makes the diff unreadable.
4. Build it and flash it to real hardware before opening the PR if you can. "Compiles" and "works" are not the same thing on this project.
5. Say what you tested it on (board + IDF version) in the PR description.
6. Update the README if you're adding/changing something user-facing (new attack, new default, changed web UI flow, etc).

## Code style

- C (ESP-IDF components) and C++ (BLE/NimBLE bits) mixed, follow whatever the file you're editing is already doing.
- Snake_case for functions/variables, matches the existing components.
- Doxygen-style comments on public functions (see `Doxyfile` / existing headers) — doesn't need to be exhaustive, just enough that someone else can tell what a function does without reading the whole body.
- No hardcoded credentials, IPs, or debug leftovers in the diff.

## Adding a new attack module

If you're adding a new Wi-Fi or BLE attack:
- Keep it under `main/wifi` or `main/bt`, matching how existing modules are laid out.
- Wire it into the web UI + API the same way the existing attacks are (check `components/webserver` and `data/app.js` for the pattern).
- Add a section to the README's Attacks list — what it does, what it needs (connected client? nearby AP only? etc), any known limitations (MFP resistance, etc).
- If it's genuinely novel (not just a variant of an existing technique), a short note on the mechanism is appreciated so reviewers aren't reverse-engineering intent from raw frame-crafting code.

## Testing

There's no hardware-in-the-loop CI (can't exactly deauth a runner), so testing here means real hardware, real radios, your own network. Please don't test against anything you don't own or don't have written permission to test.

## Licensing

This repo is GPL-3.0. By sending a PR you're agreeing your contribution is licensed under the same terms.

---

Questions before you start something big? Drop it in [Discussions](https://github.com/SameerAlSahab/Hydra-ESP/discussions) first so we're not duplicating effort or building something that's going to get rejected on scope.
