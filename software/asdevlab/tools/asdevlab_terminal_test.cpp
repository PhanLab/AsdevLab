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

void setEnvIfPresent(const char* name, const std::string& value) {
    if (value.empty()) {
        return;
    }
#if defined(_WIN32)
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
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

bool parseDouble(const std::string& text, double& value) {
    try {
        value = std::stod(text);
        return true;
    } catch (...) {
        return false;
    }
}

bool parseRa(const std::string& text, double& ra_hours) {
    const auto parsed = asdevlab::hardware::LX200Parser::parseRightAscension(text);
    if (!parsed || !parsed->valid) {
        return false;
    }
    ra_hours = parsed->hours + parsed->minutes / 60.0 + parsed->seconds / 3600.0;
    return true;
}

bool parseDec(const std::string& text, double& dec_degrees) {
    const auto parsed = asdevlab::hardware::LX200Parser::parseDeclination(text);
    if (!parsed || !parsed->valid) {
        return false;
    }
    dec_degrees = static_cast<double>(parsed->degrees) + parsed->minutes / 60.0 + parsed->seconds / 3600.0;
    if (parsed->negative) {
        dec_degrees = -dec_degrees;
    }
    return true;
}

void printUsage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " <command> [args]\n";
    std::cout << "Commands:\n";
    std::cout << "  status\n";
    std::cout << "  search <name>\n";
    std::cout << "  preview <name>\n";
    std::cout << "  goto <name>\n";
    std::cout << "  sync <name>\n";
    std::cout << "  goto-radec <ra> <dec>\n";
    std::cout << "  goto-altaz <alt> <az>\n";
    std::cout << "  tracking on|off\n";
    std::cout << "  park\n";
    std::cout << "  home\n";
    std::cout << "  abort\n";
    std::cout << "  raw <lx200 command>\n";
    std::cout << "  dry-run <name>\n";
}

bool ensureLocation(asdevlab::catalog::ObservationContext& context) {
    if (context.latitude_degrees != 0.0 || context.longitude_degrees != 0.0) {
        return true;
    }

    double latitude = 0.0;
    double longitude = 0.0;
    const std::string lat_text = getEnv("ASDEVLAB_LATITUDE");
    const std::string lon_text = getEnv("ASDEVLAB_LONGITUDE");

    if (!lat_text.empty() && parseDouble(lat_text, latitude) && !lon_text.empty() && parseDouble(lon_text, longitude)) {
        context.latitude_degrees = latitude;
        context.longitude_degrees = longitude;
        return true;
    }

    std::cout << "Location unavailable. Enter latitude degrees: ";
    std::string lat_input;
    std::getline(std::cin, lat_input);
    std::cout << "Location unavailable. Enter longitude degrees: ";
    std::string lon_input;
    std::getline(std::cin, lon_input);

    if (!parseDouble(lat_input, latitude) || !parseDouble(lon_input, longitude)) {
        std::cout << "Invalid location input; using 0.0/0.0.\n";
        latitude = 0.0;
        longitude = 0.0;
    }

    setEnvIfPresent("ASDEVLAB_LATITUDE", std::to_string(latitude));
    setEnvIfPresent("ASDEVLAB_LONGITUDE", std::to_string(longitude));
    context.latitude_degrees = latitude;
    context.longitude_degrees = longitude;
    return true;
}

std::optional<asdevlab::catalog::CatalogObject> resolveCatalogObject(asdevlab::catalog::CatalogEngine& catalog, const std::string& query) {
    auto exact = catalog.getObject(query);
    if (exact.has_value()) {
        return **exact;
    }
    const auto matches = catalog.search(query);
    if (!matches.empty()) {
        return matches.front();
    }
    return std::nullopt;
}

std::optional<std::pair<double, double>> parseResolvedCoordinates(const std::string& preview_log) {
    const std::string marker = "ra=";
    const auto pos = preview_log.find(marker);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    const auto comma = preview_log.find(',', pos + marker.size());
    if (comma == std::string::npos) {
        return std::nullopt;
    }
    const auto dec_marker = std::string(" dec=");
    const auto dec_pos = preview_log.find(dec_marker, comma);
    if (dec_pos == std::string::npos) {
        return std::nullopt;
    }
    double ra = 0.0;
    double dec = 0.0;
    try {
        ra = std::stod(preview_log.substr(pos + marker.size(), comma - pos - marker.size()));
        dec = std::stod(preview_log.substr(dec_pos + dec_marker.size()));
    } catch (...) {
        return std::nullopt;
    }
    return std::make_pair(ra, dec);
}

std::string resolveCatalogPath() {
    const std::vector<std::string> candidates = {
        getEnv("ASDEVLAB_CATALOG_PATH"),
        "../data/catalog",
        "data/catalog",
        "software/asdevlab/data/catalog",
        "/home/phananh/Desktop/Project/AsdevLab/software/asdevlab/data/catalog",
    };

    for (const auto& candidate : candidates) {
        if (candidate.empty()) {
            continue;
        }
        std::filesystem::path path(candidate);
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            return candidate;
        }
    }
    return candidates.back();
}

