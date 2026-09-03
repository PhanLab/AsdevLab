# ASDEVLAB Backend Audit vs. OnStepX LX200 Mount Protocol

## Scope and constraints

This document audits the current ASDEVLAB backend against the OnStepX mount command surface only. It does not redesign the project, does not introduce a plugin architecture, and does not change public APIs.

Project boundaries used for this audit:

- ASDEVLAB is a standalone telescope-control application.
- OnStepX is only the mount firmware.
- ASDEVLAB talks to OnStepX over LX200-compatible commands.
- Catalogs are local and not part of the mount protocol audit.
- GPS/time/location come from the user PC.
- No OnStep library support.
- No weather support.
- No rotator support.
- Software safety is implemented entirely by ASDEVLAB.
- FlipMirror servo is controlled by ASDEVLAB.

## Current implementation baseline

### Implementation status update

The backend now routes mount functionality through the existing typed Mount API and keeps LX200 command formatting/response parsing inside the mount layer. The following areas are now covered in the adapter implementation:

- Structured mount status and packed-status parsing with firmware fallback
- Site, time, tracking, guide, park, home, PEC, limits, focuser, and flip-mirror readback
- Alignment, tracking-rate, guide-rate, slew-rate, manual-motion, and auxiliary control entry points
- Capability detection and graceful fallback responses for unsupported firmware commands


### Existing backend entry points

- Mount transport and command layer: [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp)
- Mount parser layer: [software/asdevlab/src/hardware/mount/lx200_parser.cpp](../software/asdevlab/src/hardware/mount/lx200_parser.cpp)
- Mount interface: [software/asdevlab/include/asdevlab/hardware/mount/mount_interface.hpp](../software/asdevlab/include/asdevlab/hardware/mount/mount_interface.hpp)
- Motion service: [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp)
- Guide service: [software/asdevlab/src/services/guide_service.cpp](../software/asdevlab/src/services/guide_service.cpp)
- Focus service: [software/asdevlab/src/services/focus_service.cpp](../software/asdevlab/src/services/focus_service.cpp)
- Safety service: [software/asdevlab/src/safety_service.cpp](../software/asdevlab/src/safety_service.cpp)
- Tests: [software/asdevlab/test/test_onstep_adapter.cpp](../software/asdevlab/test/test_onstep_adapter.cpp), [software/asdevlab/test/test_motion_service.cpp](../software/asdevlab/test/test_motion_service.cpp), [software/asdevlab/test/test_core_services.cpp](../software/asdevlab/test/test_core_services.cpp)

### Current mount protocol coverage

Implemented today:

- Status string readback via `:GU#`
- RA/Dec target set via `:Sr...#` and `:Sd...#`
- Goto via `:MS#`
- Sync via `:CM#`
- Tracking on/off via `:Te#` and `:Td#`
- Home reset via `:hF#`
- Park via `:hP#`
- Abort via `:Q#`

Not implemented today:

- Bit-packed status parsing
- Site/time/location handling
- Alignment workflow
- Guide pulse/continuous guide
- Home status and configuration
- Park position management
- PEC control
- Limits control/readback
- Auxiliary buzzer/busy/dist bar
- Focuser and flip mirror control

## Capability audit

### 1. Status

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|
| Status string | Basic status string readback exists through the mount client and is used by the motion service and tests. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/hardware/mount/lx200_parser.cpp](../software/asdevlab/src/hardware/mount/lx200_parser.cpp) | The backend does not expose structured status flags or mount state beyond a raw response string. | `:GU#` | Yes | Partial; existing tests only verify the raw response path. |
| Bit-packed status | No implementation. | None | No service or parser exists for the packed-status byte stream. | `:Gu#` | Yes | No |
| Tracking/basic state | No implementation. | None | No parser or service abstraction for tracking/basic state. | `:GW#` | Yes | No |
| Pier side | No implementation. | None | No parser or service abstraction for pier-side reporting. | `:Gm#` | Yes | No |

### 2. Site

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|
| Site select | No implementation. | None | No site-selection service or mount-client path. | `:W[n]#`, `:W?#` | Yes | No |
| Site name | No implementation. | None | No site-name get/set support. | `:SM#`, `:SN#`, `:SO#`, `:SP#`, `:GM#`, `:GN#`, `:GO#`, `:GP#` | Yes | No |
| Longitude | No implementation. | None | No longitude read/write support. | `:Sg#`, `:Gg#` | Yes | No |
| Latitude | No implementation. | None | No latitude read/write support. | `:St#`, `:Gt#` | Yes | No |
| Elevation | No implementation. | None | No elevation read/write support. | `:Sv#`, `:Gv#` | Yes | No |

### 3. Time

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|
| Local time | No implementation. | None | No local-time setter/getter path. | `:SL#`, `:GL#` | Yes | No |
| Local date | No implementation. | None | No local-date setter/getter path. | `:SC#`, `:GC#` | Yes | No |
| Sidereal time | No implementation. | None | No sidereal-time readback path. | `:GS#`, `:GSH#` | Yes | No |
| UTC/DUT1 | No implementation. | None | No UTC/DUT1 support. | `:GX80#`, `:GX81#`, `:SU#` | Yes | No |

