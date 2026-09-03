#include "asdevlab/telescope_core.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string getEnv(const char* name, const std::string& fallback = "") {
    if (const char* value = std::getenv(name)) {
        return std::string(value);
    }
    return fallback;
}

bool parseDouble(const std::string& text, double& value) {
    try {
        value = std::stod(text);
        return true;
    } catch (...) {
        return false;
    }
}

std::string formatRaHours(double ra_hours) {
    const int hours = static_cast<int>(std::floor(ra_hours));
    const double minutes_f = (ra_hours - hours) * 60.0;
    const int minutes = static_cast<int>(std::floor(minutes_f));
    const double seconds = (minutes_f - minutes) * 60.0;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ':'
        << std::setw(2) << std::setfill('0') << minutes << ':'
        << std::fixed << std::setprecision(2) << seconds;
    return oss.str();
}

std::string formatDecDegrees(double dec_degrees) {
    const bool negative = dec_degrees < 0.0;
    const double abs_dec = std::abs(dec_degrees);
    const int degrees = static_cast<int>(std::floor(abs_dec));
    const double minutes_f = (abs_dec - degrees) * 60.0;
    const int minutes = static_cast<int>(std::floor(minutes_f));
    const double seconds = (minutes_f - minutes) * 60.0;

    std::ostringstream oss;
    oss << (negative ? '-' : '+')
        << std::setw(2) << std::setfill('0') << degrees << '*'
        << std::setw(2) << std::setfill('0') << minutes << ':'
        << std::fixed << std::setprecision(2) << seconds;
    return oss.str();
}

std::string formatUtcTime(const std::chrono::system_clock::time_point& time_point) {
    const auto time_t_value = std::chrono::system_clock::to_time_t(time_point);
    std::tm tm = {};
#if defined(_WIN32)
    gmtime_s(&tm, &time_t_value);
#else
    gmtime_r(&time_t_value, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::string mountModeToString(asdevlab::hardware::MountMode mode) {
    switch (mode) {
        case asdevlab::hardware::MountMode::AltAz:
            return "ALTAZ";
        case asdevlab::hardware::MountMode::Equatorial:
            return "EQUATORIAL";
    }
    return "UNKNOWN";
}

std::string resolveCatalogPath() {
    const std::vector<std::string> candidates = {
        getEnv("ASDEVLAB_CATALOG_PATH"),
        "data/catalog",
        "../data/catalog",
        "software/asdevlab/data/catalog",
        "/home/phananh/Desktop/Project/AsdevLab/software/asdevlab/data/catalog",
    };

    for (const auto& candidate : candidates) {
        if (candidate.empty()) {
            continue;
        }

        const std::filesystem::path path(candidate);
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            return candidate;
        }
    }

    return candidates.back();
}

bool ensureLocation(asdevlab::catalog::ObservationContext& context) {
    const std::string lat_text = getEnv("ASDEVLAB_LATITUDE");
    const std::string lon_text = getEnv("ASDEVLAB_LONGITUDE");

    if (!lat_text.empty() && !lon_text.empty()) {
        double latitude = 0.0;
        double longitude = 0.0;
        if (parseDouble(lat_text, latitude) && parseDouble(lon_text, longitude)) {
            context.latitude_degrees = latitude;
            context.longitude_degrees = longitude;
            return true;
        }
    }

    if (context.latitude_degrees != 0.0 || context.longitude_degrees != 0.0) {
        return true;
    }

    std::cout << "Location unavailable. Enter latitude degrees: ";
    std::string lat_input;
    std::getline(std::cin, lat_input);
    std::cout << "Location unavailable. Enter longitude degrees: ";
    std::string lon_input;
    std::getline(std::cin, lon_input);

    double latitude = 0.0;
    double longitude = 0.0;
    if (!parseDouble(lat_input, latitude) || !parseDouble(lon_input, longitude)) {
        std::cout << "Invalid location input; using 0.0/0.0.\n";
        latitude = 0.0;
        longitude = 0.0;
    }

    context.latitude_degrees = latitude;
    context.longitude_degrees = longitude;
    return true;
}

std::optional<asdevlab::catalog::CatalogObject> findFirstObject(asdevlab::TelescopeCore& core, const std::string& query) {
    const auto search_result = core.target().search(query);
    if (search_result.results.empty()) {
        return std::nullopt;
    }

    return search_result.results.front();
}

std::optional<std::pair<double, double>> parseResolvedCoordinates(const std::string& preview_log) {
    const std::string marker = "ra=";
    const auto ra_pos = preview_log.find(marker);
    if (ra_pos == std::string::npos) {
        return std::nullopt;
    }

    const auto comma_pos = preview_log.find(',', ra_pos + marker.size());
    if (comma_pos == std::string::npos) {
        return std::nullopt;
    }

    const std::string dec_marker = " dec=";
    const auto dec_pos = preview_log.find(dec_marker, comma_pos);
    if (dec_pos == std::string::npos) {
        return std::nullopt;
    }

    try {
        const double ra = std::stod(preview_log.substr(ra_pos + marker.size(), comma_pos - ra_pos - marker.size()));
        const double dec = std::stod(preview_log.substr(dec_pos + dec_marker.size()));
        return std::make_pair(ra, dec);
    } catch (...) {
        return std::nullopt;
    }
}

void printUsage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " <command> [args]\n";
    std::cout << "Commands:\n";
    std::cout << "  status\n";
    std::cout << "  search <name>\n";
    std::cout << "  preview <name>\n";
    std::cout << "  goto <name>\n";
    std::cout << "  sync <name>\n";
    std::cout << "  dry-run <name>\n";
    std::cout << "  tracking on|off\n";
    std::cout << "  park\n";
    std::cout << "  home\n";
    std::cout << "  abort\n";
}

} // namespace

