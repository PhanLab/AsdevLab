# ASDEVLAB catalog system audit

## Scope

This report audits the current ASDEVLAB catalog implementation as it exists in the workspace. The audit is based on the actual code paths and the current catalog JSON data files, not on design docs or comments alone.

## 1. Catalog architecture audit

### Main catalog-related classes

- CatalogEngine
  - Defined in [software/asdevlab/include/asdevlab/catalog/catalog_engine.hpp](software/asdevlab/include/asdevlab/catalog/catalog_engine.hpp)
  - Implemented in [software/asdevlab/src/catalog/catalog_engine.cpp](software/asdevlab/src/catalog/catalog_engine.cpp)
  - Responsibility: load catalog data from JSON, index objects, support search by id/name/type/alias, and expose objects to the rest of the app.
  - Current behavior: loads one or more JSON files from a path, parses them into CatalogObject entries, and builds in-memory indexes.

- CatalogObject
  - Defined in [software/asdevlab/include/asdevlab/catalog/catalog_object.hpp](software/asdevlab/include/asdevlab/catalog/catalog_object.hpp)
  - Responsibility: the canonical in-memory catalog record model.
  - Current fields: id, name, type, magnitude, ra, dec, epoch, coordinate_source, messier, ngc, ic, alias, constellation, fun_fact.

- CoordinateResolver
  - Defined in [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp)
  - Responsibility: resolve a CatalogObject into an EquatorialCoordinate for the current observation context.
  - Current behavior: tries registered providers first, then falls back to the static provider.

- StaticCoordinateProvider
  - Defined in [software/asdevlab/include/asdevlab/catalog/static_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/static_coordinate_provider.hpp)
  - Responsibility: return the stored RA/DEC from the catalog object when the object uses static catalog coordinates.

- EphemerisCoordinateProvider
  - Declared in [software/asdevlab/include/asdevlab/catalog/ephemeris_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/ephemeris_coordinate_provider.hpp)
  - Responsibility: intended for dynamic solar-system or orbital object resolution.
  - Current implementation: the class exists but its supports() and resolve() methods return false, so it is not active.

- TargetService
  - Declared in [software/asdevlab/include/asdevlab/services/target_service.hpp](software/asdevlab/include/asdevlab/services/target_service.hpp)
  - Implemented in [software/asdevlab/src/services/target_service.cpp](software/asdevlab/src/services/target_service.cpp)
  - Responsibility: connect catalog selection to motion commands.
  - Current behavior: search the catalog, fetch an object by id, resolve coordinates, and send goto/sync/preview requests to MotionService.

- MotionService
  - Declared in [software/asdevlab/include/asdevlab/services/motion_service.hpp](software/asdevlab/include/asdevlab/services/motion_service.hpp)
  - Implemented in [software/asdevlab/src/services/motion_service.cpp](software/asdevlab/src/services/motion_service.cpp)
  - Responsibility: turn resolved coordinates into mount operations.
  - Current behavior: calls the mount adapter’s gotoTarget/syncTarget methods and then starts tracking.

### Dependencies between components

- TelescopeCore wires the pieces together in [software/asdevlab/src/telescope_core.cpp](software/asdevlab/src/telescope_core.cpp).
- The real dependency chain is:

  1. CatalogEngine loads JSON files.
  2. TargetService asks CatalogEngine for objects and search results.
  3. CoordinateResolver resolves a CatalogObject into RA/DEC.
  4. TargetService passes the resolved coordinates into MotionService.
  5. MotionService passes them to the mount adapter.
  6. The mount adapter sends actual LX200-style goto commands to OnStepX.

### Current architecture diagram

```text
JSON catalog files
    ↓
CatalogEngine
    ↓
CatalogObject model
    ↓
TargetService search/select
    ↓
CoordinateResolver
    ↓
EquatorialCoordinate
    ↓
MotionService
    ↓
MountInterface / OnStepMountClient
    ↓
LX200 goto command
```

### Real execution flow

```text
JSON file

↓

Catalog loader

↓

Catalog model

↓

Search/select

↓

Target resolver

↓

MotionService

↓

Goto command
```

