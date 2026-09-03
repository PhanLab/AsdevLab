#include "asdevlab/web/catalog_api.hpp"
#include "asdevlab/web/api_utils.hpp"

using namespace httplib_for_ols;

namespace asdevlab {
namespace web {

static std::string serialize_object_summary(const catalog::CatalogObject& obj) {
    std::ostringstream oss;
    oss << "{\"id\":\"" << obj.id << "\",\"name\":\"" << obj.name << "\",\"type\":\"" << obj.type << "\",\"magnitude\":" << obj.magnitude << "}";
    return oss.str();
}

std::string catalog_search_json(TelescopeCore& core, const std::string& keyword) {
    auto sr = core.target().search(keyword);
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (const auto& obj : sr.results) {
        if (!first) oss << ",";
        first = false;
        oss << serialize_object_summary(obj);
    }
    oss << "]";
    return make_success(oss.str());
}

std::string catalog_object_json(TelescopeCore& core, const std::string& id) {
    const auto* obj = core.target().getObject(id);
    if (!obj) return make_error("object not found", 404);
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":\"" << obj->id << "\",";
    oss << "\"name\":\"" << obj->name << "\",";
    oss << "\"type\":\"" << obj->type << "\",";
    oss << "\"magnitude\":" << obj->magnitude << ",";
    oss << "\"ra\":" << obj->ra << ",";
    oss << "\"dec\":" << obj->dec;
    oss << "}";
    return make_success(oss.str());
}

void register_catalog_api(Server& svr, TelescopeCore& core) {
    svr.Get(R"(/api/catalog/search)", [&](const Request& req, Response& res) {
        const auto q = req.has_param("q") ? req.get_param_value("q") : std::string();
        if (q.empty()) {
            res.status = 400;
            res.set_content(make_error("missing query parameter q"), "application/json");
            return;
        }
        res.set_content(catalog_search_json(core, q), "application/json");
    });

    svr.Get(R"(/api/catalog/object/([A-Za-z0-9_\-]+))", [&](const Request& req, Response& res) {
        const std::string id = req.matches[1];
        res.set_content(catalog_object_json(core, id), "application/json");
    });
}

} // namespace web
} // namespace asdevlab
