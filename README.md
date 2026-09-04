# ASDEVLAB
## Computational Astrophotography & Robotic Telescope Platform

ASDEVLAB is a personal research and engineering project focused on **improving astronomical imaging with affordable, small-pixel camera sensors through computation**.

The project combines a custom telescope-control architecture, embedded hardware, astronomical coordinate processing, camera acquisition, image processing, plate solving, and live observation into a single experimental platform.

The long-term objective is not simply to build another robotic telescope, but to investigate how **software and computational imaging can compensate for limitations of inexpensive cameras**—including noise, limited effective resolution, imperfect focus, and mechanical pointing errors.

---

## Project Motivation

High-quality astronomical cameras can be expensive, while many inexpensive USB/UVC and small-pixel sensors are readily available.

ASDEVLAB explores the following idea:

```text
Affordable / small-pixel camera
            │
            ▼
     Multi-frame acquisition
            │
            ▼
 Alignment / Registration
            │
            ▼
 Noise & SNR improvement
            │
            ▼
 Super-resolution / Reconstruction
            │
            ▼
 Focus-error correction
            │
            ▼
 Higher-quality astronomical image
```

The primary research direction is **computational astrophotography**, particularly for:

- Lunar imaging
- Planetary imaging
- Small-pixel camera enhancement
- Low-cost astronomical cameras
- Multi-frame image reconstruction
- Super-resolution
- Noise reduction and SNR enhancement
- Focus-error correction
- Astronomical image registration

---

# What I Developed

ASDEVLAB uses existing open-source projects as foundations where appropriate, but the project is not intended to be a simple collection of existing repositories.

The main contribution is the **custom system architecture and integration layer** connecting embedded control, astronomy, imaging, and observation into one platform.

### Original / custom development

- Designed the overall ASDEVLAB software architecture.
- Developed the C++ telescope-control core and service architecture.
- Developed the OnStepX communication layer using direct LX200-compatible TCP commands.
- Designed the custom `MountInterface` abstraction and mount adapter architecture.
- Developed motion, tracking, guiding, alignment, target, observation, astronomy, camera, imaging, focusing, plate-solving, and scheduling service layers.
- Developed the local astronomical catalog and target-resolution pipeline.
- Integrated astronomical coordinate computation through an astronomy abstraction layer.
- Developed REST APIs and a browser-based telescope-control interface.
- Added custom LX200 functionality for project-specific hardware such as the **Flip Mirror** and mount-mode handling.
- Designed the ASDEV_V1 hardware/pinmap integration for the ESP32-based telescope controller.
- Built an automated testing structure using CMake/CTest and multiple service, parser, API, and integration tests.
- Established an audit/refactoring workflow to keep the architecture modular and avoid uncontrolled technical debt.

---

# System Architecture

The project is intentionally divided into layers.

```text
                         ┌──────────────────────┐
                         │      Web UI           │
                         │  Observation Control  │
                         └──────────┬───────────┘
                                    │ REST
                                    ▼
                         ┌──────────────────────┐
                         │   ASDEVLAB Core       │
                         │  TelescopeCore        │
                         └──────────┬───────────┘
                                    │
             ┌──────────────────────┼──────────────────────┐
             │                      │                      │
             ▼                      ▼                      ▼
       Motion / Tracking      Target / Astronomy      Camera / Imaging
       Alignment / Guide     Catalog / Observation    Focus / Plate Solve
             │                      │                      │
             └──────────────────────┼──────────────────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │   Hardware Adapters  │
                         │ OnStep / Camera etc. │
                         └──────────┬───────────┘
                                    │
                     LX200 / TCP / Camera interfaces
                                    │
              ┌─────────────────────┴─────────────────────┐
              ▼                                           ▼
      ┌─────────────────┐                         ┌─────────────────┐
      │     OnStepX     │                         │    Camera       │
      │ ESP32 Firmware  │                         │ UVC / supported │
      └────────┬────────┘                         └────────┬────────┘
               │                                          │
               ▼                                          ▼
        Stepper Drivers                              Image Frames
        ALT / AZM axes
```

A key architectural principle is:

> **The UI should not contain telescope-control logic, and image-processing components should not own mount-control logic.**

The user-facing application communicates with `TelescopeCore`, which coordinates the individual services.

---

# Telescope & Embedded Control

The current hardware target is an ESP32-based Alt-Az telescope controller.

### Main mount

- Dual-axis **ALT/AZM** configuration
- ESP32 controller
- TMC2209 stepper drivers
- Custom ASDEV_V1 pinmap
- Step/Direction motor control
- Hardware UART communication with TMC2209
- Wi-Fi communication with the host computer
- LX200-compatible telescope protocol