This flow is implemented in the code, but it is deliberately lightweight and does not yet include a full astronomy layer.

## 2. Current catalog data audit

The current catalog files are located in [software/asdevlab/data/catalog](software/asdevlab/data/catalog).

### File inventory

- [software/asdevlab/data/catalog/deep_sky/deep_sky.json](software/asdevlab/data/catalog/deep_sky/deep_sky.json)
- [software/asdevlab/data/catalog/stars/stars.json](software/asdevlab/data/catalog/stars/stars.json)
- [software/asdevlab/data/catalog/solar_system/solar_system.json](software/asdevlab/data/catalog/solar_system/solar_system.json)
- [software/asdevlab/data/catalog/satellites/satellites.json](software/asdevlab/data/catalog/satellites/satellites.json)
- [software/asdevlab/data/catalog/small_bodies/small_bodies.json](software/asdevlab/data/catalog/small_bodies/small_bodies.json)

### File-by-file audit

#### File: deep_sky.json

- Actual filename: [software/asdevlab/data/catalog/deep_sky/deep_sky.json](software/asdevlab/data/catalog/deep_sky/deep_sky.json)
- Number of objects: 60
- Schema: each entry contains id, name, type, ra, dec, messier, ngc, ic, alias, constellation, fun_fact, magnitude, epoch, coordinate_source.
- Available fields: id, name, type, magnitude, ra, dec, epoch, coordinate_source, messier, ngc, ic, alias, constellation, fun_fact.
- Missing fields: none for the common metadata set; the data is fairly complete for static deep-sky objects.
- Problems: the catalog is static and uses the same schema as other categories; it is suitable for basic pointing but not for dynamic coordinate management.

#### File: stars.json

- Actual filename: [software/asdevlab/data/catalog/stars/stars.json](software/asdevlab/data/catalog/stars/stars.json)
- Number of objects: 30
- Schema: similar to deep_sky.json, with id, name, type, ra, dec, magnitude, epoch, coordinate_source, constellation, alias, fun_fact.
- Available fields: id, name, type, magnitude, ra, dec, epoch, coordinate_source, constellation, alias, fun_fact.
- Missing fields: Messier/NGC/IC identifiers are absent for most star entries; that is acceptable for stars but makes the schema uneven.
- Problems: the file uses the same static-coordinate schema, which is fine for bright stars but not for high-precision current positions.

#### File: solar_system.json

- Actual filename: [software/asdevlab/data/catalog/solar_system/solar_system.json](software/asdevlab/data/catalog/solar_system/solar_system.json)
- Number of objects: 9
- Schema: currently uses id, name, type, mag, ra, dec, messier, ngc, ic, alias, constellation, fun_fact.
- Available fields: id, name, type, mag, alias, constellation, fun_fact.
- Missing fields: magnitude is present under mag rather than magnitude; ra/dec/epoch/coordinate_source are absent; there are no dynamic orbital fields.
- Problems: this file is not suitable for real solar-system positioning because the loader only reads magnitude from mag and ignores ra/dec/epoch because the current data model expects magnitude, ra, dec, and coordinate_source. The file is effectively metadata-only for the current pipeline.

#### File: satellites.json

- Actual filename: [software/asdevlab/data/catalog/satellites/satellites.json](software/asdevlab/data/catalog/satellites/satellites.json)
- Number of objects: 20
- Schema: similar to solar_system.json, with mag instead of magnitude and no epoch or coordinate_source fields.
- Available fields: id, name, type, mag, alias, constellation, fun_fact.
- Missing fields: ra, dec, epoch, coordinate_source, proper orbital elements, TLE data, and any object motion model.
- Problems: these are not real orbital targets in the current system; they are just static catalog entries with metadata.

#### File: small_bodies.json

- Actual filename: [software/asdevlab/data/catalog/small_bodies/small_bodies.json](software/asdevlab/data/catalog/small_bodies/small_bodies.json)
- Number of objects: 30
- Schema: similar to satellites.json, with mag instead of magnitude and no orbital metadata.
- Available fields: id, name, type, mag, alias, constellation, fun_fact.
- Missing fields: ra, dec, epoch, coordinate_source, orbital elements, and any ephemeris-related fields.
- Problems: the data is metadata-rich but not usable for real asteroid/comet motion calculations.

