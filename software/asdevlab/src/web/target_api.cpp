#include "asdevlab/web/target_api.hpp"
#include "asdevlab/web/api_utils.hpp"

#include <sstream>

using namespace httplib_for_ols;

namespace asdevlab {
namespace web {

std::string escape_json(const std::string& value) {
    std::ostringstream oss;
    for (char ch : value) {
        switch (ch) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default: oss << ch; break;
        }
    }
    return oss.str();
}

std::string workflow_status_json(const ObservationService::TargetWorkflowStatus& status) {
    std::ostringstream oss;
    oss << "{\"ok\":" << (status.ok ? "true" : "false")
        << ",\"state\":\"" << escape_json(status.stateName()) << "\""
        << ",\"progress\":" << status.progress
        << ",\"resolved\":" << (status.resolved_target.resolved ? "true" : "false")
        << ",\"previewed\":" << (status.isPreviewed() ? "true" : "false")
        << ",\"visible\":" << (status.resolved_target.visibility ? "true" : "false")
        << ",\"goto_dispatched\":" << (status.isGotoDispatched() ? "true" : "false")
        << ",\"tracking_active\":" << (status.isTrackingActive() ? "true" : "false")
        << ",\"aborted\":" << (status.isAborted() ? "true" : "false")
        << ",\"stage\":\"" << escape_json(status.legacyStage()) << "\""
        << ",\"message\":\"" << escape_json(status.message) << "\""
        << ",\"monitoring_message\":\"" << escape_json(status.monitoring_message) << "\""
        << ",\"steps\":[";
    for (size_t i = 0; i < status.steps.size(); ++i) {
        if (i != 0) {
            oss << ",";
        }
        oss << "{\"name\":\"" << escape_json(status.steps[i].name) << "\""
            << ",\"ok\":" << (status.steps[i].ok ? "true" : "false")
            << ",\"message\":\"" << escape_json(status.steps[i].message) << "\"}";
    }
    oss << "]}";
    return oss.str();
}

std::string target_search_json(TelescopeCore& core, const std::string& query) {
    const auto search_result = core.target().search(query);
    std::ostringstream oss;
    oss << "{\"query\":\"" << escape_json(query) << "\""
        << ",\"results\":[";
    for (size_t i = 0; i < search_result.results.size(); ++i) {
        if (i != 0) {
            oss << ",";
        }
        oss << "{\"id\":\"" << escape_json(search_result.results[i].id) << "\""
            << ",\"name\":\"" << escape_json(search_result.results[i].name) << "\"}";
    }
    oss << "]}";
    return make_success(oss.str());
}

std::string target_preview_json(TelescopeCore& core, const std::string& id, const catalog::ObservationContext& ctx) {
    const auto preview_result = core.target().previewObject(id, ctx);
    if (!preview_result.ok) {
        return make_error(preview_result.log);
    }
    std::ostringstream oss;
    oss << "{\"id\":\"" << escape_json(id) << "\",\"log\":\"" << escape_json(preview_result.log) << "\"}";
    return make_success(oss.str());
}

std::string target_goto_json(TelescopeCore& core, const std::string& id) {
    auto ctx = core.observation().makeObservationContext();
    const auto workflow = core.observation().runTargetWorkflow(id, ctx, true);
    return make_success(workflow_status_json(workflow));
}

std::string target_sync_json(TelescopeCore& core, const std::string& id) {
    auto ctx = core.observation().makeObservationContext();
    const auto workflow = core.observation().runTargetWorkflow(id, ctx, false);
    return make_success(workflow_status_json(workflow));
}

std::string target_abort_json(TelescopeCore& core) {
    auto ctx = core.observation().makeObservationContext();
    const auto workflow = core.observation().abortTargetWorkflow(ctx);
    return make_success(workflow_status_json(workflow));
}

std::string target_status_json(TelescopeCore& core) {
    std::string log;
    const auto status_result = core.motion().status(log);
    std::ostringstream oss;
    oss << "{\"ok\":" << (status_result.ok ? "true" : "false")
        << ",\"status\":\"" << escape_json(log) << "\"}";
    return make_success(oss.str());
}

std::string target_current_json(TelescopeCore& core) {
    const auto current_id = core.observation().currentTargetId();
    if (current_id.empty()) {
        return make_error("no current target");
    }

    const auto current_object = core.target().getObject(current_id);
    if (!current_object) {
        return make_error("current target not found");
    }

    std::ostringstream oss;
    oss << "{\"id\":\"" << escape_json(current_object->id) << "\""
        << ",\"name\":\"" << escape_json(current_object->name) << "\""
        << ",\"type\":\"" << escape_json(current_object->type) << "\"}";
    return make_success(oss.str());
}

void register_target_api(Server& svr, TelescopeCore& core) {
    svr.Get(R"(/api/target/search)", [&](const Request& req, Response& res) {
        const auto query = req.has_param("q") ? req.get_param_value("q") : std::string();
        if (query.empty()) {
            res.status = 400;
            res.set_content(make_error("missing q parameter"), "application/json");
            return;
        }
        res.set_content(target_search_json(core, query), "application/json");
    });

    svr.Get(R"(/api/target/preview)", [&](const Request& req, Response& res) {
        const auto id = req.has_param("id") ? req.get_param_value("id") : std::string();
        if (id.empty()) {
            res.status = 400;
            res.set_content(make_error("missing id parameter"), "application/json");
            return;
        }
        auto ctx = core.observation().makeObservationContext();
        res.set_content(target_preview_json(core, id, ctx), "application/json");
    });

    svr.Post(R"(/api/target/goto)", [&](const Request& req, Response& res) {
        const auto id = asdevlab::web::extract_json_string(req.body, "id");
        if (id.empty()) {
            res.status = 400;
            res.set_content(make_error("missing id in body"), "application/json");
            return;
        }
        res.set_content(target_goto_json(core, id), "application/json");
    });

    svr.Post(R"(/api/target/abort)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(target_abort_json(core), "application/json");
    });

    svr.Get(R"(/api/target/current)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(target_current_json(core), "application/json");
    });

    svr.Get(R"(/api/target/status)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(target_status_json(core), "application/json");
    });

    svr.Post(R"(/api/target/sync)", [&](const Request& req, Response& res) {
        const auto id = asdevlab::web::extract_json_string(req.body, "id");
        if (id.empty()) {
            res.status = 400;
            res.set_content(make_error("missing id in body"), "application/json");
            return;
        }
        res.set_content(target_sync_json(core, id), "application/json");
    });
}

} // namespace web
} // namespace asdevlab