### Additional hardware

The architecture also supports project-specific auxiliary hardware:

- Rotator
- Focuser
- Flip Mirror servo
- Home/limit sensors
- Status indication

The current firmware configuration enables the Flip Mirror feature and defines its camera/eyepiece positions.

---

# Direct LX200 Integration

ASDEVLAB communicates directly with the telescope firmware through **LX200-compatible commands over TCP**.

The project deliberately avoids depending on a Windows-specific ASCOM layer for its primary control path.

```text
ASDEVLAB
   │
   │ LX200 commands
   │ TCP
   ▼
OnStepX
   │
   ▼
ESP32
   │
   ▼
Stepper Drivers
   │
   ▼
ALT / AZM Mount
```

The mount communication layer provides a typed C++ interface above the raw protocol.

Examples of supported operations include:

- Mount status
- Coordinate readback
- GOTO
- Sync
- Tracking
- Manual movement
- Stop / abort
- Park
- Home
- Guiding
- Limits
- Focuser control
- Rotator control
- Flip Mirror control
- Mount capability detection
- Mount-mode detection

Project-specific commands have also been added where required by the hardware architecture, rather than forcing every function through the original LX200 feature set.

---

# Astronomy & Target Pipeline

ASDEVLAB maintains its own local astronomical catalog instead of requiring an external planetarium application.

The current catalog is divided into:

```text
Catalog
├── Deep Sky Objects
├── Stars
├── Solar System
├── Small Bodies
└── Satellites
```

The software architecture separates:

1. Catalog data
2. Target selection
3. Coordinate resolution
4. Observation context
5. Mount motion

Conceptually:

```text
User selects target
        │
        ▼
   CatalogEngine
        │
        ▼
    TargetService
        │
        ▼
 CoordinateResolver
        │
        ▼
 Resolved Target
        │
        ▼
   MotionService
        │
        ▼
    Mount Adapter
        │
        ▼
       OnStepX
```

The astronomy layer provides a controlled abstraction for astronomical calculations. A libnova-based provider is currently used for supported solar-system calculations, while static catalog coordinates are used where appropriate.

The architecture is intentionally designed so additional coordinate/ephemeris providers can be introduced without changing the higher-level telescope-control services.

---

# Computer Vision & Computational Imaging

This is the main future research direction of ASDEVLAB.

The project is being developed toward a computational imaging pipeline capable of improving data from inexpensive astronomical cameras.

### Current foundation

The repository already contains components for:

- Camera acquisition
- Frame delivery
- Image processing
- Live stacking
- Plate solving
- Star-pattern processing
- Image registration workflows
- Focus control
- Astronomical target positioning

OpenLiveStacker is included as an image-processing/live-stacking foundation and is kept conceptually separate from the ASDEVLAB telescope-control core.

### Planned research

The next development direction focuses on:

#### 1. Multi-frame super-resolution

Instead of relying only on the native resolution of a small-pixel sensor:

```text
Frame 1 ─┐
Frame 2 ─┤
Frame 3 ─┤
Frame 4 ─┤──► Sub-pixel alignment ─► Reconstruction
Frame N ─┘
```

The objective is to investigate whether multiple slightly different observations can be combined to recover spatial information beyond that available from an individual frame.

#### 2. Focus-error correction

Planetary and lunar imaging is particularly sensitive to small focus errors.

The planned pipeline will investigate:

```text
Defocused / imperfect frames
            │
            ▼
   Blur / focus estimation
            │
            ▼
 Image restoration / reconstruction
            │
            ▼
        Sharper image
```

The goal is not to claim that software can recover information that was never captured, but to investigate computational recovery of useful high-frequency detail from partially degraded observations.

#### 3. SNR enhancement

Multiple frames can also be used to improve the signal-to-noise ratio while preserving astronomical structure.

Potential processing stages include:

- Frame quality estimation
- Registration
- Outlier rejection
- Noise reduction
- Multi-frame integration
- Detail reconstruction
- Super-resolution
- Sharpening / restoration

---

# Automated Positioning & Plate Solving

ASDEVLAB is designed to connect image information back to telescope control.

The intended feedback loop is:

```text
Telescope moves
      │
      ▼
Camera captures image
      │
      ▼
Plate solving / star pattern matching
      │
      ▼
Measured sky position
      │
      ▼
ΔRA / ΔDEC or equivalent pointing error
      │
      ▼
Mount correction
      │
      ▼
New image
      │
      └──────────────► repeat
```

This creates a foundation for closed-loop astronomical positioning, where the camera is not only an imaging device but also a source of positional feedback.