void printDryRunSummary(const asdevlab::catalog::CatalogObject& object,
                        const asdevlab::catalog::ObservationContext& context,
                        const std::string& preview_log,
                        const std::string& mount_mode) {
    std::cout << "Building ObservationContext...\n";
    std::cout << "Latitude: " << context.latitude_degrees << "\n";
    std::cout << "Longitude: " << context.longitude_degrees << "\n";
    std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
    std::cout << "Searching catalog...\n";
    std::cout << "Object found...\n";
    std::cout << "Resolving coordinates...\n";
    std::cout << "Resolved target: " << preview_log << "\n";
    std::cout << "Mount mode = " << mount_mode << "\n";
    std::cout << "MotionService selected RA/DEC from the resolved target.\n";
    std::cout << "DRY RUN\n";
    std::cout << "No command dispatched.\n";
    std::cout << "Catalog coordinates: " << formatRaHours(object.ra) << " " << formatDecDegrees(object.dec) << "\n";
}

} // namespace

int main(int argc, char** argv) {
    const std::string program_name = argc > 0 ? argv[0] : "asdevlab_terminal_test";
    if (argc < 2) {
        printUsage(program_name);
        return 1;
    }

    asdevlab::TelescopeCore core;
    const std::string catalog_path = resolveCatalogPath();
    if (!core.catalog().loadCatalog(catalog_path)) {
        std::cerr << "Failed to load catalog from: " << catalog_path << "\n";
        return 1;
    }

    const std::string command = argv[1];
    auto context = core.observation().makeObservationContext();
    ensureLocation(context);
    context = core.observation().makeObservationContext();

    if (command == "status") {
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
        std::cout << "Using TelescopeCore APIs...\n";

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
        std::cout << "Searching catalog...\n";
        const auto results = core.target().search(argv[2]).results;
        if (results.empty()) {
            std::cout << "No results.\n";
            return 1;
        }
        std::cout << "Object found...\n";
        for (const auto& object : results) {
            std::cout << object.id << "\t" << object.name << "\t" << object.type << "\n";
        }
        return 0;
    }

    if (command == "preview") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const auto object_opt = resolveCatalogObject(core.catalog(), argv[2]);
        if (!object_opt.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }
        const auto object = *object_opt;
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
        std::cout << "Searching catalog...\n";
        std::cout << "Object found...\n";
        std::cout << "Resolving coordinates...\n";
        const auto preview_result = core.target().previewObject(object.id, context);
        std::cout << "Resolved coordinates: " << preview_result.log << "\n";
        std::cout << "Current mount mode: " << mountModeToString(core.motion().getMountMode()) << "\n";
        std::cout << "Horizontal coordinates: unavailable through TelescopeCore public API\n";
        return preview_result.ok ? 0 : 1;
    }

    if (command == "goto") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const auto object_opt = resolveCatalogObject(core.catalog(), argv[2]);
        if (!object_opt.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }
        const auto object = *object_opt;
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
        std::cout << "Searching catalog...\n";
        std::cout << "Object found...\n";
        std::cout << "Resolving coordinates...\n";
        const auto result = core.target().gotoObject(object.id, context);
        std::cout << "Motion request dispatched through TargetService -> MotionService -> MountInterface.\n";
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "sync") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const auto object_opt = resolveCatalogObject(core.catalog(), argv[2]);
        if (!object_opt.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }
        const auto object = *object_opt;
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
        std::cout << "Searching catalog...\n";
        std::cout << "Object found...\n";
        std::cout << "Resolving coordinates...\n";
        const auto result = core.target().syncObject(object.id, context);
        std::cout << "Motion request dispatched through TargetService -> MotionService -> MountInterface.\n";
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "goto-radec") {
        if (argc < 4) {
            printUsage(program_name);
            return 1;
        }
        double ra = 0.0;
        double dec = 0.0;
        if (!parseRa(argv[2], ra) || !parseDec(argv[3], dec)) {
            std::cout << "Invalid RA/DEC format.\n";
            return 1;
        }
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
        std::cout << "MotionService selected RA/DEC coordinates.\n";
        std::string log;
        const auto result = core.motion().goto_target(ra, dec, log);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
    }

    if (command == "goto-altaz") {
        if (argc < 4) {
            printUsage(program_name);
            return 1;
        }
        double alt = 0.0;
        double az = 0.0;
        if (!parseDouble(argv[2], alt) || !parseDouble(argv[3], az)) {
            std::cout << "Invalid ALT/AZ format.\n";
            return 1;
        }
        std::cout << "Building ObservationContext...\n";
        std::cout << "Latitude: " << context.latitude_degrees << "\n";
        std::cout << "Longitude: " << context.longitude_degrees << "\n";
        std::cout << "UTC: " << formatUtcTime(context.utc_time) << "\n";
        std::cout << "MotionService selected ALT/AZ coordinates.\n";
        std::string log;
        const auto result = core.motion().goto_target(0.0, 0.0, true, alt, az, log);
        std::cout << result.log << "\n";
        return result.ok ? 0 : 1;
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

    if (command == "raw") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        std::cout << "This operation is not exposed by TelescopeCore.\n";
        return 1;
    }

    if (command == "dry-run") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const auto object_opt = resolveCatalogObject(core.catalog(), argv[2]);
        if (!object_opt.has_value()) {
            std::cout << "Object not found.\n";
            return 1;
        }
        const auto object = *object_opt;
        const auto preview_result = core.target().previewObject(object.id, context);
        std::string mount_mode = mountModeToString(core.motion().getMountMode());
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
        printDryRunSummary(object, context, preview_result.log, mount_mode);
        std::cout << "Motion request: " << motion_log << "\n";
        std::cout << "Selected coordinates: RA=" << formatRaHours(parsed->first) << ", DEC=" << formatDecDegrees(parsed->second) << "\n";
        return dry_run_result.ok ? 0 : 1;
    }

    printUsage(program_name);
    return 1;
}
