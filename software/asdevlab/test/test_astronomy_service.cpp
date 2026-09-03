#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"

#include <cassert>
#include <chrono>
#include <memory>

int main() {
    using namespace asdevlab;

    auto provider = std::make_shared<astronomy::LibnovaEphemerisProvider>();
    astronomy::AstronomyService service(provider);

    catalog::ObservationContext context;
    context.utc_time = std::chrono::system_clock::from_time_t(1735689600);
    context.latitude_degrees = 34.0;
    context.longitude_degrees = -118.0;
    context.elevation_meters = 89.0;
    context.pressure_mbar = 1013.25;
    context.temperature_c = 10.0;
    context.timezone = "UTC";

    const auto sun = service.resolve("sun", context);
    assert(sun.ra_hours != 0.0 || sun.dec_degrees != 0.0 || sun.alt_degrees != 0.0 || sun.az_degrees != 0.0);
    assert(!sun.rise.empty() && !sun.transit.empty() && !sun.set.empty());

    const auto moon = service.resolve("moon", context);
    assert(moon.ra_hours != 0.0 || moon.dec_degrees != 0.0 || moon.alt_degrees != 0.0 || moon.az_degrees != 0.0);
    assert(!moon.rise.empty() && !moon.transit.empty() && !moon.set.empty());

    const auto mars = service.resolve("mars", context);
    assert(mars.ra_hours != 0.0 || mars.dec_degrees != 0.0 || mars.alt_degrees != 0.0 || mars.az_degrees != 0.0);
    assert(!mars.rise.empty() && !mars.transit.empty() && !mars.set.empty());

    const auto mercury = service.resolve("mercury", context);
    assert(mercury.ra_hours != 0.0 || mercury.dec_degrees != 0.0 || mercury.alt_degrees != 0.0 || mercury.az_degrees != 0.0);
    assert(!mercury.rise.empty() && !mercury.transit.empty() && !mercury.set.empty());

    const auto venus = service.resolve("venus", context);
    assert(venus.ra_hours != 0.0 || venus.dec_degrees != 0.0 || venus.alt_degrees != 0.0 || venus.az_degrees != 0.0);
    assert(!venus.rise.empty() && !venus.transit.empty() && !venus.set.empty());

    const auto jupiter = service.resolve("jupiter", context);
    assert(jupiter.ra_hours != 0.0 || jupiter.dec_degrees != 0.0 || jupiter.alt_degrees != 0.0 || jupiter.az_degrees != 0.0);
    assert(!jupiter.rise.empty() && !jupiter.transit.empty() && !jupiter.set.empty());

    const auto saturn = service.resolve("saturn", context);
    assert(saturn.ra_hours != 0.0 || saturn.dec_degrees != 0.0 || saturn.alt_degrees != 0.0 || saturn.az_degrees != 0.0);
    assert(!saturn.rise.empty() && !saturn.transit.empty() && !saturn.set.empty());

    const auto uranus = service.resolve("uranus", context);
    assert(uranus.ra_hours != 0.0 || uranus.dec_degrees != 0.0 || uranus.alt_degrees != 0.0 || uranus.az_degrees != 0.0);
    assert(!uranus.rise.empty() && !uranus.transit.empty() && !uranus.set.empty());

    const auto neptune = service.resolve("neptune", context);
    assert(neptune.ra_hours != 0.0 || neptune.dec_degrees != 0.0 || neptune.alt_degrees != 0.0 || neptune.az_degrees != 0.0);
    assert(!neptune.rise.empty() && !neptune.transit.empty() && !neptune.set.empty());

    return 0;
}