### 4. Alignment

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Manual alignment | No implementation. | None | No alignment service or mount-client path. | `:A[n]#` | Yes | No |
| Accept star | No implementation. | None | No accept-star workflow. | `:A+#` | Yes | No |
| Save alignment | No implementation. | None | No alignment persistence path. | `:AW#` | Yes | No |

### 5. GOTO

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Target RA/DEC | Implemented through the mount client and motion service for basic goto/sync. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No rich target-state handling and no explicit target readback. | `:Sr...#`, `:Sd...#` | Partial | Existing tests cover emission only. |
| Target ALT/AZ | No implementation. | None | No alt/az target setter path. | `:Sa...#`, `:Sz...#` | Yes | No |
| Read target RA/DEC | No implementation. | None | No target-readback support. | `:Gr#`, `:Gd#` | Yes | No |
| Read target ALT/AZ | No implementation. | None | No target-alt/az readback support. | `:Ga#`, `:Gz#` | Yes | No |
| GOTO | Implemented for basic RA/Dec goto. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No structured error handling beyond simple boolean success. | `:MS#` | Partial | Existing tests cover basic success/failure path. |
| ALT/AZ GOTO | No implementation. | None | No alt/az goto flow. | `:MA#` | Yes | No |
| Polar align GOTO | No implementation. | None | No polar-align goto flow. | `:MP#` | Yes | No |
| Pier-side variants | No implementation. | None | No pier-side specific goto flows. | `:MN...#`, `:MD#` | Yes | No |

### 6. Sync

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Sync current | Implemented for RA/Dec sync. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No richer response handling or structured error mapping. | `:CS#` | Partial | Existing tests cover emission only. |
| Sync by object | Implemented for object-based sync. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No richer response mapping for error codes. | `:CM#` | Partial | Existing tests cover emission only. |

### 7. Coordinates

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| RA/DEC | Implemented for readback of RA/Dec through basic mount-client methods. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/hardware/mount/lx200_parser.cpp](../software/asdevlab/src/hardware/mount/lx200_parser.cpp) | The parser is simple and currently does not expose richer structured mount state. | `:GR#`, `:GD#` | Partial | Existing tests cover basic adapter behavior only. |
| ALT/AZ | No implementation. | None | No alt/az readback path. | `:GA#`, `:GZ#` | Yes | No |
| Axis | No implementation. | None | No axis-angle or axis-encoder abstraction. | `:GX4[n]#` | Yes | No |
| Encoder | No implementation. | None | No encoder readback path. | `:GX4[n]#` | Yes | No |
| Mechanical angle | No implementation. | None | No mechanical-angle readback path. | `:GX4[n]#` | Yes | No |

### 8. Tracking

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Enable | Implemented. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No explicit state feedback after enable. | `:Te#` | No | Existing tests cover send path. |
| Disable | Implemented. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No explicit state feedback after disable. | `:Td#` | No | Existing tests cover send path. |
| Status | No implementation. | None | No readback of the current tracking state. | `:GT#`, `:GW#` | Yes | No |

### 9. Tracking rate

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Presets | No implementation. | None | No tracking-rate preset service. | `:TQ#`, `:TS#`, `:TK#`, `:TL#` | Yes | No |
| Custom | No implementation. | None | No custom-rate control. | `:ST[H.H]#` | Yes | No |

### 10. Guide

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Pulse guide | No implementation. | [software/asdevlab/src/services/guide_service.cpp](../software/asdevlab/src/services/guide_service.cpp) | The service exists but it does not dispatch real LX200 guide commands. | `:Mgd[n]#`, `:MGd[n]#` | Yes | No |
| Continuous guide | No implementation. | [software/asdevlab/src/services/guide_service.cpp](../software/asdevlab/src/services/guide_service.cpp) | No continuous guide motion path. | `:Mw#`, `:Me#`, `:Mn#`, `:Ms#` | Yes | No |
| Guide rate | No implementation. | None | No guide rate selection path. | `:RG#`, `:RC#`, `:RM#`, `:RF#`, `:RS#`, `:Rn#` | Yes | No |
| Read guide rate | No implementation. | None | No guide rate readback. | `:GX90#` | Yes | No |

### 11. Manual motion

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Stop | The motion service exposes stop semantics, but only as a high-level wrapper. The actual LX200 stop path is not implemented by the mount client. | [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No real stop dispatch to axis-specific stop commands. | `:Q#`, `:Qe#`, `:Qw#`, `:Qn#`, `:Qs#` | No | Existing tests do not cover the low-level stop path. |

### 12. Slew

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Presets | No implementation. | None | No slew-rate preset support. | `:SX9,3#` | Yes | No |
| Custom | No implementation. | None | No custom slew-rate control. | `:SX9,2#` | Yes | No |

### 13. Home

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Status | No implementation. | None | No home-state/status reporting. | `:h?#` | Yes | No |
| Go Home | No implementation. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | The mount client only sends `:hF#` reset-home; it does not implement `:hC#`. | `:hC#` | Yes | No |
| Reset | Implemented for the reset-home command. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp) | No richer home-state handling or configuration. | `:hF#` | Partial | Existing tests cover the command send path. |
| Auto Home | No implementation. | None | No auto-home configuration support. | `:hA#`, `:hC1,n#`, `:hC2,n#` | Yes | No |

