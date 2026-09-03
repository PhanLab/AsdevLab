# ASDEVLAB Telescope Core

This folder contains the coordination layer for the ASDEVLAB platform.

## Architecture

The system is intentionally split so each layer has one job:

- OnStepX firmware: low-level mount control over serial/CAN, step/dir, tracking rate, PID, and basic motion primitives.
- asdevlab core: the single orchestration brain. It accepts high-level commands such as goto, park, guide, autofocus, plate solve, and scheduling, then coordinates camera and imaging work.
- LiveStacker: image processing module. It consumes frames from the camera pipeline and handles stacking/stretching/hot-pixel removal. It should not own mount control.
- Web/Mobile UI: thin client that calls the asdevlab API and renders state. It should not contain mount business logic.

## Current services

- MotionService
- TrackingService
- GuideService
- AlignmentService
- CameraService
- ImageService
- PlateSolverService
- SchedulerService
- FocusService
- AstronomyService

## Build and test

```bash
cmake -S . -B build
cmake --build build
./build/asdevlab_core_app
ctest --test-dir build --output-on-failure
```

## Astronomy backend

ASDEVLAB owns the astronomy workflow used for resolving target coordinates and observation context. The initial implementation uses a vendored libnova backend under external/libnova/ for Sun, Moon, and Mars calculations. The libnova sources are built directly from the repository and are not exposed outside the astronomy wrapper layer.

## Mount transport configuration

The mount stack now uses a direct LX200-over-TCP transport that is configured through environment variables. Prefer the OnStep-specific names, with legacy mount names still supported for compatibility:

- ASDEVLAB_ONSTEP_HOST (fallback: ASDEVLAB_MOUNT_HOST)
- ASDEVLAB_ONSTEP_PORT (fallback: ASDEVLAB_MOUNT_PORT)
- ASDEVLAB_ONSTEP_TIMEOUT_SECONDS (fallback: ASDEVLAB_MOUNT_TIMEOUT_SECONDS)
- ASDEVLAB_ONSTEP_HEARTBEAT_SECONDS (fallback: ASDEVLAB_MOUNT_HEARTBEAT_SECONDS)

Example:

```bash
ASDEVLAB_ONSTEP_HOST=192.168.0.1 \
ASDEVLAB_ONSTEP_PORT=9998 \
ASDEVLAB_ONSTEP_TIMEOUT_SECONDS=2 \
ASDEVLAB_ONSTEP_HEARTBEAT_SECONDS=5 \
./build/asdevlab_core_app
```

## Mount API coverage

The mount adapter now exposes a typed LX200/OnStepX surface through the existing Mount API instead of leaking raw strings outside the mount layer.

Implemented capabilities include:

- Status and packed-status parsing via `:Gu#` with a graceful fallback to `:GU#`
- Tracking, guide, park, home, site, time, PEC, limits, focuser, and flip-mirror readback
- Time values are simplified to host-local date/time plus sidereal time when the mount reports it; ASDEVLAB does not maintain separate UT1/UTC-offset/DUT1/GPS abstractions
- Alignment, tracking-rate, guide-rate, slew-rate, manual-motion, and auxiliary control entry points
- Capability detection so unsupported firmware features return typed `Unsupported` responses instead of failing silently

## Real mount integration test

A hardware integration test is available but is opt-in to avoid breaking the default CI run. Set the environment variables below before running the test binary directly:

```bash
ASDEVLAB_RUN_MOUNT_INTEGRATION_TESTS=1 \
ASDEVLAB_ONSTEP_HOST=192.168.0.1 \
ASDEVLAB_ONSTEP_PORT=9998 \
./build/asdevlab_mount_integration_test
```

## Offline simulation

The sim directory is for offline tests and fixture data so autofocus and plate-solving loops can be exercised without hardware.

## Python strategy

Python is treated as tooling and integration glue rather than part of the core C++ library.

- Helper scripts live under scripts/ and tools/.
- The proxy helper is a standalone utility for bridging an OnStepX endpoint to the C++ core.
- The browser UI remains a thin client; it should not embed mount logic.