## 3. Data quality audit

### Schema consistency issues

| File | Problem | Severity | Recommendation |
|---|---|---|---|
| solar_system.json | Uses mag instead of magnitude | High | Normalize to magnitude for consistency with the main CatalogObject model |
| satellites.json | Uses mag instead of magnitude | High | Normalize to magnitude |
| small_bodies.json | Uses mag instead of magnitude | High | Normalize to magnitude |
| solar_system.json | Missing ra/dec/epoch/coordinate_source | High | Either provide static coordinates or define a dynamic provider model |
| satellites.json | Missing ra/dec/epoch/coordinate_source | High | Add orbital metadata or separate orbital schema |
| small_bodies.json | Missing ra/dec/epoch/coordinate_source | High | Add orbital metadata or separate orbital schema |
| all non-deep-sky files | Lack of dynamic position information | High | Introduce explicit provider/ephemeris fields |
| deep_sky.json and stars.json | Use static coordinates only | Medium | Keep as static catalog data, but make the pipeline explicit about this |
| all files | No TLE or orbital-element support exists in the current loader | High | Add support if satellites or small bodies are meant to be real targets |
| all files | The loader does not validate object type against the expected category | Medium | Add validation and warnings |

### Additional observations

- There are no duplicate object IDs in the current data files.
- The current loader does support some field-name flexibility: it reads either mag or magnitude, but it does not read other aliases such as ra_deg or dec_deg.
- The current catalog model does not include dedicated fields for orbital elements, TLEs, or ephemeris source.
- The current schema is sufficient for a static listing of objects, but it is not sufficient for time-varying objects.

## 4. Coordinate pipeline audit

### What happens when the user selects an object?

The actual path is:

1. The user selects an object through TargetService.
2. TargetService calls CatalogEngine::getObject(id).
3. TargetService resolves the object through CoordinateResolver.
4. The resulting RA/DEC is passed to MotionService.
5. MotionService calls mount_.gotoTarget(...).
6. The mount adapter sends the goto command to OnStepX.

### Does ASDEVLAB do the following?

1. Load catalog object? Yes.
2. Extract RA/DEC? Yes, but only from static fields in the CatalogObject.
3. Convert coordinates? Not in the current catalog pipeline. The resolver simply returns the stored RA/DEC.
4. Calculate Alt/Az? No, not in the catalog layer.
5. Send GOTO? Yes, via MotionService and the mount adapter.

### Coordinate calculations that exist

- RA/DEC handling: Implemented
  - The catalog loader parses ra/dec values into the CatalogObject model.
  - The resolver returns an EquatorialCoordinate.

- Alt/Az conversion: Missing
  - The current catalog layer does not compute alt/az from the current site/location/time.

- Epoch conversion: Missing
  - The model stores epoch but does not use it for conversion.

- Precession: Missing
  - No precession handling is present in the catalog pipeline.

- Nutation: Missing
  - No nutation handling is present.

- Refraction: Missing
  - No refraction correction is present.

- Current object position: Partial
  - For static objects, current RA/DEC is effectively the catalog-stored coordinate.
  - For solar-system, satellite, and small-body objects, there is no active ephemeris calculation.

### Summary of coordinate capability

- Implemented: static RA/DEC loading and transfer to motion.
- Partial: coordinate resolution via a provider abstraction.
- Missing: dynamic astronomical coordinate computation and mount-appropriate alt/az conversion.

## 5. Solar system object handling audit

### Solar system objects in scope

- Sun
- Moon
- Mercury
- Venus
- Mars
- Jupiter
- Saturn
- Uranus
- Neptune

### Current status

The current implementation is best described as static metadata entries with no active ephemeris engine.

- A. Static catalog coordinates? Not really; the current solar_system.json file contains no usable RA/DEC values for the current pipeline.
- B. Dynamic ephemeris objects? No.
- C. Hardcoded examples? In MotionService there are hardcoded sample targets such as M31, M42, Jupiter, and Saturn, but this is not a general solar-system ephemeris system.
- D. Not implemented? For real object positioning, yes: the current catalog system does not calculate solar-system positions from time/location.

