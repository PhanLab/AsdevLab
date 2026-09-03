#include "asdevlab/web/mount_api.hpp"
#include "asdevlab/web/api_utils.hpp"
#include "asdevlab/hardware/mount/mount_config.hpp"

#include <sstream>
#include <cctype>

// Minimal JSON string escaper (kept local to this translation unit).
static std::string escape_json(const std::string& value) {
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

using namespace httplib_for_ols;

namespace asdevlab {
namespace web {

static bool parse_double(const std::string& s, double& out) {
    try {
        size_t idx = 0;
        out = std::stod(s, &idx);
        return idx == s.size();
    } catch (...) {
        return false;
    }
}

ParsedGoto parse_goto_body(const std::string& body) {
    ParsedGoto p;
    // Try RA/DEC as strings first
    const std::string ra_s = web::extract_json_string(body, "ra");
    const std::string dec_s = web::extract_json_string(body, "dec");
    bool has_ra = !ra_s.empty();
    bool has_dec = !dec_s.empty();

    // Helper to extract numeric fields (unquoted numbers)
    auto extract_number = [&](const std::string& key, double& out)->bool{
        const std::string keyq = '"' + key + '"';
        auto pos = body.find(keyq);
        if (pos == std::string::npos) return false;
        pos = body.find(':', pos);
        if (pos == std::string::npos) return false;
        pos++;
        while (pos < body.size() && isspace((unsigned char)body[pos])) pos++;
        if (pos >= body.size()) return false;
        // parse until comma or closing brace
        size_t end = pos;
        while (end < body.size() && ( (body[end] >= '0' && body[end] <= '9') || body[end] == '.' || body[end] == '-' || body[end] == '+' || body[end] == 'e' || body[end] == 'E')) end++;
        if (end == pos) return false;
        std::string token = body.substr(pos, end - pos);
        try {
            size_t idx = 0;
            out = std::stod(token, &idx);
            return idx == token.size();
        } catch (...) {
            return false;
        }
    };

    double alt_v = 0.0, az_v = 0.0;
    const bool has_alt = extract_number("alt", alt_v);
    const bool has_az = extract_number("az", az_v);

    // Mixed systems are rejected
    if ((has_ra || has_dec) && (has_alt || has_az)) {
        p.type = ParsedGoto::Type::Invalid;
        p.error = "mixing coordinate systems";
        return p;
    }

    if (has_ra || has_dec) {
        if (!has_ra || !has_dec) {
            p.type = ParsedGoto::Type::Invalid;
            p.error = "incomplete ra/dec pair";
            return p;
        }
        double ra = 0.0, dec = 0.0;
        if (!parse_double(ra_s, ra) || !parse_double(dec_s, dec)) {
            p.type = ParsedGoto::Type::Invalid;
            p.error = "invalid ra/dec values";
            return p;
        }
        p.type = ParsedGoto::Type::RADEC;
        p.ra_hours = ra;
        p.dec_deg = dec;
        return p;
    }

    if (has_alt || has_az) {
        if (!has_alt || !has_az) {
            p.type = ParsedGoto::Type::Invalid;
            p.error = "incomplete alt/az pair";
            return p;
        }
        if (!std::isfinite(alt_v) || !std::isfinite(az_v)) {
            p.type = ParsedGoto::Type::Invalid;
            p.error = "invalid alt/az values";
            return p;
        }
        p.type = ParsedGoto::Type::ALTAZ;
        p.alt_deg = alt_v;
        p.az_deg = az_v;
        return p;
    }

    p.type = ParsedGoto::Type::Invalid;
    p.error = "missing coordinates";
    return p;
}

static double ra_hours_from(const hardware::RightAscension& ra) {
    return ra.hours + (ra.minutes / 60.0) + (ra.seconds / 3600.0);
}

static double dec_degrees_from(const hardware::Declination& dec) {
    const double abs_deg = dec.degrees + (dec.minutes / 60.0) + (dec.seconds / 3600.0);
    return dec.negative ? -abs_deg : abs_deg;
}

std::string mount_status_json(TelescopeCore& core, bool include_raw) {
    hardware::MountStatusRequest req;
    req.include_raw = include_raw;
    const auto resp = core.mountStatus(req);
    std::ostringstream oss;
    oss << "{";
    oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
    oss << "\"error\":" << (resp.error == hardware::MountError::None ? "null" : "\"1\"") << ",";
    oss << "\"message\":\"" << escape_json(resp.message) << "\",";
    oss << "\"supported\":" << (resp.supported ? "true" : "false") << ",";
    // backend connection and raw
    oss << "\"raw\":\"" << escape_json(resp.raw_response) << "\"";
    oss << "}";
    return oss.str();
}

std::string mount_coordinates_json(TelescopeCore& core, bool include_raw) {
    hardware::MountCoordinatesRequest req;
    req.include_raw = include_raw;
    const auto resp = core.mountCoordinates(req);
    std::ostringstream oss;
    oss << "{";
    oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
    oss << "\"error\":" << (resp.error == hardware::MountError::None ? "null" : "\"1\"") << ",";
    oss << "\"message\":\"" << escape_json(resp.message) << "\",";
    oss << "\"supported\":" << (resp.supported ? "true" : "false");
    if (resp.ok && resp.coordinates.valid) {
        oss << ",\"ra_hours\":" << ra_hours_from(resp.coordinates.ra);
        oss << ",\"dec_degrees\":" << dec_degrees_from(resp.coordinates.dec);
    }
    if (!resp.raw_response.empty()) {
        oss << ",\"raw\":\"" << escape_json(resp.raw_response) << "\"";
    }
    oss << "}";
    return oss.str();
}

std::string mount_mode_json(TelescopeCore& core) {
    const auto resp = core.mountMode(hardware::MountModeRequest{});
    std::ostringstream oss;
    oss << "{";
    oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
    oss << "\"error\":" << (resp.error == hardware::MountError::None ? "null" : "\"1\"") << ",";
    oss << "\"message\":\"" << escape_json(resp.message) << "\",";
    oss << "\"supported\":" << (resp.supported ? "true" : "false") << ",";
    oss << "\"mode\":\"" << (resp.mode == hardware::MountMode::AltAz ? "ALTAZ" : "EQUATORIAL") << "\"";
    oss << "}";
    return make_success(oss.str());
}

std::string mount_config_json() {
    const auto cfg = hardware::MountConfig::fromEnvironment();
    std::ostringstream oss;
    oss << "{";
    oss << "\"host\":\"" << escape_json(cfg.host) << "\",";
    oss << "\"port\":" << cfg.port << ",";
    oss << "\"timeout_seconds\":" << cfg.timeout_seconds;
    oss << "}";
    return make_success(oss.str());
}

std::string mount_focuser_json(TelescopeCore& core) {
    const auto resp = core.mountFocuser(hardware::MountFocuserRequest{});
    std::ostringstream oss;
    oss << "{";
    oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
    oss << "\"error\":" << (resp.error == hardware::MountError::None ? "null" : "\"1\"") << ",";
    oss << "\"message\":\"" << escape_json(resp.message) << "\",";
    oss << "\"supported\":" << (resp.supported ? "true" : "false");
    if (resp.focuser.valid) {
        oss << ",\"position\":" << resp.focuser.position;
        oss << ",\"moving\":" << (resp.focuser.moving ? "true" : "false");
        oss << ",\"busy\":" << (resp.focuser.busy ? "true" : "false");
        oss << ",\"goto_rate\":" << resp.focuser.goto_rate;
    }
    if (!resp.raw_response.empty()) {
        oss << ",\"raw\":\"" << escape_json(resp.raw_response) << "\"";
    }
    oss << "}";
    return make_success(oss.str());
}

std::string mount_flipmirror_json(TelescopeCore& core) {
    const auto resp = core.mountFlipMirror(hardware::MountFlipMirrorRequest{});
    std::ostringstream oss;
    oss << "{";
    oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
    oss << "\"error\":" << (resp.error == hardware::MountError::None ? "null" : "\"1\"") << ",";
    oss << "\"message\":\"" << escape_json(resp.message) << "\",";
    oss << "\"supported\":" << (resp.supported ? "true" : "false");
    if (resp.flip_mirror.valid) {
        oss << ",\"position\":\"" << (resp.flip_mirror.position == hardware::FlipMirrorPosition::Camera ? "camera" : "eyepiece") << "\"";
    }
    if (!resp.raw_response.empty()) {
        oss << ",\"raw\":\"" << escape_json(resp.raw_response) << "\"";
    }
    oss << "}";
    return make_success(oss.str());
}

std::string mount_flipmirror_move_json(TelescopeCore& core, const std::string& body) {
    const std::string pos = web::extract_json_string(body, "position");
    if (pos.empty()) { return make_error("missing position"); }
    hardware::MountFlipMirrorRequest r;
    if (pos == "camera") r.position = hardware::FlipMirrorPosition::Camera;
    else if (pos == "eyepiece") r.position = hardware::FlipMirrorPosition::Eyepiece;
    else { return make_error("invalid position"); }
    const auto resp = core.mountFlipMirror(r);
    if (!resp.ok) { return make_error(resp.message); }
    return make_success(std::string("{}"));
}

std::string mount_rotator_json(TelescopeCore& core, bool include_raw) {
    hardware::MountRotatorRequest req;
    req.action = hardware::RotatorControlAction::Query;
    req.query_availability = true;
    req.query_driver_status = true;
    const auto resp = core.mountRotator(req);
    std::ostringstream oss;
    oss << "{";
    oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
    oss << "\"error\":" << (resp.error == hardware::MountError::None ? "null" : "\"1\"") << ",";
    oss << "\"message\":\"" << escape_json(resp.message) << "\",";
    oss << "\"supported\":" << (resp.supported ? "true" : "false");
    if (resp.rotator.valid) {
        if (resp.rotator.current_angle_deg.has_value()) {
            oss << ",\"angle_deg\":" << resp.rotator.current_angle_deg.value();
        }
        if (resp.rotator.driver_status.has_value()) {
            oss << ",\"driver_status\":" << resp.rotator.driver_status.value();
        }
        oss << ",\"availability\":\"" << (resp.rotator.availability == hardware::RotatorAvailability::Available ? "Available" : (resp.rotator.availability == hardware::RotatorAvailability::NotInstalled ? "NotInstalled" : "Unknown")) << "\"";
    }
    if (!resp.raw_response.empty() && include_raw) {
        oss << ",\"raw\":\"" << escape_json(resp.raw_response) << "\"";
    }
    oss << "}";
    return make_success(oss.str());
}

void register_mount_api(Server& svr, TelescopeCore& core) {
    // status
    svr.Get(R"(/api/mount/status)", [&](const Request& req, Response& res) {
        const bool raw = req.has_param("raw") && req.get_param_value("raw") == "1";
        res.set_content(mount_status_json(core, raw), "application/json");
    });

    // config (read-only; reflects environment / startup configuration)
    svr.Get(R"(/api/mount/config)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(mount_config_json(), "application/json");
    });

    // coordinates
    svr.Get(R"(/api/mount/coordinates)", [&](const Request& req, Response& res) {
        const bool raw = req.has_param("raw") && req.get_param_value("raw") == "1";
        res.set_content(mount_coordinates_json(core, raw), "application/json");
    });

    // mode
    svr.Get(R"(/api/mount/mode)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(mount_mode_json(core), "application/json");
    });

    // goto (use MotionService)
    svr.Post(R"(/api/mount/goto)", [&](const Request& req, Response& res) {
        const auto parsed = parse_goto_body(req.body);
        if (parsed.type == ParsedGoto::Type::Invalid) {
            res.status = 400;
            res.set_content(make_error(parsed.error), "application/json");
            return;
        }
        std::string log;
        if (parsed.type == ParsedGoto::Type::RADEC) {
            const auto result = core.motion().goto_target(parsed.ra_hours, parsed.dec_deg, log);
            res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(log) + "\"}"), "application/json");
            return;
        }
        // ALTAZ
        const auto mode_resp = core.mountMode(hardware::MountModeRequest{});
        if (!(mode_resp.ok && mode_resp.supported && mode_resp.mode == hardware::MountMode::AltAz)) {
            res.status = 400;
            res.set_content(make_error("mount not in ALTAZ mode"), "application/json");
            return;
        }
        // basic sensible range checks
        if (parsed.alt_deg < -90.0 || parsed.alt_deg > 90.0 || parsed.az_deg < -360.0 || parsed.az_deg > 360.0) {
            res.status = 400;
            res.set_content(make_error("invalid alt/az range"), "application/json");
            return;
        }
        const auto result = core.motion().goto_target(0.0, 0.0, true, parsed.alt_deg, parsed.az_deg, log);
        res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(log) + "\"}"), "application/json");
    });

    // sync
    svr.Post(R"(/api/mount/sync)", [&](const Request& req, Response& res) {
        const auto parsed = parse_goto_body(req.body);
        if (parsed.type == ParsedGoto::Type::Invalid) {
            res.status = 400;
            res.set_content(make_error(parsed.error), "application/json");
            return;
        }
        std::string log;
        if (parsed.type == ParsedGoto::Type::RADEC) {
            const auto result = core.motion().sync(parsed.ra_hours, parsed.dec_deg, log);
            res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(log) + "\"}"), "application/json");
            return;
        }
        const auto mode_resp = core.mountMode(hardware::MountModeRequest{});
        if (!(mode_resp.ok && mode_resp.supported && mode_resp.mode == hardware::MountMode::AltAz)) {
            res.status = 400;
            res.set_content(make_error("mount not in ALTAZ mode"), "application/json");
            return;
        }
        if (parsed.alt_deg < -90.0 || parsed.alt_deg > 90.0 || parsed.az_deg < -360.0 || parsed.az_deg > 360.0) {
            res.status = 400;
            res.set_content(make_error("invalid alt/az range"), "application/json");
            return;
        }
        const auto result = core.motion().sync(0.0, 0.0, true, parsed.alt_deg, parsed.az_deg, log);
        res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(log) + "\"}"), "application/json");
    });

    // tracking
    svr.Post(R"(/api/mount/tracking)", [&](const Request& req, Response& res) {
        const auto action = web::extract_json_string(req.body, "action");
        std::string log;
        if (action == "on") {
            const auto r = core.motion().start_tracking(log);
            res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(r.log) + "\"}"), "application/json");
            return;
        }
        if (action == "off") {
            const auto r = core.motion().stop_tracking(log);
            res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(r.log) + "\"}"), "application/json");
            return;
        }
        res.status = 400;
        res.set_content(make_error("invalid tracking action; use 'on' or 'off'"), "application/json");
    });

    // home
    svr.Post(R"(/api/mount/home)", [&](const Request& req, Response& res) {
        std::string log;
            const auto r = core.motion().home(log);
        res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(r.log) + "\"}"), "application/json");
    });

    // park
    svr.Post(R"(/api/mount/park)", [&](const Request& req, Response& res) {
        std::string log;
        const auto r = core.motion().park(log);
        res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(r.log) + "\"}"), "application/json");
    });

    // abort
    svr.Post(R"(/api/mount/abort)", [&](const Request& req, Response& res) {
        std::string log;
        const auto r = core.motion().abort(log);
        res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(r.log) + "\"}"), "application/json");
    });

    // manual (north/south/east/west/stop)
    svr.Post(R"(/api/mount/manual)", [&](const Request& req, Response& res) {
        const std::string dir = web::extract_json_string(req.body, "direction");
        if (dir.empty()) {
            res.status = 400;
            res.set_content(make_error("missing direction"), "application/json");
            return;
        }
        // MotionService::move returns a log string; manualMotion() is used internally.
        const auto log = core.motion().move(dir, 0);
        res.set_content(web::make_success(std::string("{\"message\":\"") + escape_json(log) + "\"}"), "application/json");
    });

    // Focuser endpoints
    svr.Get(R"(/api/focuser/status)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(mount_focuser_json(core), "application/json");
    });

    svr.Get(R"(/api/focuser/position)", [&](const Request& req, Response& res) {
        (void)req;
        const auto resp = core.mountFocuser(hardware::MountFocuserRequest{});
        std::ostringstream oss;
        oss << "{";
        oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
        oss << "\"message\":\"" << escape_json(resp.message) << "\",";
        if (resp.focuser.valid) {
            oss << "\"position\":" << resp.focuser.position << ",";
            oss << "\"moving\":" << (resp.focuser.moving ? "true" : "false");
        } else {
            oss << "\"position\":null";
        }
        oss << "}";
        res.set_content(make_success(oss.str()), "application/json");
    });

    svr.Post(R"(/api/focuser/in)", [&](const Request& req, Response& res) {
        hardware::MountFocuserRequest r; r.action = hardware::FocuserControlAction::MoveInward;
        const auto resp = core.mountFocuser(r);
        if (!resp.supported) { res.set_content(make_error("focuser unsupported"), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    svr.Post(R"(/api/focuser/out)", [&](const Request& req, Response& res) {
        hardware::MountFocuserRequest r; r.action = hardware::FocuserControlAction::MoveOutward;
        const auto resp = core.mountFocuser(r);
        if (!resp.supported) { res.set_content(make_error("focuser unsupported"), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    svr.Post(R"(/api/focuser/stop)", [&](const Request& req, Response& res) {
        hardware::MountFocuserRequest r; r.action = hardware::FocuserControlAction::Stop;
        const auto resp = core.mountFocuser(r);
        if (!resp.supported) { res.set_content(make_error("focuser unsupported"), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    svr.Post(R"(/api/focuser/move)", [&](const Request& req, Response& res) {
        // expecting JSON: {"position": 1000}
        const std::string body = req.body;
        // crude JSON extraction (consistent with other handlers)
        const std::string key = '"' + std::string("position") + '"';
        auto pos = body.find(key);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("missing position"), "application/json"); return; }
        pos = body.find(':', pos);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("invalid position"), "application/json"); return; }
        pos++;
        while (pos < body.size() && isspace((unsigned char)body[pos])) pos++;
        size_t end = pos;
        while (end < body.size() && (body[end]=='-'||body[end]=='+'|| (body[end] >= '0' && body[end] <= '9'))) end++;
        if (end == pos) { res.status = 400; res.set_content(make_error("invalid position"), "application/json"); return; }
        const std::string token = body.substr(pos, end-pos);
        long long target = 0;
        try { target = std::stoll(token); } catch(...) { res.status = 400; res.set_content(make_error("invalid position"), "application/json"); return; }
        if (target < LLONG_MIN || target > LLONG_MAX) { res.status = 400; res.set_content(make_error("position out of range"), "application/json"); return; }
        hardware::MountFocuserRequest r; r.action = hardware::FocuserControlAction::GotoPosition; r.target_position = (int)target;
        const auto resp = core.mountFocuser(r);
        if (!resp.supported) { res.set_content(make_error("focuser unsupported"), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    // Flip mirror endpoints
    svr.Get(R"(/api/flipmirror/status)", [&](const Request& req, Response& res) {
        (void)req;
        res.set_content(mount_flipmirror_json(core), "application/json");
    });

    svr.Post(R"(/api/flipmirror/move)", [&](const Request& req, Response& res) {
        const std::string body = req.body;
        const std::string pos = web::extract_json_string(body, "position");
        if (pos.empty()) { res.status = 400; res.set_content(make_error("missing position"), "application/json"); return; }
        hardware::MountFlipMirrorRequest r;
        if (pos == "camera") r.position = hardware::FlipMirrorPosition::Camera;
        else if (pos == "eyepiece") r.position = hardware::FlipMirrorPosition::Eyepiece;
        else { res.status = 400; res.set_content(make_error("invalid position"), "application/json"); return; }
        const auto resp = core.mountFlipMirror(r);
        if (!resp.ok) { res.set_content(make_error(resp.message), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    // Rotator endpoints
    svr.Get(R"(/api/rotator/status)", [&](const Request& req, Response& res) {
        const bool raw = req.has_param("raw") && req.get_param_value("raw") == "1";
        res.set_content(mount_rotator_json(core, raw), "application/json");
    });

    svr.Get(R"(/api/rotator/position)", [&](const Request& req, Response& res) {
        (void)req;
        hardware::MountRotatorRequest r;
        r.action = hardware::RotatorControlAction::GetAngle;
        const auto resp = core.mountRotator(r);
        std::ostringstream oss;
        oss << "{";
        oss << "\"ok\":" << (resp.ok ? "true" : "false") << ",";
        oss << "\"message\":\"" << escape_json(resp.message) << "\"";
        if (resp.rotator.current_angle_deg.has_value()) {
            oss << ",\"angle_deg\":" << resp.rotator.current_angle_deg.value();
        } else {
            oss << ",\"angle_deg\":null";
        }
        oss << "}";
        res.set_content(make_success(oss.str()), "application/json");
    });

    svr.Post(R"(/api/rotator/goto)", [&](const Request& req, Response& res) {
        const std::string body = req.body;
        // Expect JSON: {"angle": 123.0}
        const std::string key = '"' + std::string("angle") + '"';
        auto pos = body.find(key);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("missing angle"), "application/json"); return; }
        pos = body.find(':', pos);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("invalid angle"), "application/json"); return; }
        pos++;
        while (pos < body.size() && isspace((unsigned char)body[pos])) pos++;
        size_t end = pos;
        while (end < body.size() && (body[end]=='-'||body[end]=='+'|| (body[end] >= '0' && body[end] <= '9') || body[end]=='.' )) end++;
        if (end == pos) { res.status = 400; res.set_content(make_error("invalid angle"), "application/json"); return; }
        const std::string token = body.substr(pos, end-pos);
        double angle = 0.0;
        try { angle = std::stod(token); } catch(...) { res.status = 400; res.set_content(make_error("invalid angle"), "application/json"); return; }
        hardware::MountRotatorRequest r; r.action = hardware::RotatorControlAction::AbsoluteGoto; r.target_angle_deg = angle;
        const auto resp = core.mountRotator(r);
        if (!resp.ok) { res.set_content(make_error(resp.message), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    svr.Post(R"(/api/rotator/relative)", [&](const Request& req, Response& res) {
        const std::string body = req.body;
        const std::string key = '"' + std::string("delta") + '"';
        auto pos = body.find(key);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("missing delta"), "application/json"); return; }
        pos = body.find(':', pos);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("invalid delta"), "application/json"); return; }
        pos++;
        while (pos < body.size() && isspace((unsigned char)body[pos])) pos++;
        size_t end = pos;
        while (end < body.size() && (body[end]=='-'||body[end]=='+'|| (body[end] >= '0' && body[end] <= '9') || body[end]=='.' )) end++;
        if (end == pos) { res.status = 400; res.set_content(make_error("invalid delta"), "application/json"); return; }
        const std::string token = body.substr(pos, end-pos);
        double delta = 0.0;
        try { delta = std::stod(token); } catch(...) { res.status = 400; res.set_content(make_error("invalid delta"), "application/json"); return; }
        hardware::MountRotatorRequest r; r.action = hardware::RotatorControlAction::Relative; r.relative_angle_deg = delta;
        const auto resp = core.mountRotator(r);
        if (!resp.ok) { res.set_content(make_error(resp.message), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });

    auto simple_post = [&](const std::string& path, hardware::RotatorControlAction action) {
        svr.Post(path, [&](const Request& req, Response& res) {
            (void)req;
            const auto resp = core.mountRotator(hardware::MountRotatorRequest{action, false, false});
            if (!resp.ok) { res.set_content(make_error(resp.message), "application/json"); return; }
            res.set_content(make_success(std::string("{}")), "application/json");
        });
    };

    // simple actions
    simple_post(R"(/api/rotator/cw)", hardware::RotatorControlAction::ContinuousCW);
    simple_post(R"(/api/rotator/ccw)", hardware::RotatorControlAction::ContinuousCCW);
    simple_post(R"(/api/rotator/stop)", hardware::RotatorControlAction::Stop);
    simple_post(R"(/api/rotator/zero)", hardware::RotatorControlAction::SetZero);
    simple_post(R"(/api/rotator/half-travel)", hardware::RotatorControlAction::SetHalfTravel);
    simple_post(R"(/api/rotator/home)", hardware::RotatorControlAction::MoveHalfTravelOrHome);
    simple_post(R"(/api/rotator/derotate)", hardware::RotatorControlAction::DerotateEnable);
    simple_post(R"(/api/rotator/reverse)", hardware::RotatorControlAction::DerotateReverse);
    simple_post(R"(/api/rotator/park)", hardware::RotatorControlAction::Park);
    simple_post(R"(/api/rotator/unpark)", hardware::RotatorControlAction::Unpark);

    // rate endpoint expects {"rate": N}
    svr.Post(R"(/api/rotator/rate)", [&](const Request& req, Response& res) {
        const std::string body = req.body;
        const std::string key = '"' + std::string("rate") + '"';
        auto pos = body.find(key);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("missing rate"), "application/json"); return; }
        pos = body.find(':', pos);
        if (pos == std::string::npos) { res.status = 400; res.set_content(make_error("invalid rate"), "application/json"); return; }
        pos++;
        while (pos < body.size() && isspace((unsigned char)body[pos])) pos++;
        size_t end = pos;
        while (end < body.size() && (body[end]=='-'||body[end]=='+'|| (body[end] >= '0' && body[end] <= '9'))) end++;
        if (end == pos) { res.status = 400; res.set_content(make_error("invalid rate"), "application/json"); return; }
        const std::string token = body.substr(pos, end-pos);
        int rate = 0;
        try { rate = std::stoi(token); } catch(...) { res.status = 400; res.set_content(make_error("invalid rate"), "application/json"); return; }
        if (rate < 1 || rate > 9) { res.status = 400; res.set_content(make_error("rate out of range"), "application/json"); return; }
        hardware::MountRotatorRequest r; r.action = hardware::RotatorControlAction::SetRate; r.rate_index = rate;
        const auto resp = core.mountRotator(r);
        if (!resp.ok) { res.set_content(make_error(resp.message), "application/json"); return; }
        res.set_content(make_success(std::string("{}")), "application/json");
    });
}

} // namespace web
} // namespace asdevlab
