#include "asdevlab/web/mount_api.hpp"
#include "asdevlab/telescope_core.hpp"
#include "asdevlab/hardware/mount/onstep_mount_client.hpp"
#include "asdevlab/hardware/mount/transport_interface.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

namespace asdevlab {
namespace hardware {

class MockTransportSimple : public TransportInterface {
public:
    std::vector<std::string> commands;
    std::string flip_mirror_state = "1";
    bool send(const std::string& command, std::string& response_out) override {
        commands.push_back(command);
        if (command == ":FM0#") { flip_mirror_state = "0"; response_out = "OK"; }
        else if (command == ":FM1#") { flip_mirror_state = "1"; response_out = "OK"; }
        else if (command == ":FM?#") { response_out = flip_mirror_state; }
        else response_out = "OK";
        return true;
    }
    bool sendGet(const std::string&, const std::vector<std::pair<std::string,std::string>>&, std::string& out) override { out = "OK"; return true; }
};

} // namespace hardware
} // namespace asdevlab

int main() {
    using namespace asdevlab;
    using namespace asdevlab::hardware;

    // Parser checks
    auto camera = LX200Parser::parseFlipMirrorState("1");
    assert(camera.has_value());
    assert(camera->position == FlipMirrorPosition::Camera);
    auto eyepiece = LX200Parser::parseFlipMirrorState("0");
    assert(eyepiece.has_value());
    assert(eyepiece->position == FlipMirrorPosition::Eyepiece);

    // OnStep client with mock transport: verify correct commands emitted
    auto mock = std::make_unique<MockTransportSimple>();
    MockTransportSimple* mp = mock.get();
    OnStepMountClient client(std::move(mock));
    bool ok = client.setFlipMirrorCamera();
    assert(ok);
    assert(!mp->commands.empty() && mp->commands.back() == ":FM1#");
    ok = client.setFlipMirrorEyepiece();
    assert(ok);
    assert(!mp->commands.empty() && mp->commands.back() == ":FM0#");
    const auto st = client.getFlipMirror(MountFlipMirrorRequest{});
    assert(st.ok);

    // REST helper smoke: ensure JSON helpers produce wrapper
    TelescopeCore core;
    const auto status_json = asdevlab::web::mount_flipmirror_json(core);
    assert(status_json.find("\"ok\":") != std::string::npos);
    const auto move_json = asdevlab::web::mount_flipmirror_move_json(core, std::string("{\"position\":\"camera\"}"));
    assert(move_json.find("\"ok\":") != std::string::npos);

    // invalid position should return error JSON
    const auto bad = asdevlab::web::mount_flipmirror_move_json(core, std::string("{\"position\":\"down\"}"));
    assert(bad.find("\"ok\":false") != std::string::npos);

    std::cout << "flip mirror rest tests passed\n";
    return 0;
}