### Astronomy library usage

- No astronomy library usage was found in the catalog implementation.
- No ephemeris provider was activated in the current resolver path.
- No time/location-aware solar-system coordinate calculation is wired into the catalog pipeline.

### Verdict

Solar-system handling is not implemented as a true astronomy feature. The catalog data files exist, but the runtime pipeline simply does not have a working ephemeris provider for them.

## 6. Satellite and small body audit

### Satellites

- File: [software/asdevlab/data/catalog/satellites/satellites.json](software/asdevlab/data/catalog/satellites/satellites.json)
- Current support: static catalog metadata only.
- TLE support: not present.
- SGP4 support: not present.
- Real-time position: not present.

### Small bodies

- File: [software/asdevlab/data/catalog/small_bodies/small_bodies.json](software/asdevlab/data/catalog/small_bodies/small_bodies.json)
- Current support: static catalog metadata only.
- Asteroid handling: not implemented as a real orbital solver.
- Comet handling: not implemented as a real orbital solver.
- Orbital calculation: not present.

### Verdict

The satellite and small-body categories are currently catalog entries with descriptions, not operational astronomical targets.

## 7. Catalog user workflow audit

### Current workflow

The user workflow that exists in code is:

1. Search object
2. Retrieve object by ID
3. Resolve coordinates
4. Send goto command

### Can ASDEVLAB currently do the following?

- Search object: Implemented
- Display information: Partial
  - The catalog model carries metadata fields such as magnitude, constellation, fun_fact, and identifiers.
  - The current code does not show a rich UI workflow in the catalog path itself.
- Show magnitude: Partial
  - The data contains magnitude and the model stores it.
  - The current catalog search path does not appear to expose it in a user-facing workflow beyond the object data model.
- Show RA/DEC: Partial
  - The object model stores RA/DEC.
  - The current runtime path resolves them and passes them to MotionService, but there is no rich display layer in the catalog service path itself.
- Calculate current position: Missing for dynamic objects; partial for static objects.
- Goto: Implemented via the motion path.

### Summary

The catalog system is functional as a lightweight object index and target-routing layer, but it is not yet a full user-facing astronomy catalog experience.

## 8. Compare with target design

### Intended design

The intended design in the request is:

- Static catalog + astronomy resolver + mount control
- Deep sky: catalog RA/DEC -> precession if needed -> current Alt/Az -> goto
- Planets: object ID -> ephemeris calculation -> current RA/DEC -> goto

### Current implementation vs intended design

| Design goal | Current status |
|---|---|
| Static catalog | Implemented |
| Astronomy resolver | Partial, but only a lightweight provider abstraction |
| Mount control | Implemented via MotionService and mount adapter |
| Deep-sky coordinate precession | Missing |
| Current Alt/Az conversion | Missing |
| Planet/solar-system ephemeris | Missing |
| Satellite orbital propagation | Missing |
| Small-body orbital propagation | Missing |

### Main difference from the intended design

The current implementation is a catalog-to-motion bridge, not a full astronomy computation pipeline. It can route a static object to the mount, but it does not yet compute current apparent positions for time-varying objects.

## 9. Architecture split proposal for the new design

The requested long-term architecture can be introduced incrementally without changing the UI or motion stack. The minimal boundary is:

```text
Catalog (metadata only)
    ↓
CatalogService
    ↓
AstronomyService (libnova)
    ↓
ResolvedTarget
    ↓
MotionService
    ↓
OnStepX
```

### 9.1 Proposed service responsibilities

#### CatalogService
- Own the static catalog database.
- Store only metadata such as object id, name, aliases, catalog identifiers, type, magnitude, angular size, constellation, description, and optional images.
- Never store calculated coordinates, alt/az, rise/set, or current RA/DEC.
- Expose lookup/search APIs for the rest of the app.
- Remain independent from libnova and from the catalog file format.

#### AstronomyService
- Own all astronomical computation.
- Convert static reference coordinates into current apparent coordinates using libnova.
- Handle current RA/DEC, epoch conversion, site/time/location-aware computations, altitude/azimuth, visibility, and rise/set.
- Return a ResolvedTarget object for use by MotionService.
- Be the only layer that knows about libnova.

