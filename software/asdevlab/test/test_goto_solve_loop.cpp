#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static std::string file_to_string(const std::filesystem::path& path) {
    std::ifstream input(path);
    assert(input.is_open());
    std::ostringstream ss;
    ss << input.rdbuf();
    return ss.str();
}

static std::filesystem::path project_root() {
    std::filesystem::path source_file = __FILE__;
    if (source_file.is_relative()) {
        source_file = std::filesystem::current_path() / source_file;
    }
    return source_file.parent_path().parent_path();
}

static std::string extract_json_string(const std::string& json, const std::string& key) {
    const std::string quoted = '"' + key + '"';
    auto pos = json.find(quoted);
    assert(pos != std::string::npos);
    pos = json.find(':', pos);
    assert(pos != std::string::npos);
    pos++;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        pos++;
    }
    assert(json[pos] == '"');
    pos++;
    auto end = json.find('"', pos);
    assert(end != std::string::npos);
    return json.substr(pos, end - pos);
}

static double extract_json_number(const std::string& json, const std::string& key) {
    const std::string quoted = '"' + key + '"';
    auto pos = json.find(quoted);
    assert(pos != std::string::npos);
    pos = json.find(':', pos);
    assert(pos != std::string::npos);
    pos++;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        pos++;
    }
    auto end = pos;
    while (end < json.size() && (std::isdigit(static_cast<unsigned char>(json[end])) || json[end] == '.' || json[end] == '+' || json[end] == '-' || json[end] == 'e' || json[end] == 'E')) {
        end++;
    }
    assert(end > pos);
    return std::stod(json.substr(pos, end - pos));
}

int main() {
    const auto root = project_root();
    setenv("ASDEVLAB_ROOT", root.c_str(), 1);

    const auto sim_root = root / "sim";
    const auto frames_dir = sim_root / "frames";
    const auto frame_file = frames_dir / "frame_0000.jpeg";
    assert(std::filesystem::exists(frame_file));

    const auto target_json = file_to_string(sim_root / "target.json");
    const auto mount_json = file_to_string(sim_root / "mount_state.json");

    const auto target_ra = extract_json_number(target_json, "ra");
    const auto target_dec = extract_json_number(target_json, "dec");

    const auto mount_ra = extract_json_number(mount_json, "ra");
    const auto mount_dec = extract_json_number(mount_json, "dec");

    auto core = asdevlab::TelescopeCore();
    const auto motion_target = core.motion().goto_target("M31");
    assert(motion_target.find("MotionService") != std::string::npos);

    const auto solve_output = core.plate_solver().solve("frame_0000");
    assert(solve_output.find("PlateSolverService: solved") != std::string::npos);
    assert(solve_output.find("target_ra=") != std::string::npos);
    assert(solve_output.find("mount_ra=") != std::string::npos);
    assert(solve_output.find("offset=") != std::string::npos);

    const auto offset_pos = solve_output.find("offset=");
    assert(offset_pos != std::string::npos);
    const auto offset_value = std::stod(solve_output.substr(offset_pos + 7));
    assert(offset_value < 0.5);

    const auto solve_output_no_warning = core.plate_solver().solve("frame_0001");
    assert(solve_output_no_warning.find("warning='") == std::string::npos);

    const auto solve_output_warning = core.plate_solver().solve("frame_0002");
    assert(solve_output_warning.find("warning='") != std::string::npos);
    assert(solve_output_warning.find("Cloud warning") != std::string::npos);
    assert(solve_output_warning.find("Wind/Shake warning") != std::string::npos);

    const auto retry_result = core.scheduler().gotoAndSolve("M31", 5);
    assert(!retry_result.solved);
    assert(retry_result.attempts == 5);
    assert(retry_result.message.find("PlateSolverService: solved") != std::string::npos);

    std::cout << "asdevlab goto-solve integration test passed\n";
    return 0;
}
