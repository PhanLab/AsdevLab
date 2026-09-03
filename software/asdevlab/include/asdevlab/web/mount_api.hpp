#pragma once

#include "httplib_for_ols.h"
#include "asdevlab/telescope_core.hpp"

namespace asdevlab {
namespace web {

// JSON helpers (used by tests) and registration
std::string mount_status_json(TelescopeCore& core, bool include_raw = false);
std::string mount_coordinates_json(TelescopeCore& core, bool include_raw = false);
std::string mount_mode_json(TelescopeCore& core);
std::string mount_focuser_json(TelescopeCore& core);
std::string mount_rotator_json(TelescopeCore& core, bool include_raw = false);
std::string mount_flipmirror_json(TelescopeCore& core);
std::string mount_flipmirror_move_json(TelescopeCore& core, const std::string& body);

void register_mount_api(httplib_for_ols::Server& svr, TelescopeCore& core);

struct ParsedGoto {
	enum class Type { None, RADEC, ALTAZ, Invalid };
	Type type = Type::None;
	double ra_hours = 0.0;
	double dec_deg = 0.0;
	double alt_deg = 0.0;
	double az_deg = 0.0;
	std::string error;
};

// Parse the JSON body for a goto/sync request. Does not query mount mode.
ParsedGoto parse_goto_body(const std::string& body);

} // namespace web
} // namespace asdevlab
