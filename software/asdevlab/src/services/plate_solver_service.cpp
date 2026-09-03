#include "asdevlab/services/plate_solver_service.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

namespace asdevlab {
namespace {

struct FrameStats {
    std::string id;
    double star_count = 0;
    double background = 0;
    double fwhm_x = 0;
    double fwhm_y = 0;
    double centroid0_x = 0;
    double centroid0_y = 0;
    double centroid1_x = 0;
    double centroid1_y = 0;
};

struct WarningInfo {
    std::string type;
    std::string message;
};

std::optional<std::filesystem::path> sim_root_path() {
    const char* env_root = std::getenv("ASDEVLAB_SIM_ROOT");
    if (env_root && *env_root) {
        return std::filesystem::path(env_root);
    }
    const char* env_base = std::getenv("ASDEVLAB_ROOT");
    if (env_base && *env_base) {
        return std::filesystem::path(env_base) / "sim";
    }
    return std::nullopt;
}

std::optional<double> parse_json_number(const std::string& json, const std::string& key) {
    const std::string quoted = std::string("\"") + key + "\"";
    auto pos = json.find(quoted);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos++;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        pos++;
    }
    auto end = pos;
    while (end < json.size() && (std::isdigit(static_cast<unsigned char>(json[end])) || json[end] == '.' || json[end] == '+' || json[end] == '-' || json[end] == 'e' || json[end] == 'E')) {
        end++;
    }
    if (end == pos) {
        return std::nullopt;
    }
    return std::stod(json.substr(pos, end - pos));
}

std::optional<std::string> parse_json_string(const std::string& json, const std::string& key) {
    const std::string quoted = std::string("\"") + key + "\"";
    auto pos = json.find(quoted);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos++;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        pos++;
    }
    if (pos >= json.size() || json[pos] != '"') {
        return std::nullopt;
    }
    pos++;
    const auto end = json.find('"', pos);
    if (end == std::string::npos) {
        return std::nullopt;
    }
    return json.substr(pos, end - pos);
}

std::optional<std::string> extract_json_object(const std::string& json, size_t start) {
    auto brace = json.find('{', start);
    if (brace == std::string::npos) {
        return std::nullopt;
    }
    int depth = 0;
    for (size_t i = brace; i < json.size(); ++i) {
        if (json[i] == '{') {
            depth++;
        } else if (json[i] == '}') {
            depth--;
            if (depth == 0) {
                return json.substr(brace, i - brace + 1);
            }
        }
    }
    return std::nullopt;
}

std::optional<FrameStats> parse_frame_stats_from_block(const std::string& block) {
    FrameStats stats;
    auto id = parse_json_string(block, "id");
    if (!id) {
        return std::nullopt;
    }
    stats.id = *id;
    if (auto value = parse_json_number(block, "star_count"); value) {
        stats.star_count = *value;
    }
    if (auto value = parse_json_number(block, "background"); value) {
        stats.background = *value;
    }
    if (auto value = parse_json_number(block, "fwhm_x"); value) {
        stats.fwhm_x = *value;
    }
    if (auto value = parse_json_number(block, "fwhm_y"); value) {
        stats.fwhm_y = *value;
    }
    if (auto value = parse_json_number(block, "centroid0_x"); value) {
        stats.centroid0_x = *value;
    }
    if (auto value = parse_json_number(block, "centroid0_y"); value) {
        stats.centroid0_y = *value;
    }
    if (auto value = parse_json_number(block, "centroid1_x"); value) {
        stats.centroid1_x = *value;
    }
    if (auto value = parse_json_number(block, "centroid1_y"); value) {
        stats.centroid1_y = *value;
    }
    return stats;
}

