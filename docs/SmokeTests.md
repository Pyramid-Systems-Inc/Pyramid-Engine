# Smoke Tests

Use this quick render sanity check after local changes.

## Prerequisites

- Build with tests enabled:

```powershell
cmake --preset vs2022-debug-tests
cmake --build --preset build-debug-tests-clean
```

## Run Smoke Runner

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run-smoke.ps1 -BuildDir build-clean -Config Debug -DurationSeconds 5
```

## Expected Output

Typical successful run:

```text
[INFO] Launching BasicGame: ...\BasicGamed.exe
[PASS] BasicGame remained running for 5 seconds.
[INFO] Launching BasicRenderingExample: ...\BasicRenderingExample.exe
[PASS] BasicRenderingExample remained running for 5 seconds.
[PASS] BasicGame log file contains startup markers.
[PASS] Smoke test completed successfully.
```

## Notes

- The smoke runner is intended for local desktop environments with GPU/window support.
- CI uses `ctest` for automated validation; GUI smoke runs are not required in CI.