int main(int argc, char** argv) {
    const std::string program_name = argc > 0 ? argv[0] : "asdevlab_core_app";
    if (argc < 2) {
        asdevlab::TelescopeCore core;
        std::string log;
        const auto status = core.motion().status(log);
        std::cout << "Status: " << (status.ok ? status.log : log) << "\n";
        std::cout << "Mount mode: " << mountModeToString(core.motion().getMountMode()) << "\n";
        return 0;
    }

    asdevlab::TelescopeCore core;

    // The CLI is intentionally thin. All business logic stays inside the services.
    const std::string catalog_path = resolveCatalogPath();
    if (!core.catalog().loadCatalog(catalog_path)) {
        std::cerr << "Failed to load catalog from: " << catalog_path << "\n";
        return 1;
    }

    auto context = core.observation().makeObservationContext();
    ensureLocation(context);

    const std::string command = argv[1];

    if (command == "status") {
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";

        std::string log;
        const auto status_result = core.motion().status(log);
        std::cout << "Mount status: " << (status_result.ok ? status_result.log : log) << "\n";
        std::cout << "Mount mode: " << mountModeToString(core.motion().getMountMode()) << "\n";
        return status_result.ok ? 0 : 1;
    }

    if (command == "search") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }

        const auto objects = core.target().search(argv[2]).results;
        if (objects.empty()) {
            std::cout << "No objects found.\n";
            return 1;
        }

        for (const auto& object : objects) {
            std::cout << object.id << "\t" << object.name << "\t" << object.type << "\n";
        }
        return 0;
    }

    if (command == "preview") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }

        const auto object = findFirstObject(core, argv[2]);
        if (!object.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }

        const auto preview_result = core.target().previewObject(object->id, context);
        std::cout << preview_result.log << "\n";
        return preview_result.ok ? 0 : 1;
    }

    if (command == "goto") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }

        const auto object = findFirstObject(core, argv[2]);
        if (!object.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }

        const auto result = core.target().gotoObject(object->id, context);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "sync") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }

        const auto object = findFirstObject(core, argv[2]);
        if (!object.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }

        const auto result = core.target().syncObject(object->id, context);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "dry-run") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }

        const auto object = findFirstObject(core, argv[2]);
        if (!object.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }

        const auto preview_result = core.target().previewObject(object->id, context);
        if (!preview_result.ok) {
            std::cout << preview_result.log << "\n";
            return 1;
        }

        const auto parsed = parseResolvedCoordinates(preview_result.log);
        if (!parsed.has_value()) {
            std::cout << preview_result.log << "\n";
            return 1;
        }

        std::vector<std::string> commands;
        std::string motion_log;
        const auto dry_run_result = core.motion().goto_target(parsed->first, parsed->second, false, 0.0, 0.0, true, commands, motion_log);
        std::cout << "Resolved coordinates: " << preview_result.log << "\n";
        std::cout << "Dry run: " << motion_log << "\n";
        return dry_run_result.ok ? 0 : 1;
    }

    if (command == "tracking") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }

        std::string log;
        if (std::string(argv[2]) == "on") {
            const auto result = core.motion().start_tracking(log);
            std::cout << result.log << "\n";
            return result.ok ? 0 : 1;
        }

        if (std::string(argv[2]) == "off") {
            const auto result = core.motion().stop_tracking(log);
            std::cout << result.log << "\n";
            return result.ok ? 0 : 1;
        }

        printUsage(program_name);
        return 1;
    }

    if (command == "park") {
        std::string log;
        const auto result = core.motion().park(log);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "home") {
        std::string log;
        const auto result = core.motion().home(log);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "abort") {
        std::string log;
        const auto result = core.motion().abort(log);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    printUsage(program_name);
    return 1;
}