std::vector<FrameStats> read_all_frame_stats(const std::filesystem::path& root) {
    const auto path = root / "frame_stats.json";
    std::ifstream input(path);
    if (!input.is_open()) {
        return {};
    }
    const std::string json((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    std::vector<FrameStats> frames;
    size_t pos = 0;
    while (true) {
        const auto found = json.find("\"id\"", pos);
        if (found == std::string::npos) {
            break;
        }
        if (auto block = extract_json_object(json, found); block) {
            if (auto frame = parse_frame_stats_from_block(*block); frame) {
                frames.push_back(std::move(*frame));
            }
            pos = found + 1;
            continue;
        }
        break;
    }
    std::sort(frames.begin(), frames.end(), [](const FrameStats& lhs, const FrameStats& rhs) {
        const auto parse_index = [](const std::string& id) -> uint64_t {
            const std::string prefix = "frame_";
            if (id.rfind(prefix, 0) != 0) {
                return std::numeric_limits<uint64_t>::max();
            }
            try {
                return static_cast<uint64_t>(std::stoull(id.substr(prefix.size())));
            } catch (...) {
                return std::numeric_limits<uint64_t>::max();
            }
        };
        return parse_index(lhs.id) < parse_index(rhs.id);
    });
    return frames;
}

std::optional<FrameStats> compute_rolling_baseline(const std::vector<FrameStats>& frames, size_t current_index) {
    if (current_index == 0 || current_index > frames.size()) {
        return std::nullopt;
    }

    FrameStats baseline;
    const size_t count = current_index;
    for (size_t index = 0; index < current_index; ++index) {
        baseline.star_count += frames[index].star_count;
        baseline.background += frames[index].background;
        baseline.fwhm_x += frames[index].fwhm_x;
        baseline.fwhm_y += frames[index].fwhm_y;
        baseline.centroid0_x += frames[index].centroid0_x;
        baseline.centroid0_y += frames[index].centroid0_y;
        baseline.centroid1_x += frames[index].centroid1_x;
        baseline.centroid1_y += frames[index].centroid1_y;
    }
    baseline.star_count /= static_cast<double>(count);
    baseline.background /= static_cast<double>(count);
    baseline.fwhm_x /= static_cast<double>(count);
    baseline.fwhm_y /= static_cast<double>(count);
    baseline.centroid0_x /= static_cast<double>(count);
    baseline.centroid0_y /= static_cast<double>(count);
    baseline.centroid1_x /= static_cast<double>(count);
    baseline.centroid1_y /= static_cast<double>(count);
    baseline.id = frames[current_index].id;
    return baseline;
}

std::optional<WarningInfo> make_warning_info(const FrameStats& baseline, const FrameStats& current) {
    const double star_drop = (baseline.star_count > 0) ? ((baseline.star_count - current.star_count) / baseline.star_count) : 0.0;
    const double background_change = current.background - baseline.background;
    const double fwhm_ratio = (current.fwhm_y > 0) ? (current.fwhm_x / current.fwhm_y) : 1.0;
    const double centroid_dx = current.centroid0_x - baseline.centroid0_x;
    const double centroid_dy = current.centroid0_y - baseline.centroid0_y;
    const double centroid_shift = std::sqrt(centroid_dx * centroid_dx + centroid_dy * centroid_dy);

    bool has_wind_shake = (fwhm_ratio > 1.4 || fwhm_ratio < 0.7 || centroid_shift > 5.0);
    bool has_cloud = ((star_drop > 0.4 && current.star_count < baseline.star_count) || background_change > 100);
    if (!has_wind_shake && !has_cloud) {
        return std::nullopt;
    }

    WarningInfo warning;
    if (has_wind_shake) {
        warning.type = "wind_shake";
        warning.message = "Wind/Shake warning: unstable stars detected (FWHM anisotropy or centroid jump).";
    }
    if (has_cloud) {
        if (!warning.type.empty()) {
            warning.type += ";cloud";
            warning.message += " ";
        } else {
            warning.type = "cloud";
        }
        warning.message += "Cloud warning: star count dropped and background changed significantly.";
    }
    return warning;
}

std::optional<WarningInfo> detect_frame_warnings(const std::filesystem::path& root, const std::string& image_id) {
    const auto frames = read_all_frame_stats(root);
    if (frames.empty()) {
        return std::nullopt;
    }
    const auto it = std::find_if(frames.begin(), frames.end(), [&](const FrameStats& frame) {
        return frame.id == image_id;
    });
    if (it == frames.end()) {
        return std::nullopt;
    }
    const size_t index = std::distance(frames.begin(), it);
    const auto baseline = compute_rolling_baseline(frames, index);
    if (!baseline) {
        return std::nullopt;
    }
    return make_warning_info(*baseline, *it);
}

std::optional<std::pair<double, double>> read_target_coordinates(const std::filesystem::path& root) {
    const auto path = root / "target.json";
    std::ifstream input(path);
    if (!input.is_open()) {
        return std::nullopt;
    }
    const std::string json((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto ra = parse_json_number(json, "ra");
    const auto dec = parse_json_number(json, "dec");
    if (!ra || !dec) {
        return std::nullopt;
    }
    return std::make_pair(*ra, *dec);
}

std::optional<std::pair<double, double>> read_mount_coordinates(const std::filesystem::path& root) {
    const auto path = root / "mount_state.json";
    std::ifstream input(path);
    if (!input.is_open()) {
        return std::nullopt;
    }
    const std::string json((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto ra = parse_json_number(json, "ra");
    const auto dec = parse_json_number(json, "dec");
    if (!ra || !dec) {
        return std::nullopt;
    }
    return std::make_pair(*ra, *dec);
}

} // namespace

SolveResult PlateSolverService::solve_result(const std::string& image_id) {
    SolveResult result;
    if (const auto sim_root = sim_root_path(); sim_root) {
        if (auto warning = detect_frame_warnings(*sim_root, image_id); warning) {
            result.warning_type = warning->type;
            result.warning_message = warning->message;
        }
        const auto target = read_target_coordinates(*sim_root);
        const auto mount = read_mount_coordinates(*sim_root);
        if (target && mount) {
            result.target_ra = target->first;
            result.target_dec = target->second;
            result.mount_ra = mount->first;
            result.mount_dec = mount->second;
            const double dx = target->first - mount->first;
            const double dy = target->second - mount->second;
            result.offset = std::sqrt(dx * dx + dy * dy);
            result.solved = true;
        }
    }
    return result;
}

std::string PlateSolverService::solve(const std::string& image_id) {
    const auto result = solve_result(image_id);
    std::ostringstream oss;
    if (result.solved) {
        oss << "PlateSolverService: solved image_id=" << image_id
            << " target_ra=" << *result.target_ra
            << " target_dec=" << *result.target_dec
            << " mount_ra=" << *result.mount_ra
            << " mount_dec=" << *result.mount_dec
            << " offset=" << *result.offset;
    } else {
        oss << "PlateSolverService: solve " << image_id;
    }
    if (result.warning_message) {
        if (result.warning_type) {
            oss << " warning_type=" << *result.warning_type;
        }
        oss << " warning='" << *result.warning_message << "'";
    }
    return oss.str();
}

std::string PlateSolverService::calibrate(const std::string& image_id) {
    std::ostringstream oss;
    oss << "PlateSolverService: calibrate " << image_id;
    return oss.str();
}

} // namespace asdevlab
