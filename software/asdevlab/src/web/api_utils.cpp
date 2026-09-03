#include "asdevlab/web/api_utils.hpp"

#include <algorithm>
#include <sstream>

namespace asdevlab {
namespace web {

std::string make_success(const std::string& data_json) {
    std::ostringstream oss;
    oss << "{\"ok\":true,";
    if (!data_json.empty()) {
        oss << "\"data\":" << data_json;
    } else {
        oss << "\"data\":null";
    }
    oss << "}";
    return oss.str();
}

std::string make_error(const std::string& error_message, int /*status*/) {
    std::ostringstream oss;
    oss << "{\"ok\":false,\"error\":\"";
    for (char c : error_message) {
        if (c == '"') oss << "\\\"";
        else if (c == '\n') oss << "\\n";
        else oss << c;
    }
    oss << "\"}";
    return oss.str();
}

std::string extract_json_string(const std::string& body, const std::string& key) {
    // find "key" : "value"
    const std::string keyq = '"' + key + '"';
    auto pos = body.find(keyq);
    if (pos == std::string::npos) return {};
    pos = body.find(':', pos);
    if (pos == std::string::npos) return {};
    // skip spaces
    pos++;
    while (pos < body.size() && isspace((unsigned char)body[pos])) pos++;
    if (pos >= body.size()) return {};
    if (body[pos] != '"') return {};
    pos++;
    std::ostringstream val;
    while (pos < body.size()) {
        char c = body[pos++];
        if (c == '"') break;
        if (c == '\\' && pos < body.size()) {
            char n = body[pos++];
            switch (n) {
                case 'n': val << '\n'; break;
                case '"': val << '"'; break;
                case '\\': val << '\\'; break;
                default: val << n; break;
            }
        } else {
            val << c;
        }
    }
    return val.str();
}

} // namespace web
} // namespace asdevlab
