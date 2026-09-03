# ASDEVLAB OnStepX capability audit

## Scope

This report audits the current ASDEVLAB host software implementation against the stated goal of controlling an OnStepX mount over LX200-compatible transport. The audit is based on the code paths in the current workspace, including the mount adapter, transport layer, parser layer, motion services, catalog engine, and telescope core.

## Executive summary

The current ASDEVLAB implementation is a partially wired telescope control stack rather than a complete turnkey mount controller. It does have a real transport path to an OnStepX-style mount, a typed mount abstraction, and a service layer that can dispatch many LX200 commands and parse many response formats. However, the implementation is still incomplete for reliable real-world operation:

- Basic command dispatch and status parsing are present.
- GOTO, sync, tracking, park, home, abort, and manual guide commands are implemented in the adapter layer.
- High-level workflow services exist, but the system is still more of a command skeleton than a full astronomy workflow engine.
- Alignment, robust recovery, and true astronomical coordinate handling are not implemented as a complete user workflow.
- The current code does not yet look like a complete Seestar-style autonomous telescope experience.

## 1. OnStepX communication layer audit

### Transport and connection architecture

Current execution path:

1. [software/asdevlab/src/telescope_core.cpp](software/asdevlab/src/telescope_core.cpp) creates the mount client.
2. [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) uses a typed mount abstraction.
3. [software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp](software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp) manages connection state.
4. [software/asdevlab/src/hardware/mount/lx200_socket_transport.cpp](software/asdevlab/src/hardware/mount/lx200_socket_transport.cpp) sends raw LX200 commands over a TCP socket.
5. [software/asdevlab/src/hardware/mount/http_transport.cpp](software/asdevlab/src/hardware/mount/http_transport.cpp) contains an alternate HTTP transport implementation, but it is not the active path for the main OnStepMountClient flow.

### Communication stack findings

- Active path: TCP socket transport via Lx200ConnectionManager.
- Alternate path: HTTP transport via curl exists in code, but is not wired into the main mount client control path.
- Connection startup sends initialization commands for site/time/UTC settings from environment variables.
- Reconnect behavior exists in the connection manager and transport.
- Timeout handling exists through socket timeouts and curl timeouts.
- Heartbeat exists through a periodic ":GU#" query loop.

### Command support matrix

| Feature area | LX200 syntax | Implemented in | Called from | Current status |
|---|---|---|---|---|
| Mount status query | :Gu# / :GU# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | readMountStatus(), getStatus() | Partial |
| RA query | :GR# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | readRightAscension(), readCoordinates() | Working |
| Dec query | :GD# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | readDeclination(), readCoordinates() | Working |
| Alt/Az query | :GA# / :GZ# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | getOrientation() | Partial |
| GOTO (RA/Dec) | :Srhh:mm:ss# + :Sd+dd*mm:ss# + :MS# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::goto_target(), MountInterface::gotoTarget() | Partial |
| GOTO (Alt/Az) | :Saalt# + :Szaz# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MountInterface::gotoTarget() | Partial |
| Sync | :CM# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::sync(), MountInterface::syncTarget() | Partial |
| Tracking start | :Te# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::start_tracking() | Partial |
| Tracking stop | :Td# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::stop_tracking() | Partial |
| Tracking query | :GW# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | getTracking() | Partial |
| Park | :hP# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::park() | Partial |
| Park state | :h?# / :hQ# / :hR# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | getParkStatus() | Partial |
| Home | :hF# / :hC# / :h?# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::home(), getHomeStatus() | Partial |
| Abort motion | :Q# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | MotionService::abort(), safety() | Partial |
| Guiding manual | :Mn# / :Ms# / :Me# / :Mw# / :Qn# / :Qs# / :Qe# / :Qw# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | guide(), manualMotion() | Partial |
| Pulsed guiding | :Mgdn# / :Mgdw# / :Mgde# / :Mgds# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | guide() | Partial |
| Alignment | :A1# / :A# / :AW# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | align() | Partial |
| Mount mode query | :GXM# | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | getMountMode() | Partial |