---

# Live Observation

The project includes a browser-based control interface.

The intended user experience is:

```text
User
 │
 ▼
Web Browser
 │
 ▼
ASDEVLAB Web API
 │
 ├── Telescope control
 ├── Target selection
 ├── Coordinates
 ├── Tracking
 ├── Focuser
 ├── Rotator
 ├── Flip Mirror
 └── Camera / imaging functions
```

The UI is intentionally kept thin. Business logic remains in the C++ backend.

---

# Software Components

## ASDEVLAB Core

Location:

```text
software/asdevlab/
```

Main services:

- `MotionService`
- `TrackingService`
- `GuideService`
- `AlignmentService`
- `CameraService`
- `ImageService`
- `PlateSolverService`
- `SchedulerService`
- `FocusService`
- `TargetService`
- `ObservationService`
- `AstronomyService`

Core infrastructure includes:

- `TelescopeCore`
- `MountInterface`
- `OnStepMountClient`
- `LX200Parser`
- `LX200ConnectionManager`
- `CatalogEngine`
- `CoordinateResolver`
- REST API layer
- State machine
- Safety service

---

## OpenLiveStacker

Location:

```text
software/LiveStacker/
```

OpenLiveStacker provides an existing open-source foundation for:

- Live stacking
- Camera integration
- Plate solving
- Image processing
- Stretching
- Hot-pixel removal
- Calibration
- EAA workflows

ASDEVLAB does not treat OpenLiveStacker as the telescope-control brain. Its role is primarily image acquisition and processing.

This separation allows the project to experiment with new computational-imaging algorithms without coupling them directly to mount-control code.

---

## OnStepX

Location:

```text
firmware/OnStepX/
```

OnStepX is used as the embedded telescope-control foundation.

ASDEVLAB intentionally does **not** attempt to duplicate low-level motor-control functionality already provided by the firmware.

Instead, the project concentrates custom development above and around that layer:

```text
Low-level motor control
        │
     OnStepX
        │
        ▼
LX200 communication
        │
     ASDEVLAB
        │
        ▼
Observation / Imaging / CV
```

This separation keeps the project focused on the research problem rather than unnecessarily rewriting a mature motor-control subsystem.

---

# Repository Structure

```text
ASDEVLAB/
│
├── firmware/
│   └── OnStepX/
│       └── ESP32 telescope firmware
│
├── software/
│   ├── asdevlab/
│   │   ├── include/
│   │   ├── src/
│   │   │   ├── astronomy/
│   │   │   ├── catalog/
│   │   │   ├── hardware/
│   │   │   │   └── mount/
│   │   │   ├── services/
│   │   │   └── web/
│   │   ├── data/
│   │   │   └── catalog/
│   │   ├── test/
│   │   └── sim/
│   │
│   └── LiveStacker/
│       ├── include/
│       ├── src/
│       ├── test/
│       └── www-data/
│
├── external/
│   └── libnova/
│
├── docs/
│   ├── architecture
│   ├── protocol documentation
│   ├── mount audits
│   ├── catalog audits
│   └── implementation reports
│
└── .github/
    └── workflows/
```

---

# Build

## Requirements

The ASDEVLAB core uses:

- C++17
- CMake
- GCC/Clang
- libcurl
- POSIX threads
- libnova source included in the project
- Arduino/ESP32 toolchain for firmware

### Build ASDEVLAB Core

From:

```text
software/asdevlab/
```

run:

```bash
cmake -S . -B build
cmake --build build
```

Run the test suite:

```bash
ctest --test-dir build --output-on-failure
```

Run the core application:

```bash
./build/asdevlab_core_app
```

Run the web application:

```bash
./build/asdevlab_web_app
```

The mount-test web interface is served locally by the ASDEVLAB web application.

---

# Mount Connection

The current default OnStepX network configuration is:

```text
Host: 192.168.0.1
Port: 9998
Protocol: LX200-compatible TCP
```

The connection can also be configured through environment variables:

```bash
export ASDEVLAB_ONSTEP_HOST=192.168.0.1
export ASDEVLAB_ONSTEP_PORT=9998
export ASDEVLAB_ONSTEP_TIMEOUT_SECONDS=2
export ASDEVLAB_ONSTEP_HEARTBEAT_SECONDS=5
```

Then run:

```bash
./build/asdevlab_web_app
```

---

# Testing & Engineering Practice

Testing is treated as part of the architecture rather than an afterthought.

The repository contains tests for:

