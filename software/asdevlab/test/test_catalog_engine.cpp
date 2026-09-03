#include "asdevlab/catalog/catalog_data_source.hpp"
#include "asdevlab/catalog/catalog_engine.hpp"
#include "asdevlab/catalog/coordinate_resolver.hpp"
#include "asdevlab/catalog/ephemeris_coordinate_provider.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <cassert>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

static void writeSampleJson(const std::string& path) {
    std::ofstream out(path);
    out << R"({
  "name": "core",
  "objects": [
    {
      "id": "M31",
      "name": "Andromeda Galaxy",
      "provider": "static",
      "type": "Galaxy",
      "magnitude": 3.44,
      "ra": 0.712,
      "dec": 41.269
    },
    {
      "id": "NGC7000",
      "name": "North America Nebula",
      "provider": "static",
      "type": "Nebula",
      "magnitude": 4.0,
      "ra": 20.953,
      "dec": 44.318
    },
    {
      "id": "Mars",
      "name": "Mars",
      "provider": "ephemeris",
      "type": "Planet",
      "magnitude": 1.0,
      "ra": 0.0,
      "dec": 0.0
    }
  ]
})";
}

int main() {
    struct InMemoryCatalogDataSource : public asdevlab::catalog::CatalogDataSource {
        std::vector<asdevlab::catalog::CatalogObject> load() const override {
            std::vector<asdevlab::catalog::CatalogObject> objects;
            asdevlab::catalog::CatalogObject object;
            object.id = "MEM1";
            object.name = "In Memory Target";
            object.type = "Star";
            object.provider = "static";
            object.ra = 1.0;
            object.dec = 2.0;
            object.coordinate_source = "catalog";
            objects.push_back(object);

            asdevlab::catalog::CatalogObject messier_object;
            messier_object.id = "CAT10";
            messier_object.name = "Messier 31 Proxy";
            messier_object.display_name = "Messier 31 Proxy";
            messier_object.type = "Galaxy";
            messier_object.provider = "static";
            messier_object.ra = 0.712;
            messier_object.dec = 41.269;
            messier_object.coordinate_source = "catalog";
            messier_object.messier = "M31";
            messier_object.ngc = "NGC224";
            objects.push_back(messier_object);
            return objects;
        }
    };

    asdevlab::catalog::CatalogEngine in_memory_engine;
    assert(in_memory_engine.loadCatalog(InMemoryCatalogDataSource{}));
    auto memory_object = in_memory_engine.getObject("MEM1");
    assert(memory_object.has_value());
    assert((*memory_object)->name == "In Memory Target");

    auto memory_search = in_memory_engine.search("M31");
    assert(memory_search.size() == 1);
    assert(memory_search.front().id == "CAT10");

    auto ngc_search = in_memory_engine.search("NGC224");
    assert(ngc_search.size() == 1);
    assert(ngc_search.front().id == "CAT10");

    const std::string path = "catalog_core_test.json";
    writeSampleJson(path);

    asdevlab::catalog::CatalogEngine engine;
    assert(engine.loadCatalog(path));
    assert(engine.loadCatalog(path));

    auto object_opt = engine.getObject("M31");
    assert(object_opt.has_value());
    const auto* object = *object_opt;
    assert(object->name == "Andromeda Galaxy");

    auto search_results = engine.search("Andromeda");
    assert(search_results.size() == 1);
    assert(search_results.front().id == "M31");

    search_results = engine.search("ngc");
    assert(search_results.size() == 1);
    assert(search_results.front().id == "NGC7000");

    auto mars_opt = engine.getObject("Mars");
    assert(mars_opt.has_value());
    const auto* mars_object = *mars_opt;
    assert(mars_object->ra == 0.0);
    assert(mars_object->dec == 0.0);

    search_results = engine.search("mars");
    assert(search_results.size() == 1);
    assert(search_results.front().id == "Mars");

    struct TestEphemerisProvider : public asdevlab::catalog::EphemerisCoordinateProvider {
        bool supports(const asdevlab::catalog::CatalogObject& object) const override {
            return object.type == "Planet";
        }

        asdevlab::catalog::ResolvedTarget resolve(
            const asdevlab::catalog::CatalogObject& /*object*/,
            const asdevlab::catalog::ObservationContext& /*context*/
        ) const override {
            asdevlab::catalog::ResolvedTarget target;
            target.ra_hours = 5.123;
            target.dec_degrees = -22.456;
            target.visibility = true;
            return target;
        }

        asdevlab::catalog::EquatorialCoordinate getCoordinates(
            const asdevlab::catalog::CatalogObject& /*object*/,
            const asdevlab::catalog::ObservationContext& /*context*/
        ) const override {
            return {5.123, -22.456};
        }
    };

    asdevlab::catalog::CoordinateResolver resolver(std::make_shared<TestEphemerisProvider>());
    asdevlab::catalog::ObservationContext context;
    context.utc_time = std::chrono::system_clock::now();
    context.latitude_degrees = 34.0;
    context.longitude_degrees = -118.0;
    context.elevation_meters = 89.0;
    context.timezone = "UTC";

    auto dynamic_target = resolver.resolveTarget(*mars_object, context);
    assert(dynamic_target.ra_hours == 5.123);
    assert(dynamic_target.dec_degrees == -22.456);
    assert(dynamic_target.visibility);

    auto m31_opt = engine.getObject("M31");
    assert(m31_opt.has_value());
    const auto* loaded_m31 = *m31_opt;
    auto static_target = resolver.resolveTarget(*loaded_m31, context);
    assert(static_target.ra_hours == 0.712);
    assert(static_target.dec_degrees == 41.269);
    assert(static_target.epoch == "J2000");

    auto nebula_results = engine.filterByType("nebula");
    assert(nebula_results.size() == 1);
    assert(nebula_results.front()->id == "NGC7000");

    const std::filesystem::path recursive_dir = "catalog_recursive_test";
    std::filesystem::create_directories(recursive_dir / "nested");
    {
        std::ofstream recursive_a(recursive_dir / "alpha.json");
        recursive_a << R"({
  "name": "alpha",
  "objects": [
    { "id": "ALPHA", "name": "Alpha Target", "type": "Star", "magnitude": 2.5, "ra": 1.0, "dec": 2.0 }
  ]
})";
    }
    {
        std::ofstream recursive_b(recursive_dir / "nested" / "beta.json");
        recursive_b << R"({
  "name": "beta",
  "objects": [
    { "id": "BETA", "name": "Beta Target", "type": "Planet", "magnitude": 3.0, "ra": 3.0, "dec": 4.0 }
  ]
})";
    }

    asdevlab::catalog::CatalogEngine recursive_engine;
    assert(recursive_engine.loadCatalog(recursive_dir.string()));
    auto recursive_alpha = recursive_engine.getObject("ALPHA");
    assert(recursive_alpha.has_value());
    auto recursive_beta = recursive_engine.getObject("BETA");
    assert(recursive_beta.has_value());
    auto recursive_search = recursive_engine.search("target");
    assert(recursive_search.size() == 2);
    std::filesystem::remove_all(recursive_dir);

    std::remove(path.c_str());

    const std::string solar_system_path = "../data/catalog/solar_system/solar_system.json";
    const std::string deep_sky_path = "../data/catalog/deep_sky/deep_sky.json";
    const std::string stars_path = "../data/catalog/stars/stars.json";

    asdevlab::catalog::CatalogEngine solar_system_engine;
    assert(solar_system_engine.loadCatalog(solar_system_path));
    auto moon = solar_system_engine.getObject("moon");
    assert(moon.has_value());
    assert((*moon)->coordinate_source == "ephemeris");
    auto moon_search = solar_system_engine.search("Luna");
    assert(!moon_search.empty());
    assert(moon_search.front().id == "moon");

    asdevlab::catalog::CoordinateResolver real_resolver;
    auto moon_target = real_resolver.resolveTarget(**moon, context);
    assert(std::isfinite(moon_target.ra_hours));
    assert(std::isfinite(moon_target.dec_degrees));
    assert(moon_target.epoch == "J2000");

    auto mars = solar_system_engine.getObject("planet_mars");
    assert(mars.has_value());
    auto mars_target = real_resolver.resolveTarget(**mars, context);
    assert(std::isfinite(mars_target.ra_hours));
    assert(std::isfinite(mars_target.dec_degrees));

    auto sun = solar_system_engine.getObject("sun");
    assert(sun.has_value());
    auto sun_target = real_resolver.resolveTarget(**sun, context);
    assert(std::isfinite(sun_target.ra_hours));
    assert(std::isfinite(sun_target.dec_degrees));

    asdevlab::catalog::CatalogEngine deep_sky_engine;
    assert(deep_sky_engine.loadCatalog(deep_sky_path));
    auto m42 = deep_sky_engine.getObject("M42");
    assert(m42.has_value());
    assert((*m42)->name == "Orion Nebula");
    assert(std::abs((*m42)->ra - 5.5881) < 1e-4);
    assert(std::abs((*m42)->dec + 5.3911) < 1e-4);

    asdevlab::catalog::CatalogEngine stars_engine;
    assert(stars_engine.loadCatalog(stars_path));
    auto sirius = stars_engine.getObject("Sirius");
    assert(sirius.has_value());
    assert(std::abs((*sirius)->ra - 6.7525) < 1e-4);
    assert(std::abs((*sirius)->dec + 16.7161) < 1e-4);

    auto sirius_search = stars_engine.search("Sirius");
    assert(!sirius_search.empty());

    auto orion_search = deep_sky_engine.search("Orion Nebula");
    assert(!orion_search.empty());

    asdevlab::catalog::ObservationContext deep_sky_context;
    auto m42_target = resolver.resolveTarget(**m42, deep_sky_context);
    assert(std::abs(m42_target.ra_hours - 5.5881) < 1e-4);
    assert(std::abs(m42_target.dec_degrees + 5.3911) < 1e-4);

    auto sirius_target = resolver.resolveTarget(**sirius, context);
    assert(std::abs(sirius_target.ra_hours - 6.7525) < 1e-4);
    assert(std::abs(sirius_target.dec_degrees + 16.7161) < 1e-4);

    asdevlab::catalog::CatalogObject static_object;
    static_object.type = "Star";
    static_object.provider = "static";
    static_object.coordinate_source = "catalog";
    static_object.epoch = "J2000";
    static_object.ra = 1.0;
    static_object.dec = 2.0;
    assert(static_object.isValidForResolution());

    asdevlab::catalog::CatalogObject missing_static_coordinates;
    missing_static_coordinates.type = "Star";
    missing_static_coordinates.provider = "static";
    missing_static_coordinates.coordinate_source = "catalog";
    missing_static_coordinates.epoch = "J2000";
    assert(!missing_static_coordinates.isValidForResolution());

    asdevlab::catalog::CatalogObject ephemeris_object;
    ephemeris_object.type = "Planet";
    ephemeris_object.provider = "ephemeris";
    ephemeris_object.coordinate_source = "ephemeris";
    ephemeris_object.epoch = "J2000";
    assert(ephemeris_object.isValidForResolution());

    asdevlab::catalog::CatalogObject unknown_object;
    unknown_object.coordinate_source = "unknown";
    unknown_object.ra = 1.0;
    unknown_object.dec = 2.0;
    assert(!unknown_object.isValidForResolution());
    auto unknown_target = resolver.resolveTarget(unknown_object, context);
    assert(unknown_target.ra_hours == 0.0);
    assert(unknown_target.dec_degrees == 0.0);
    assert(!unknown_target.visibility);

    std::cout << "catalog engine unit test passed\n";
    return 0;
}