#### CoordinateService
- Optional thin helper layer if the project wants a dedicated coordinate transformation boundary.
- It can encapsulate coordinate-system conversions, but it should remain a helper behind AstronomyService rather than a separate public dependency path.
- In the initial refactor, CoordinateService can be a lightweight adapter or alias around AstronomyService to keep the structure simple.

#### EphemerisService
- Own dynamic body resolution for solar system, moon, satellites, and small bodies.
- Provide a pluggable interface for future implementations.
- For now, it can be an interface-only placeholder that is not yet implemented.
- Satellite support should remain interface-only for now, with no SGP4 implementation.

### 9.2 Classes to keep

These classes are still appropriate for the new architecture:

- CatalogEngine
  - Keep as the catalog database access layer, but narrow its responsibility to metadata lookup and search.
- CatalogObject
  - Keep as the metadata record type.
- ResolvedTarget
  - Keep as the canonical output contract from astronomy resolution to motion control.
- ObservationContext
  - Keep as the input context for time/location/site-based astronomy resolution.
- TargetService
  - Keep, but refactor it so it does not directly own coordinate logic.

### 9.3 Classes to rename

These names should be adjusted to reflect the new architecture:

- CatalogEngine -> CatalogService
  - Reason: the class is no longer just an engine; it represents the metadata catalog service.
- CoordinateResolver -> AstronomyService or CoordinateService
  - Reason: it is better described as a resolver that hands off to astronomy computation rather than as a catalog utility.
- StaticCoordinateProvider -> StaticReferenceProvider
  - Reason: the current provider is still a reference-data concept, but the naming should not imply it is the primary astronomy engine.

### 9.4 Classes to split

These should be split along responsibility lines:

- CatalogEngine
  - Split into:
    - CatalogService: metadata lookup/search
    - Data source adapter: curated catalog JSON files only
- CoordinateResolver
  - Split into:
    - AstronomyService: orchestrates astronomy resolution
    - EphemerisService: dynamic object resolution for planets/moons/satellites/small bodies
    - CoordinateService: coordinate transformation helpers if needed
- Existing provider classes
  - Split the current provider-style code into:
    - static reference handling
    - dynamic ephemeris handling
    - satellite interface placeholder

### 9.5 Classes or pieces to remove

These should be removed or retired from the runtime path:

- Any class that directly stores computed RA/DEC in the catalog record as a long-term source of truth.
- Any class that mixes metadata storage with astronomical calculations.
- Any class that assumes the catalog format is the same as the astronomy computation format.
- The current provider implementations that behave like full astronomy engines should be reduced to placeholders or removed from the main runtime path.

### 9.6 New architecture diagram

```text
Catalog data source
    ↓
CatalogService
    ↓
AstronomyService
    ↓
ResolvedTarget
    ↓
MotionService
    ↓
OnStepX
```

### 9.7 Migration plan

1. Keep the current CatalogEngine and CatalogObject in place temporarily.
2. Introduce CatalogService as a thin rename/wrapper over the existing catalog access path.
3. Introduce AstronomyService as the only layer that knows how to build a ResolvedTarget.
4. Move all coordinate conversion and ephemeris logic out of the catalog layer and into AstronomyService/EphemerisService.
5. Keep MotionService unchanged and continue to receive ResolvedTarget values.
6. Keep the current catalog JSON loading path as the single curated catalog source for ASDEVLAB.
7. Replace the current provider-style classes gradually with interface-only placeholders for satellites and dynamic bodies.
8. Only after this boundary is stable should libnova be integrated.

### 9.8 Files requiring modification

The following files are the obvious touch points for the refactor:

