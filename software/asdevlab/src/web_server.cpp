// Lightweight REST API server using bundled cpp-httplib (httplib_for_ols)
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "asdevlab/telescope_core.hpp"
#include "asdevlab/web/catalog_api.hpp"
#include "asdevlab/web/target_api.hpp"
#include "asdevlab/web/mount_api.hpp"
#include "httplib_for_ols.h"

using namespace httplib_for_ols;

static std::string json_response(bool ok, const std::string& log) {
  std::ostringstream oss;
  oss << "{\"ok\":" << (ok ? "true" : "false") << ",\"log\":\"";
  for (char c : log) {
    if (c == '"') oss << "\\\"";
    else if (c == '\n') oss << "\\n";
    else oss << c;
  }
  oss << "\"}";
  return oss.str();
}

int main() {
  asdevlab::TelescopeCore core;

  Server svr;

  // register new API endpoints
  asdevlab::web::register_catalog_api(svr, core);
  asdevlab::web::register_target_api(svr, core);
  asdevlab::web::register_mount_api(svr, core);

  // Keep a simple root page for convenience
  svr.Get("/", [&](const Request& req, Response& res) {
    std::ostringstream oss;
    oss << "<html><body><h1>ASDEVLAB REST API</h1><p>Use /api/mount/* endpoints.</p><p>Open <a href=\"/mount-test\">Mount Hardware Test</a></p></body></html>";
    res.set_content(oss.str(), "text/html");
  });

  // serve mount-test page (static file from source tree)
  svr.Get("/mount-test", [&](const Request& req, Response& res) {
    (void)req;
    std::string path = "../www/mount_test.html"; // relative to build directory
    std::ifstream in(path);
    if (!in.good()) {
      res.status = 404;
      res.set_content("Mount test page not found.", "text/plain");
      return;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    res.set_content(ss.str(), "text/html");
  });

  std::cout << "ASDEVLAB REST server started on http://localhost:8080/" << std::endl;
  svr.listen("0.0.0.0", 8080);

  return 0;
}
