# Copilot instructions for DarkishTower

This repository is an Arduino/ESP32 game (Darkish Tower tribute) that runs on an Adafruit Feather S2 + ST7789 LCD. The agent should focus on small, well-scoped edits (bug fixes, screen logic, sounds, assets) and preserve hardware-aware constraints.

Key areas (big picture)
- Entry point: `main.ino` — initializes `gGameState` and runs the main loop calling `gGameState.update()`.
- Game orchestration: `src/game_state.*` — holds single global `gGameState` with subsystems: `InputManager`, `SoundManager`, `DisplayManager`, `WorldState`.
- UI model: `src/screen_layout.h` + `src/display_manager.*` — screens are described by `ScreenLayout` (title, bitmap, text lines, options, selection). `DisplayManager::setDesiredLayout()` is used to push UI changes; `update()` diffs current vs desired layout and repaints efficiently.
- Screens & flow: `src/game_screens.*` — implements concrete `GameScreen` objects and helpers (screen lists, auto-confirm screens). Screens call `gGameState` to push/pop/swap screens.
- Game rules/state: `src/world_state.*` — player data, inventories, and game logic helpers.
- Input & sound: `src/input_manager.h` (debounced, pullups, ReadInputState()) and `src/sound_manager.*` (esp timers, DAC playback). Sound is asynchronous and may block selection when `isWaitingForSong()` is true.

Practical editing guidance
- Prefer small single-file changes. Touching `SoundManager` or `DisplayManager` affects timing/hardware; keep edits conservative and test on-device.
- UI changes: construct a `ScreenLayout` literal (see `game_screens.cpp` examples) and call `gGameState.displayManager.setDesiredLayout(...)`. Use `addInfo()` / `addOption()` patterns provided in `ScreenLayout`.
- Navigation: selection state lives in `DisplayManager` (mSelection). Screens should not directly mutate display internals; use `displayManager.setSelection()` or `gGameState.pushScreen()` / `popScreen()`.
- Input: `InputManager::setup(upPin,downPin,selectPin,count)` — pins are set in `game_state.cpp` with `inputManager.setup(1,3,7,5);`. Debounce count matters; avoid changing without testing physical buttons.

Build / test / debug notes (how to run)
- This is an Arduino/ESP32 sketch. Use Arduino IDE, PlatformIO, or `arduino-cli` to build and flash to a Feather S2. The repository does not contain PlatformIO or build config files — use your normal ESP32/Feather S2 workflow.
- Serial prints: `Serial.begin(115200)` in `main.ino`. Use serial output to observe startup messages and SoundManager debug prints (e.g., "Starting playback!").
- On-device checks: verify DAC pin (DAC1) and pin 21 power enable for speaker are correct for your board. Sound timing relies on esp timers; crashing or freezes often indicate timer misuse or blocking loops.

Conventions & patterns to follow
- Minimal global state: the project uses one global `gGameState` (declared in `game_state.cpp`). New subsystems should be added to `GameState` to follow the same pattern.
- Screen description over imperative drawing: prefer creating `ScreenLayout` objects and let `DisplayManager` diff/repaint for consistent visuals.
- Async sound: call `soundManager.play(sound, waitForSong)` instead of blocking playback.
- Debounce & consume: `InputManager::ReadInputState()` returns presses and consumes them (pressDetected cleared). Code should call `ReadInputState()` each loop and not expect persistent press flags.

Integration points & files to inspect for changes
- `main.ino` — startup and loop timing
- `src/game_state.*` — flow control and how screens are started/confirmed
- `src/game_screens.*` — canonical examples of UI flows and usage of `ScreenLayout` and sounds
- `src/display_manager.*` and `src/screen_layout.h` — UI rendering and layout primitives
- `src/sound_manager.*` and `src/assets/sounds.h` — how sounds are defined/played
- `src/world_state.*` — game rules, Player behavior, inventory operations

Quick examples from repo
- Create a simple one-screen layout:
  ScreenLayout s = {"TITLE", tile_bitmap_victory, {{"Line1", ST77XX_WHITE}}, {{"OK",0}}, 0};
  gGameState.displayManager.setDesiredLayout(s);
- Play a sound without blocking:
  gGameState.soundManager.play(darktower_snd, true);

When editing, run these checks
- Keep edits small and compile locally with Arduino/PlatformIO. Watch for esp timer and FreeRTOS usage in `sound_manager.cpp` for concurrency issues.
- Preserve usages of `String` (Arduino type) across UI objects; mixing std::string will not work without extra changes.

If unsure or missing hardware
- Ask for a small unit test or simulation: e.g., a small `main` replacement that calls `DisplayManager::update()` with mocked bitmaps, or add Serial-only paths that exercise logic without hardware.

If you modify this guidance, keep it concise and reference the concrete files above.

---
Please review any unclear areas (hardware pins, assumed board variant) or tell me if you want the file expanded with build commands for PlatformIO or arduino-cli.

Additional actionable items (implemented suggestions)

Build & flash examples
- arduino-cli (adjust the --fqbn for your install/board):

```bash
# compile
arduino-cli compile --fqbn <YOUR_BOARD_FQBN> .
# flash (replace /dev/ttyUSB0 with your serial port)
arduino-cli upload -p /dev/ttyUSB0 --fqbn <YOUR_BOARD_FQBN> .
# monitor serial
arduino-cli monitor -p /dev/ttyUSB0 -b 115200
```

- PlatformIO (minimal template below). From the project root run `pio run` to build and `pio run -t upload` to flash.

Example minimal `platformio.ini` (create at repo root if you use PlatformIO):

```ini
[env:adafruit_feather_s2]
platform = espressif32
board = adafruit_feather_s2
framework = arduino
monitor_speed = 115200

# If your board id differs, replace `adafruit_feather_s2` with the correct board id.
```

Safe-edit checklist: SoundManager & DisplayManager
1. Make one small change at a time. These subsystems interact with hardware timers and peripherals.
2. Add Serial debug traces before/after critical sections (e.g., start/stop playback in `sound_manager.cpp`) and verify messages on serial at 115200.
  - The code already prints messages such as "Starting playback!" and "Finished playback!" — mirror that style.
3. Preserve semaphore/esp_timer use. If editing `SoundManager::onTimer()` or `play()`, don't remove the `xSemaphoreTake`/`xSemaphoreGive` calls or the periodic timer unless you also handle synchronization.
4. Avoid blocking loops in timer callbacks. Timer callbacks should be short — move long work to update() or a task.
5. When changing `DisplayManager` drawing logic, prefer creating/updating `ScreenLayout` objects and using `setDesiredLayout()` rather than direct drawing from screens.
6. After change, verify these smoke checks on-device:
  - Board boots to "Booting" title shown by `DisplayManager::setup()`.
  - Buttons (pins 1/3/7) debounce correctly (use `inputManager.setup(1,3,7,5)` counting) and selection moves.
  - Playing a short sound: call `gGameState.soundManager.play(beep_snd, false);` and confirm serial logs + audible output.
7. If you must change pin assignments or DAC usage, document the change in `main.ino` comments and in this doc.

Merging note
- I searched the repo for existing agent docs (common filenames like `AGENT.md`, `.github/copilot-instructions.md`) and found none prior to this file — so no merge was necessary. If you maintain additional internal agent guidance, provide it and I'll fold important items in.