### Communication-layer verdict

The code can definitely send and receive LX200-compatible commands. The transport and parser stack are real and active. What is missing is not the low-level command syntax, but the robustness of real operation: error recovery, state validation, and end-to-end mount workflow integration.

## 2. High-level service architecture audit

### Current control flow

User action -> application service -> mount abstraction -> LX200 client -> OnStepX

Concrete paths:

- Motion workflow: [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp) -> [software/asdevlab/include/asdevlab/hardware/mount/mount_interface.hpp](software/asdevlab/include/asdevlab/hardware/mount/mount_interface.hpp) -> [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp)
- Catalog workflow: [software/asdevlab/src/services/target_service.cpp](software/asdevlab/src/services/target_service.cpp) -> [software/asdevlab/src/catalog/catalog_engine.cpp](software/asdevlab/src/catalog/catalog_engine.cpp) -> [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp) -> MotionService
- Core orchestration: [software/asdevlab/include/asdevlab/telescope_core.hpp](software/asdevlab/include/asdevlab/telescope_core.hpp) and [software/asdevlab/src/telescope_core.cpp](software/asdevlab/src/telescope_core.cpp)

### Ownership of decisions

- The higher-level application layer is represented by TelescopeCore and service classes.
- MotionService owns the motion-state transitions and calls into the mount adapter.
- StateMachine provides a simple state machine for IDLE, SLEWING, TRACKING, PARKING, etc.
- SafetyService gates some motion commands.
- The hardware adapter owns the actual LX200 protocol translation.

### Separation of responsibilities

Responsibilities are partially separated, but not fully cleanly:

- Good: protocol details are mostly contained in the mount adapter.
- Good: motion workflows are routed through MotionService rather than hardcoded in the transport layer.
- Partial: the adapter still implements a richer API surface than a pure hardware shim, including some workflow-like behavior for mount mode and mount response handling.
- Incomplete: alignment, multi-step telescope workflows, and astronomy calculation are not yet represented by a dedicated higher-level engine.

### Architectural verdict

The architecture is directionally correct for a desktop telescope application. It uses a layered structure, but it is still an early-stage skeleton rather than a complete, battle-tested control architecture.

## 3. Mount motion capability audit

### Basic movement

| Feature | Implemented? | File location | How it works | Missing components |
|---|---|---|---|---|
| Slew to RA/Dec | Yes | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Formats RA/Dec and sends :Sr#/:Sd#/:MS# | No robust goto completion tracking, no error recovery |
| Slew to Alt/Az | Yes | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :Sa#/:Sz# | No real altitude/azimuth workflow or validation |
| Manual directional movement | Yes | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :Mn#/:Ms#/:Me#/:Mw# and stop commands | No joystick-style state handling or acceleration control |
| Stop movement | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Stop commands are available | Not integrated into a robust motion-stop workflow |
| Emergency abort | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :Q# | No hard-stop safety interlocks or full state reconciliation |

### Tracking

| Feature | Implemented? | File location | How it works | Missing components |
|---|---|---|---|---|
| Start tracking | Yes | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :Te# | No verification of actual mount state after command |
| Stop tracking | Yes | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :Td# | No integration with state-machine transitions beyond simple calls |
| Change tracking mode | Partial | [software/asdevlab/src/services/tracking_service.cpp](software/asdevlab/src/services/tracking_service.cpp) | Service methods exist but do not dispatch real mount commands | No real mode command implementation beyond placeholders |
| Solar/lunar/sidereal tracking | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Tracking rate preset helper exists | No real rate-mode workflow or validation |

### Parking

| Feature | Implemented? | File location | How it works | Missing components |
|---|---|---|---|---|
| Park | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :hP# | No full park-state verification or park position management |
| Unpark | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :hR# | No workflow for unpark sequencing |
| Park state monitoring | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Reads :h?# and parses park state | State interpretation is simplistic |

### Home

