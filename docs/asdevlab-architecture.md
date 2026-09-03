# ASDEVLAB Architecture Proposal

This document captures the new service-based architecture for the ASDEVLAB project.

## Core idea

The user-facing experience should talk to one Telescope Core layer, not directly to OnStepX or OpenLiveStacker.

## Service responsibilities

- MotionService: handle goto, move, park, stop, speed, limits.
- TrackingService: manage sidereal/solar/lunar/custom tracking.
- GuideService: translate guiding corrections into commands.
- AlignmentService: apply plate-solver based corrections.
- CameraService: manage camera lifecycle and frame delivery.
- ImageService: process frames for stacking and display.
- PlateSolverService: solve images and produce alignment data.
- SchedulerService: coordinate multi-step observation workflows.

## Data flow

1. The UI calls Telescope Core.
2. Telescope Core dispatches to the appropriate service.
3. Services use adapters for OnStepX, camera hardware, or OpenLiveStackter processing.
4. Results are returned as structured events or simple status messages.

## Mount mode workflow

The mount adapter keeps two distinct values for mount mode:

- desiredMode: the user-selected mode that should be active after firmware changes.
- detectedMode: the last successfully observed mount mode from the firmware via :GXM#.

The workflow is intentionally application-side only:

1. Selecting a mode stores desiredMode and does not send any LX200 command.
2. The user is told that the firmware must be rebuilt and uploaded.
3. After firmware upload is confirmed, the adapter reconnects and calls updateDetectedMountMode().
4. The adapter compares desiredMode with detectedMode and reports success when they match, or firmware mismatch when they do not.
