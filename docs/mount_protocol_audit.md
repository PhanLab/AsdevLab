# OnStepX Mount Protocol Audit for ASDEVLAB

## Scope

This report captures the current state of the ASDEVLAB mount protocol boundary against the OnStepX LX200-compatible command surface implemented under [firmware/OnStepX/src/telescope/mount](../firmware/OnStepX/src/telescope/mount).

The intent is to document the current wrapper coverage, parser coverage, test coverage, and the remaining gaps without changing upper services or adding new application behavior.

## Evidence base

### Firmware handlers reviewed

- [firmware/OnStepX/src/telescope/mount/status/Status.command.cpp](../firmware/OnStepX/src/telescope/mount/status/Status.command.cpp)
- [firmware/OnStepX/src/telescope/mount/Mount.command.cpp](../firmware/OnStepX/src/telescope/mount/Mount.command.cpp)
- [firmware/OnStepX/src/telescope/mount/goto/Goto.command.cpp](../firmware/OnStepX/src/telescope/mount/goto/Goto.command.cpp)
- [firmware/OnStepX/src/telescope/mount/guide/Guide.command.cpp](../firmware/OnStepX/src/telescope/mount/guide/Guide.command.cpp)
- [firmware/OnStepX/src/telescope/mount/home/Home.command.cpp](../firmware/OnStepX/src/telescope/mount/home/Home.command.cpp)
- [firmware/OnStepX/src/telescope/mount/park/Park.command.cpp](../firmware/OnStepX/src/telescope/mount/park/Park.command.cpp)
- [firmware/OnStepX/src/telescope/mount/pec/Pec.command.cpp](../firmware/OnStepX/src/telescope/mount/pec/Pec.command.cpp)
- [firmware/OnStepX/src/telescope/mount/limits/Limits.command.cpp](../firmware/OnStepX/src/telescope/mount/limits/Limits.command.cpp)
- [firmware/OnStepX/src/telescope/mount/site/Site.command.cpp](../firmware/OnStepX/src/telescope/mount/site/Site.command.cpp)

### ASDEVLAB implementation reviewed

- [software/asdevlab/include/asdevlab/hardware/mount/mount_interface.hpp](../software/asdevlab/include/asdevlab/hardware/mount/mount_interface.hpp)
- [software/asdevlab/include/asdevlab/hardware/mount/lx200_parser.hpp](../software/asdevlab/include/asdevlab/hardware/mount/lx200_parser.hpp)
- [software/asdevlab/src/hardware/mount/onstep_mount_client.cpp](../software/asdevlab/src/hardware/mount/onstep_mount_client.cpp)
- [software/asdevlab/src/hardware/mount/lx200_parser.cpp](../software/asdevlab/src/hardware/mount/lx200_parser.cpp)
- [software/asdevlab/test/test_onstep_adapter.cpp](../software/asdevlab/test/test_onstep_adapter.cpp)
- [software/asdevlab/test/test_lx200_parser.cpp](../software/asdevlab/test/test_lx200_parser.cpp)

## Current implementation summary

The mount boundary now exposes a typed adapter API for the main LX200/OnStep command families and keeps the implementation inside the hardware layer. The current adapter is no longer limited to a minimal status/position path; it now includes typed wrappers for site, time, tracking rates, guide rates, slew rates, home, park, PEC, limits, focuser, flip mirror, manual motion, alignment, mount mode, orientation, environment, rotator, and auxiliary commands. Optional capability families that are not implemented by a given backend now default to unsupported responses instead of forcing every mock or stub to implement every firmware-specific branch.

## Coverage matrix

| Capability family | Firmware commands reviewed | Wrapper status | Parser status | Test status | Notes |
| --- | --- | --- | --- | --- | --- |
| Status and state | :GU#, :Gu#, :GW# | Implemented | Implemented | Covered | Packed status parsing and raw status parsing are both exercised. |
| Coordinates | :GR#, :GD#, :Sr..., :Sd..., :MS#, :CM# | Implemented | Implemented | Covered | RA/Dec readback and goto/sync are wired. |
| Goto and sync | :MS#, :CM#, :MA#, :MN#, :MP# | Partially implemented | Implemented | Covered | Basic goto/sync wrappers exist; alt/az and richer error mapping are still shallow. |
| Tracking | :Te#, :Td#, :TQ#, :TS#, :TL# | Implemented | Implemented | Covered | Basic tracking on/off is covered; rate preset handling is present but minimal. |
| Guide | :Mw#, :Me#, :Mn#, :Ms#, :Mgd..., :Q... | Implemented | Implemented | Covered | Pulse guide and continuous guide motion are wrapped. |
| Manual motion | :Mn#, :Ms#, :Me#, :Mw#, :Qn#, :Qs#, :Qe#, :Qw# | Implemented | Implemented | Covered | Manual motion uses the same command surface as guide and is exposed through a distinct Motion capability. |
| Home | :h?#, :hC#, :hF# | Implemented | Implemented | Covered | Home status and move-to-home are present. |
| Park | :hP#, :hQ#, :hR#, :h?# | Implemented | Implemented | Covered | Park set/unpark/status are available. |
| Site | :SL#, :SG#, :SS#, :W?# | Implemented | Implemented | Covered | Latitude/longitude/elevation and site selection are surfaced. |
| Time | :GL#, :GC#, :GS#, :GX80#, :GX81# | Implemented | Implemented | Covered | Local time/date/sidereal/UTC values are parsed. |
| Alignment | :A..., :AW# | Implemented | Not structured | Partial | Manual alignment and accept/save entry points are wrapped, but status readback is not yet modeled. |
| PEC | :$QZ?#, :VS#, :VW#, :V..., :WR... | Partial | Partial | Partial | Status readback is exposed; write and recording flow remain thin. |
| Limits | :Gh#, :Go#, :GXE9#, :SXEA#, :SXE9# | Implemented | Implemented | Covered | Basic horizon/overhead/meridian values are mapped. |
| Focuser | :F?# | Implemented | Implemented | Covered | Typed focuser state, movement, stop, and goto-style control are surfaced. |
| Flip mirror | :FM?# | Implemented (readback only) | Implemented | Covered | The adapter exposes abstract camera/eyepiece/unknown status; movement is unsupported by the current firmware command surface. |
| Tracking rate | :TQ#, :TS#, :TL#, :ST... | Implemented | Implemented | Covered | Preset-based wrapper and readback are available. |
| Guide rate | :RG#, :RC#, :RM#, :RF#, :RS#, :GX90# | Implemented | Implemented | Covered | Preset-based rate and readback are supported. |
| Slew rate | :SX9,..., :GX93# | Implemented | Implemented | Covered | Preset-based slew rate and readback are supported. |
| Auxiliary | :D#, :SX97,... | Implemented | Partial | Covered | Basic buzzer/aux control is exposed. |
| Mount mode | :GXE M#, :SX... | Implemented | Implemented | Covered | Query/set support is exposed as Eq/AltAz abstractions. |
| Orientation / environment / rotator | :Gg#, :Gd#, :GX... | Implemented | Implemented | Covered | These are exposed as optional typed readback paths rather than application-level features. |
| Safety / abort | :Q# | Implemented | N/A | Covered | Abort path is available through the safety API. |

