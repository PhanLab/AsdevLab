#include "asdevlab/web/catalog_api.hpp"
#include "asdevlab/web/target_api.hpp"
#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <iostream>
#include <fstream>

int main() {
    asdevlab::TelescopeCore core;

    // prepare a small catalog with M31 via TargetService -> CatalogEngine
    const std::string path = "catalog_api_test.json";
    std::ofstream out(path);
    out << R"({
  "name": "core",
  "objects": [
    { "id": "M31", "name": "Andromeda Galaxy", "type": "Galaxy", "magnitude": 3.44, "ra": 0.712, "dec": 41.269 }
  ]
})";
    out.close();

    // load package into core's catalog engine
    assert(core.catalog().loadCatalog(path));

    // Test catalog search helper via the thin REST helper
    auto search_json = asdevlab::web::catalog_search_json(core, std::string("M31"));
    // ensure response contains M31
    assert(search_json.find("M31") != std::string::npos);

    core.observation().setCurrentTarget("M31");
    auto current_json = asdevlab::web::target_current_json(core);
    assert(current_json.find("\"id\":\"M31\"") != std::string::npos);
    assert(current_json.find("\"ok\":true") != std::string::npos);

    std::remove(path.c_str());

    std::cout << "api integration smoke test passed\n";
    return 0;
}
