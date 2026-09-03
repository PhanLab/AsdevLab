#pragma once

#include "httplib_for_ols.h"
#include "asdevlab/telescope_core.hpp"

#include <string>

namespace asdevlab {
namespace web {

void register_target_api(httplib_for_ols::Server& svr, TelescopeCore& core);

std::string target_goto_json(TelescopeCore& core, const std::string& id);
std::string target_sync_json(TelescopeCore& core, const std::string& id);
std::string target_preview_json(TelescopeCore& core, const std::string& id, const catalog::ObservationContext& ctx);
std::string target_current_json(TelescopeCore& core);

} // namespace web
} // namespace asdevlab
