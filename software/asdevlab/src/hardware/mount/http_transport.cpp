#include "asdevlab/hardware/mount/http_transport.hpp"

#include <chrono>
#include <curl/curl.h>
#include <sstream>

namespace asdevlab {
namespace hardware {

namespace {
size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    const size_t real_size = size * nmemb;
    std::string* out = static_cast<std::string*>(userdata);
    out->append(ptr, real_size);
    return real_size;
}

std::string escapeUrl(CURL* curl, const std::string& value) {
    char* escaped = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.size()));
    if (!escaped) {
        return std::string();
    }
    std::string result(escaped);
    curl_free(escaped);
    return result;
}

std::string normalizeResource(const std::string& resource) {
    if (resource.empty() || resource[0] != '/') {
        return resource;
    }
    return resource.substr(1);
}

std::string buildUrl(CURL* curl,
                     const std::string& base_url,
                     const std::string& resource,
                     const std::vector<std::pair<std::string, std::string>>& query_params) {
    std::string url = base_url;

    const std::string resource_clean = normalizeResource(resource);
    if (!resource_clean.empty()) {
        if (!url.empty() && url.back() != '/') {
            url.push_back('/');
        }
        url += resource_clean;
    }

    bool first_query = (url.find('?') == std::string::npos);
    auto append_param = [&](const std::string& key, const std::string& value) {
        url.push_back(first_query ? '?' : '&');
        first_query = false;
        url += escapeUrl(curl, key);
        url.push_back('=');
        url += escapeUrl(curl, value);
    };

    for (const auto& param : query_params) {
        append_param(param.first, param.second);
    }

    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    append_param("x", std::to_string(now_ms));

    return url;
}

bool executeRequest(CURL* curl, const std::string& url, std::string& response_out, long timeout_seconds) {
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_out);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        return false;
    }

    long http_code = 0;
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK) {
        return false;
    }

    return http_code >= 200 && http_code < 300;
}
} // namespace

HttpTransport::HttpTransport(std::string base_url, long timeout_seconds)
    : base_url_(std::move(base_url))
    , timeout_seconds_(timeout_seconds) {}

bool HttpTransport::send(const std::string& command, std::string& response_out) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    const std::string url = buildUrl(curl, base_url_, std::string(), {{"cmd", command}});
    const bool ok = executeRequest(curl, url, response_out, timeout_seconds_);
    curl_easy_cleanup(curl);
    return ok;
}

bool HttpTransport::sendGet(const std::string& resource,
                            const std::vector<std::pair<std::string, std::string>>& query_params,
                            std::string& response_out) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    const std::string url = buildUrl(curl, base_url_, resource, query_params);
    const bool ok = executeRequest(curl, url, response_out, timeout_seconds_);
    curl_easy_cleanup(curl);
    return ok;
}

} // namespace hardware
} // namespace asdevlab