## What is currently covered well

The following areas are now in good shape at the mount boundary:

- status reading and structured status parsing
- RA/Dec coordinate readback and goto/sync dispatch
- guide motion and manual motion control
- home/park/status flows
- site/time readback
- tracking/guide/slew rate set/readback
- focuser/flip-mirror state readback

## What remains incomplete or shallow

The following areas are still missing or only partially surfaced:

1. Alt/Az target handling
   - The firmware exposes target altitude/azimuth commands and related getters, but the typed adapter currently does not expose them as a first-class path.

2. Richer goto/sync error mapping
   - The adapter sends the goto/sync commands, but the response mapping is still thin compared to the full OnStep error-code surface.

3. Full alignment workflow
   - The adapter exposes the basic alignment entry points, but it does not yet expose alignment status, star-progress, or post-alignment state in a structured way.

4. Flip mirror movement
   - The current adapter implements flip mirror status readback only. The firmware does not expose a documented movement command, so move semantics are intentionally unsupported.

5. PEC write and recording flow
   - The current wrapper is mostly a status-read wrapper; the recording/playback controls remain outside the typed API.

6. Extended configuration space
   - The firmware contains a wide :GX... / :SX... command surface for advanced configuration and diagnostics. Only the most relevant subsets have been surfaced so far.

## Intentionally unsupported scope

The mount boundary is intentionally narrow and protocol-oriented. The following capabilities are not promoted into the ASDEVLAB-facing layer because they are either firmware-specific, diagnostic-only, or outside the current application needs:

- raw servo/PWM calibration details
- firmware-internal mount configuration tuning
- advanced PEC recording/playback control beyond basic status exposure
- full alignment progress state and diagnostic telemetry
- extended command families that do not map cleanly onto the current ASDEVLAB abstractions

These remain outside the boundary so that higher-level services can continue to depend on the same stable typed API without becoming coupled to OnStepX internals.

## Wrapper status summary

### Implemented through the typed mount API

- Status API
- Coordinates API
- Tracking API
- Guide API
- Park API
- Home API
- Site API
- Time API
- Limits API
- Focuser API
- Flip mirror API
- Tracking rate API
- Guide rate API
- Slew rate API
- Auxiliary API
- Safety API

### Partial coverage

- Goto/sync target handling
- Alignment workflow
- PEC control
- Advanced extended command surface

## Parser status summary

The parser layer now includes typed support for:

- packed and text-based mount status
- RA/Dec and coordinate pairs
- goto state values
- tracking, guide, park, home, and site/time values
- guide rate and slew rate values
- PEC, limits, focuser, and flip-mirror state

The remaining parser gaps are mainly around richer alignment and advanced extended-response formats.

## Test status summary

The current test coverage includes:

- parser tests for status, coordinates, goto state, tracking, guide, park, home, site, time, rates, PEC, limits, and focuser/flip-mirror parsing
- adapter tests for command emission and typed response handling through a mock transport

## Estimated completion

Using the current capability-module view, roughly 70% of the mount protocol surface is now wired through the typed adapter layer with at least basic wrapper and parser support. The remaining gap is concentrated in advanced command families and richer error/diagnostic handling rather than in the core motion and status flow.

## Verification status

The current implementation was verified by running:

```bash
cmake --build build -j4 --target asdevlab_onstep_adapter_test asdevlab_lx200_parser_test && ctest --test-dir build --output-on-failure -R 'onstep_adapter|lx200_parser'
```

Result: both adapter and parser test targets passed.

## Generated artifacts

- `docs/mount_protocol_inventory.md`
- `docs/mount_protocol_coverage.md`
- `docs/mount_protocol_missing.md`
- `docs/mount_protocol_statistics.md`
- `docs/firmware_lx200_inventory.csv`
