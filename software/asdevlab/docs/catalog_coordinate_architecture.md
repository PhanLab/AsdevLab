# ASDEVLAB Catalog Coordinate Architecture

## Overview

This refactor separates catalog metadata from coordinate production.
CatalogEngine remains metadata-only and supports two object categories:

- `static` objects: fixed J2000 coordinates stored in the loaded catalog JSON.
- `ephemeris` objects: dynamic solar-system objects whose coordinates are resolved later using observation context.

## Folder Structure

software/asdevlab/
- include/asdevlab/catalog/
  - `catalog_object.hpp`
  - `catalog_engine.hpp`
  - `coordinate_provider.hpp`
  - `static_coordinate_provider.hpp`
  - `ephemeris_coordinate_provider.hpp`
  - `coordinate_resolver.hpp`
  - `equatorial_coordinate.hpp`
  - `observation_context.hpp`
- src/catalog/
  - `catalog_engine.cpp`
  - `static_coordinate_provider.cpp`
  - `coordinate_resolver.cpp`
- test/
  - `test_catalog_engine.cpp`

## Class Diagram

```text
CatalogEngine
  - loadCatalog(path)
  - search(keyword)
  - getObject(id)
  - filterByType(type)
  - filterByConstellation(name)
    |
    +-- CatalogObject
                     - id
                     - display_name
                     - provider ("static" | "ephemeris")
                     - ra_hours
                     - dec_degrees
                     - aliases
                     - object_type
                     - constellation
                     - magnitude
                     - angular_size
                     - description

CoordinateProvider (interface)
  - getCoordinates(object, context)
    |
    +-- StaticCoordinateProvider
    +-- EphemerisCoordinateProvider

CoordinateResolver
  - resolve(object, context)
    - if provider == "ephemeris" => ephemeris_provider->getCoordinates()
    - else => static provider returns stored RA/DEC

ObservationContext
  - utc_time
  - latitude_degrees
  - longitude_degrees
  - elevation_meters
  - timezone

ASDEVLAB keeps this context intentionally simple: it uses the host computer's local date/time, and only carries sidereal time from the mount when OnStep needs it.

EquatorialCoordinate
  - ra_hours
  - dec_degrees
```

## Key Interfaces

### CatalogObject
- `provider`: "static" or "ephemeris".
- Static objects store `ra_hours` and `dec_degrees` in the loaded JSON catalog.
- Ephemeris objects do not depend on stored RA/DEC and can be resolved later.

### CoordinateProvider
- Abstract interface for obtaining equatorial coordinates.
- `StaticCoordinateProvider` returns stored RA/DEC for static objects.
- `EphemerisCoordinateProvider` is an abstract stub to be implemented later by astronomical engines.

### CoordinateResolver
- Chooses the correct provider based on `CatalogObject::provider`.
- Ensures MotionService always receives `EquatorialCoordinate`.
- Keeps CatalogEngine protocol-independent and unaware of computation details.

## Search Behavior

Search remains catalog-wide and provider-agnostic.
The same search APIs work for static and dynamic objects like `M31`, `Mars`, `Moon`, `Sun`, and `NGC7000`.

## Future Compatibility

This architecture permits later integration of different ephemeris engines:
- SOFA
- ERFA
- Skyfield
- SPICE
- JPL DE

Only the concrete `EphemerisCoordinateProvider` implementation needs to change; `CatalogEngine` and search remain unchanged.

## Astronomy Backend Ownership

ASDEVLAB owns the astronomy workflow for target resolution and observation context handling.
Libnova is now vendored as an internal calculation backend under the repository's external tree and is used only through the ASDEVLAB astronomy wrapper layer.

- `AstronomyService` is the only component that speaks to the astronomy provider abstraction.
- `LibnovaEphemerisProvider` is the internal implementation detail that translates observation context into RA/DEC/ALT/AZ outputs.
- CatalogService, MotionService, and LX200 transport remain unchanged.

## Notes

- The initial libnova-backed implementation supports Sun, Moon, and Mars.
- No MotionService or WebUI logic was modified.
- Libnova headers are not exposed outside the astronomy wrapper boundary.
