Mock harness for DarkishTower

This mock is a small host-side simulator that exercises DisplayManager and SoundManager flows without hardware.

How to build (Linux):

```bash
# from repository root
g++ -std=c++17 mock/mock_main.cpp -o mock/darkish_mock -lpthread

# run
./mock/darkish_mock
```

What it does
- Prints a boot screen, then a MAIN MENU with options
- Emulates a few navigation events (selection up/down)
- Calls a mocked sound play which prints and completes after a short delay

Notes
- This is purely for local development and quick logic tests. It does not compile against the Arduino APIs and is intentionally minimal.
- If you want a more feature-complete host harness (e.g., to test DisplayManager::update() internals), I can add a small adapter that compiles a subset of the project's headers with mocked Arduino types.

CI and convenience
- A GitHub Actions workflow `/.github/workflows/mock-ci.yml` will build and run both mocks on push/PR.
- Locally you can run both mocks with the convenience script:

```bash
./mock/run_all.sh
```

Additional harnesses

1) Closer-to-source harness (uses real `ScreenLayout` type)

```bash
# compile the host adapter that uses project headers
g++ -std=c++17 mock/mock_main_full.cpp -o mock/darkish_mock_full -lpthread

# run
./mock/darkish_mock_full
```

2) If you want to build/flash to hardware via PlatformIO, a `platformio.ini` example is present at the repo root. Adjust `board` if your Feather S2 variant differs.
