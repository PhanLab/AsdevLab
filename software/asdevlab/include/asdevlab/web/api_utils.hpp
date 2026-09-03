#pragma once

#include <string>

namespace asdevlab {
namespace web {

std::string make_success(const std::string& data_json);
std::string make_error(const std::string& error_message, int status = 400);

// simple JSON extractor for flat string fields (not a full JSON parser)
std::string extract_json_string(const std::string& body, const std::string& key);

} // namespace web
} // namespace asdevlab