| Feature | Implemented? | File location | How it works | Missing components |
|---|---|---|---|---|
| Home command | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Sends :hF# | No homing-state tracking or recovery strategy |
| Homing state | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Parses home status responses | No complete home workflow |
| Limit handling | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Supports limits commands and parsing | No integrated limit policy |

## 4. Coordinate and astronomy workflow audit

### Current coordinate handling

The host side currently handles coordinates in three ways:

1. Direct RA/Dec input from the motion layer.
2. Catalog object RA/Dec from JSON catalog records.
3. A static coordinate resolver used by the catalog pipeline.

### What the code actually does

- [software/asdevlab/src/services/target_service.cpp](software/asdevlab/src/services/target_service.cpp) resolves a catalog object into RA/Dec and sends it to MotionService.
- [software/asdevlab/src/catalog/catalog_engine.cpp](software/asdevlab/src/catalog/catalog_engine.cpp) loads catalog JSON files and indexes object records.
- [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp) resolves coordinates from providers or a static provider.
- [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp) contains hardcoded sample targets such as M31, M42, Jupiter, Saturn, and NGC7000.

### Astronomy engine status

- ASDEVLAB does not appear to contain a full astronomy engine for topocentric coordinate conversion, apparent place calculations, precession/nutation handling, refraction correction, or ephemeris lookup.
- The code does not delegate full astronomical calculations to the OnStepX firmware; instead, it relies on the caller to provide RA/Dec coordinates and the mount to execute them.
- The host side is therefore closer to a controller/orchestrator than a true astronomy computation engine.

### Coordinate providers and catalog usage

- Current coordinate providers are lightweight and not a full astronomical library.
- Catalog coordinates are loaded from JSON and used as input to motion commands.
- Missing components include: ephemeris support, solar-system body calculators, sidereal-time generation, local apparent coordinates, and full time/location-aware conversion.

## 5. Alignment workflow audit

### Current alignment support

The implementation has a thin alignment API, but not a real user workflow.

- [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) has an align() method that sends simple alignment commands such as :A1#, :A#, and :AW#.
- [software/asdevlab/src/services/alignment_service.cpp](software/asdevlab/src/services/alignment_service.cpp) exposes a placeholder sync_from_solution() method.

### What is missing for a real alignment workflow

The following are not implemented as a real workflow:

1. One-star, two-star, and three-star alignment flow
2. User-guided star selection
3. Centering and sync step orchestration
4. Alignment model storage/persistence
5. Post-alignment GOTO accuracy improvement
6. Error handling for poor alignment or failed sync

### Alignment verdict

ASDEVLAB cannot currently guide a user through a proper mount alignment procedure. It can send low-level alignment commands, but it does not yet implement the full alignment model and assistive workflow expected for accurate GOTO.

## 6. Catalog integration audit

### Current catalog system

- [software/asdevlab/src/catalog/catalog_engine.cpp](software/asdevlab/src/catalog/catalog_engine.cpp) loads JSON catalogs from the data catalog directory and indexes objects.
- [software/asdevlab/src/services/target_service.cpp](software/asdevlab/src/services/target_service.cpp) lets the application search objects and resolve them to motion commands.
- [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp) resolves catalog coordinates.

### Current user journey

User search -> select object -> display info -> calculate/resolve coordinates -> send GOTO

The current implementation supports the first half of this flow in a basic way:

- Search object: implemented by CatalogEngine and TargetService.
- Select object: implemented by TargetService accessors.
- Display information: available via catalog object data.
- Resolve coordinates: implemented at a basic level via the coordinate resolver.
- Send GOTO: implemented via MotionService and the mount adapter.

### Missing catalog steps

- No rich object metadata UI layer is visible in the current motion path.
- No robust coordinate-precession or time-aware resolution pipeline.
- No object type-specific handling for solar-system bodies beyond simple examples.
- No full catalog-driven observation workflow with context and planning.

## 7. Current feature matrix