### 14. Park

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Park | Implemented. | [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | No park-state feedback beyond a simple service result. | `:hP#` | Partial | Existing tests cover send path. |
| Set Park | No implementation. | None | No park-position set command. | `:hQ#` | Yes | No |
| Restore Park | No implementation. | None | No unpark/restore path. | `:hR#` | Yes | No |

### 15. PEC

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Read | No implementation. | None | No PEC readout service. | `:VS#`, `:VW#`, `:VH#`, `:VR[n]#` | Yes | No |
| Record | No implementation. | None | No PEC recording workflow. | `:WR...#` | Yes | No |
| Enable | No implementation. | None | No PEC-enable path. | `$QZ+#` | Yes | No |
| Disable | No implementation. | None | No PEC-disable path. | `$QZ-#` | Yes | No |
| Status | No implementation. | None | No PEC-state reporting. | `$QZ?#` | Yes | No |

### 16. Limits

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Horizon | No implementation. | None | No horizon-limit control. | `:Sh#` | Yes | No |
| Overhead | No implementation. | None | No overhead-limit control. | `:So#` | Yes | No |
| Meridian | No implementation. | None | No meridian-limit control. | `:SXE9#`, `:SXEA#` | Yes | No |
| Readback | No implementation. | None | No limit readback service. | `:Gh#`, `:Go#`, `:GXE...#` | Yes | No |

### 17. Auxiliary

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Buzzer | No implementation. | None | No buzzer control path. | `:SX97,[n]#` | Yes | No |
| Busy state | No implementation. | None | No busy-state or distance-bar readback. | `:D#` | Yes | No |
| Distance bar | No implementation. | None | No distance-bar readback path. | `:D#` | Yes | No |

### 18. Devices

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Focuser | No implementation. | None | No focus-control service over LX200. | Device-specific focus commands are outside the current mount-client scope. | Yes | No |
| FlipMirror servo | No implementation. | None | No flip-mirror servo service. | No current LX200 command mapping exists in the backend. | Yes | No |

### 19. Software

| Feature | Current implementation | Current files | Missing parts | Required LX200 commands | Missing parser | Missing tests |
|---|---|---|---|---|---|---|---|
| Safety policy | Implemented in software only, not as an LX200 feature. | [software/asdevlab/src/safety_service.cpp](../software/asdevlab/src/safety_service.cpp), [software/asdevlab/src/services/motion_service.cpp](../software/asdevlab/src/services/motion_service.cpp) | The safety policy is basic and should be expanded to cover more mount states and command families. | No direct LX200 command; software-only guard | No | Existing smoke tests cover only basic policy cases. |

## Migration plan

### Phase 1 — Foundation and status

Priority: highest

1. Add a structured mount-status model to the parser layer.
2. Implement `:GU#`, `:Gu#`, `:GW#`, and `:Gm#` support.
3. Expose status through the existing mount interface and motion service.
4. Add parser and adapter tests for each response family.

### Phase 2 — Site, time, and alignment

Priority: high

1. Implement site selection and site-name/location read/write.
2. Implement local-time/date/sidereal-time/UT1 support.
3. Add alignment entry points for manual alignment, accept-star, and save-alignment.
4. Keep these features behind the existing backend services rather than introducing a new architecture.

### Phase 3 — Goto, sync, coordinates, tracking, and guide

Priority: high

1. Extend the mount client to support target RA/Dec, Alt/Az, readback, and goto variants.
2. Add structured response parsing for `:MS#` and `:CM#`.
3. Implement tracking-state readback and tracking-rate control.
4. Add pulse guide and continuous guide support through the guide service.
5. Add guide-rate readback and selection support.

### Phase 4 — Home, park, limits, and PEC

Priority: medium

1. Implement home status, `:hC#`, auto-home settings, and park restore.
2. Add limits control/readback.
3. Add PEC read/record/enable/disable/status support.
4. Add parser coverage for the new command families.

### Phase 5 — Auxiliary and device features

Priority: medium / optional

1. Implement buzzer and busy/dist-bar support.
2. Add focuser and flip-mirror servo support only if the backend needs them for practical observing workflows.
3. Keep these as thin service wrappers over LX200 transport rather than new subsystems.

## Out-of-scope items under the current project philosophy

The following should remain outside the migration plan:

- OnStep library support
- Weather support
- Rotator support
- Full OnStepX web UI behavior
- Any plugin-style abstraction layer

## Estimated completion

For the in-scope mount-control features in this audit, the current backend is roughly at the early stages of implementation. A practical estimate is:

- Basic mount control core: around 25% complete
- Full LX200-compatible mount capability surface: well below 50% complete

The highest-value first step is not a redesign; it is to close the gap around status parsing, site/time support, guide support, and richer goto/sync error handling while preserving the existing service and transport boundaries.
