#pragma once

#include "httplib_for_ols.h"
#include "asdevlab/telescope_core.hpp"

#include <string>

namespace asdevlab {
namespace web {

void register_catalog_api(httplib_for_ols::Server& svr, TelescopeCore& core);

std::string catalog_search_json(TelescopeCore& core, const std::string& keyword);
std::string catalog_object_json(TelescopeCore& core, const std::string& id);

} // namespace web
} // namespace asdevlab