- Core services
- Motion
- GOTO / solve loops
- Target handling
- Astronomy services
- Catalog engine
- LX200 parser
- LX200 socket transport
- OnStep adapter
- Mount APIs
- Alt-Az behavior
- Focuser API
- Rotator API
- Flip Mirror API
- Integration paths
- Proxy injection

The project also uses:

- CMake/CTest
- AddressSanitizer during debugging
- Offline simulation fixtures
- Regular architecture audits
- Protocol audits
- Catalog audits
- Refactoring passes

A design goal is to avoid:

- Spaghetti code
- Hidden business logic in the UI
- Unnecessary abstraction layers
- Uncontrolled directory fragmentation
- Technical debt caused by rapid feature additions

---

# Current Status

### Implemented foundation

- [x] ESP32-based Alt-Az mount integration
- [x] Custom ASDEV_V1 hardware pinmap
- [x] Direct LX200/TCP communication
- [x] Typed mount abstraction
- [x] GOTO and tracking control
- [x] Mount status and coordinate handling
- [x] Target catalog and search
- [x] Astronomy service abstraction
- [x] libnova-based astronomical calculations for supported bodies
- [x] Camera service architecture
- [x] Image service architecture
- [x] Plate-solver service architecture
- [x] Focuser API
- [x] Rotator API
- [x] Flip Mirror API
- [x] REST/Web control interface
- [x] Offline simulation infrastructure
- [x] Automated C++ test suite
- [x] Architecture and protocol audit documentation

### In progress / research direction

- [ ] Robust multi-frame astronomical image registration
- [ ] Camera-specific noise characterization
- [ ] Computational SNR enhancement
- [ ] Multi-frame super-resolution
- [ ] Focus-error estimation and correction
- [ ] Planetary/lunar image reconstruction
- [ ] Closed-loop plate-solve pointing correction
- [ ] ML-based image-quality assessment
- [ ] ML-assisted astronomical image restoration

These items represent the research direction and should not be interpreted as already-completed features unless corresponding implementations are present in the codebase.

---

# Research Direction

The central research question of ASDEVLAB is:

> **How far can computational imaging improve the practical performance of inexpensive astronomical cameras?**

Rather than relying entirely on more expensive optics or sensors, the project investigates the complementary approach of extracting more useful information from the data that an affordable sensor can already capture.

The intended research pipeline is:

```text
Low-cost sensor
      │
      ▼
Raw / video frames
      │
      ├── Sensor noise characterization
      ├── Frame quality estimation
      ├── Star / feature registration
      ├── Sub-pixel alignment
      ├── Multi-frame integration
      ├── SNR enhancement
      ├── Super-resolution
      ├── Focus-error correction
      └── Image restoration
              │
              ▼
      Enhanced astronomical image
```

This makes ASDEVLAB both an engineering platform and a potential experimental framework for **Computer Vision, Machine Learning, and Computational Imaging research**.

---

# Open-Source Foundations vs. Original Work

ASDEVLAB intentionally builds on established open-source technologies where they provide reliable low-level functionality.

Examples include:

- **OnStepX** — telescope/motor-control firmware foundation
- **OpenLiveStacker** — live-stacking and astronomical imaging foundation
- **libnova** — astronomical calculation foundation

The purpose of using these projects is to avoid unnecessarily reinventing mature infrastructure.

The original work in ASDEVLAB is concentrated on the layer that connects these components and turns them into a coherent experimental platform:

```text
Open-source foundations
        │
        ▼
Custom ASDEVLAB architecture
        │
        ├── Telescope control
        ├── Astronomy / target pipeline
        ├── Camera integration
        ├── Web API
        ├── Hardware integration
        ├── Automated testing
        └── Computational-imaging research
```

Third-party licenses and attribution requirements remain applicable to their respective components.

---

# Project Philosophy

ASDEVLAB follows several principles:

1. **Build on mature open-source foundations instead of duplicating them.**
2. **Keep project-specific logic in clearly defined layers.**
3. **Separate telescope control from image processing.**
4. **Treat camera data as a computational resource, not only as a final image.**
5. **Use testing and audits continuously during development.**
6. **Prefer simple architecture over excessive abstraction.**
7. **Document limitations honestly and distinguish implemented functionality from research goals.**

---

# Vision

The long-term goal is to create a compact and affordable astronomical imaging platform in which computational methods compensate for some of the limitations of inexpensive cameras.

The desired end state is:

```text
Affordable Camera
        +
Robotic Telescope
        +
Astronomical Computing
        +
Computer Vision
        +
Computational Imaging
        +
Machine Learning
        │
        ▼
High-quality planetary / lunar observations
```

ASDEVLAB is therefore evolving from a telescope-control project into a broader **computational astrophotography research platform**.