- [software/asdevlab/include/asdevlab/catalog/catalog_engine.hpp](software/asdevlab/include/asdevlab/catalog/catalog_engine.hpp)
- [software/asdevlab/src/catalog/catalog_engine.cpp](software/asdevlab/src/catalog/catalog_engine.cpp)
- [software/asdevlab/include/asdevlab/catalog/catalog_object.hpp](software/asdevlab/include/asdevlab/catalog/catalog_object.hpp)
- [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp)
- [software/asdevlab/include/asdevlab/catalog/coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/static_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/static_coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/planet_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/planet_coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/moon_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/moon_coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/satellite_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/satellite_coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/small_body_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/small_body_coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/ephemeris_coordinate_provider.hpp](software/asdevlab/include/asdevlab/catalog/ephemeris_coordinate_provider.hpp)
- [software/asdevlab/include/asdevlab/catalog/resolved_target.hpp](software/asdevlab/include/asdevlab/catalog/resolved_target.hpp)
- [software/asdevlab/include/asdevlab/catalog/observation_context.hpp](software/asdevlab/include/asdevlab/catalog/observation_context.hpp)
- [software/asdevlab/include/asdevlab/services/target_service.hpp](software/asdevlab/include/asdevlab/services/target_service.hpp)
- [software/asdevlab/src/services/target_service.cpp](software/asdevlab/src/services/target_service.cpp)

### 9.9 Potential build issues

Potential issues to watch for during the migration:

- Renaming classes without updating all call sites will break the build.
- The current provider classes are header-only or near-header-only, so renaming or moving them must be done carefully to avoid linker issues.
- The target service currently depends on the catalog and resolver types directly; introducing new service names will require a small compatibility layer.
- The current catalog headers live under the same namespace as the new service concepts; the split should preserve namespace boundaries to avoid churn.
- The CMake target will need new source files if the new service classes are implemented as real C++ files rather than header-only adapters.

## 10. Final assessment

The current codebase already has a useful separation between catalog access and motion execution, but it still mixes static metadata and coordinate resolution in the same path. The safest next step is to preserve the existing catalog model and target service while introducing a new service boundary for astronomy computation. That gives the project a maintainable migration path toward libnova integration without touching UI, motion, or mount communication, while keeping the catalog curated internally.

### Current strengths

- The catalog data is present and structured.
- The catalog loader is real and functional.
- The object model is simple and practical.
- The catalog can be searched and used to route a selected object to a goto command.
- The code is already organized around a separation between catalog data and motion execution.

### Current implemented features

- Load catalog JSON files into memory
- Search by identifier/name/type/alias
- Store object metadata
- Resolve static RA/DEC values
- Send selected objects to the motion layer for goto/sync/preview

### Current problems

- The catalog pipeline is static-only for the common path.
- The resolver does not compute current apparent coordinates for dynamic objects.
- The solar-system, satellite, and small-body categories do not have a working runtime positioning model.
- The schema is inconsistent across categories.
- The loader does not validate or normalize the catalog schema thoroughly.
- The code path is more of a catalog-to-mount bridge than a true astronomy resolver.

### Architectural risks

- The current design can make it easy to overstate catalog capability because the code appears to support many object categories while the runtime pipeline only truly works for static objects.
- The current architecture would need a real ephemeris provider layer before satellites or small bodies can be treated as real targets.
- The current static-coordinate path is fine for basic deep-sky and bright-star pointing, but it is not sufficient for a serious astronomy catalog experience.

### Missing components

- A real ephemeris provider for solar-system objects
- A real orbital propagation layer for satellites and small bodies
- Current-position calculations for site/time/location
- Alt/Az conversion from RA/DEC
- Epoch/precession/nutation/refraction handling
- A richer catalog UI/workflow layer

## Priority recommendations

### P0

- Make the catalog schema consistent across all category files.
- Ensure the runtime pipeline clearly distinguishes static objects from dynamic ones.
- Prevent non-physical or unsupported categories from being presented as real goto targets.

### P1

- Add a real ephemeris provider abstraction and activate it for solar-system objects.
- Add a clear dynamic-object coordinate path for satellites and small bodies.
- Add site/time/location-aware coordinate calculation for current object position.

### P2

- Add a real astronomy resolver that computes current apparent coordinates before goto.
- Add altitude/azimuth conversion and proper coordinate transformations.
- Make the catalog system a genuine astronomy workflow rather than a simple static target index.

### P3

- Add richer catalog UI support, object detail views, and observation planning features.
- Add support for more advanced orbital datasets such as TLEs and comet elements.