| Feature | Status | Evidence | Missing |
|---|---|---|---|
| Communication | Partial | [software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp](software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp) | Real-world reliability and recovery |
| Connection management | Partial | [software/asdevlab/src/hardware/mount/lx200_socket_transport.cpp](software/asdevlab/src/hardware/mount/lx200_socket_transport.cpp) | DNS/hostname handling, reconnect policy quality |
| LX200 commands | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Full command coverage and robust response handling |
| GOTO | Partial | [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp) | Completion status, retries, safety handling |
| Sync | Partial | [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp) | Workflow integration and verification |
| Tracking | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Mode switching and real verification |
| Parking | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Full park-state workflow |
| Home | Partial | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | Full homing workflow |
| Alignment | Partial | [software/asdevlab/src/services/alignment_service.cpp](software/asdevlab/src/services/alignment_service.cpp) | Multi-star alignment and storage |
| Coordinate conversion | Partial | [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp) | Full astronomy engine |
| Catalog | Partial | [software/asdevlab/src/catalog/catalog_engine.cpp](software/asdevlab/src/catalog/catalog_engine.cpp) | Rich object workflows |
| Solar system objects | Partial | [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp) | Real ephemeris integration |
| Deep sky objects | Partial | [software/asdevlab/src/services/target_service.cpp](software/asdevlab/src/services/target_service.cpp) | Robust coordinate context handling |
| Satellite tracking | Missing | No dedicated implementation found | Needs dedicated orbital/ephemeris pipeline |
| Error handling | Partial | [software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp](software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp) | Recovery policies, operator feedback |
| Recovery | Partial | [software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp](software/asdevlab/src/hardware/mount/lx200_connection_manager.cpp) | Automatic retry and state reconciliation |
| Logging | Partial | [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp) | Structured diagnostics and event history |

## 8. Realistic current capability

### If I connect ASDEVLAB to a real OnStepX mount today, what can I actually do?

#### A. Fully working now

Very little should be considered fully working for real-world operation. The codebase does provide a real command dispatch path and parser logic, but it is not yet mature enough to be trusted as a complete mount-control product.

#### B. Partially working

The following are partially working today:

- Establish a TCP connection and send initialization commands.
- Query basic mount state and coordinate values.
- Send RA/Dec and Alt/Az goto commands.
- Send sync commands.
- Send tracking start/stop commands.
- Send park, home, abort, and manual guide commands.
- Search and resolve basic catalog objects into goto targets.

#### C. Architecture exists but incomplete

The following exist as architecture but are incomplete for a production-like experience:

- Service-oriented motion control.
- Typed mount abstraction.
- Catalog and target selection pipeline.
- Safety and state-machine scaffolding.
- Mount mode support and backend query path.

#### D. Not implemented

The following are not implemented as complete user-facing capabilities:

- True multi-star alignment workflow
- Alignment model storage and persistence
- Robust automated recovery after failed goto or disconnect
- High-accuracy astronomy engine and ephemeris calculations
- Seestar-like autonomous observation workflow
- Satellite tracking
- Advanced plate-solve-driven pointing correction

## 9. Next development priority

### P0: required before real telescope operation

- End-to-end real-hardware validation for connection, goto, sync, track, park, and abort
- Robust error handling and recovery from disconnects and timeout failures
- Real status verification after every motion command
- Basic safety interlocks and operator-visible error states

### P1: required for reliable GOTO

- Alignment workflow and model building
- Better coordinate context and local site/time handling
- Mount-state reconciliation after motion completes
- Retry and fallback strategies for failed commands

### P2: required for Seestar-like experience

- Catalog-driven observation planning
- Plate-solve-assisted pointing correction
- Autoguiding and acquisition workflow
- Simple autonomous target execution

### P3: advanced features

- Full ephemeris support for solar-system objects
- Satellite tracking
- Advanced scheduling and multi-target observation planning
- Rich UI and remote operation features

## Bottom line

The current ASDEVLAB codebase is best described as a real but incomplete telescope-control skeleton. It has a real LX200 command path and a reasonable layered architecture, but it is not yet a complete, reliable, real-world OnStepX control application. The biggest gaps are in robustness, alignment, astronomy workflow, and end-to-end operation validation.
