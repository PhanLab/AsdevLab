#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"
#include "asdevlab/catalog/catalog_engine.hpp"
#include "asdevlab/catalog/catalog_object.hpp"
#include "asdevlab/catalog/coordinate_resolver.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/services/observation_service.hpp"
#include "asdevlab/catalog/resolved_target.hpp"
#include "asdevlab/hardware/mount/lx200_parser.hpp"
#include "asdevlab/hardware/mount/mount_config.hpp"
#include "asdevlab/hardware/mount/onstep_mount_client.hpp"
#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/services/target_service.hpp"
#include "asdevlab/safety_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

static std::string getEnv(const char* name, const std::string& fallback = "") {
    if (const char* value = std::getenv(name)) {
        return std::string(value);
    }
    return fallback;
}
// ObservationContext is produced by ObservationService now.

namespace {

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

bool parseDouble(const std::string& text, double& value) {
    try {
        value = std::stod(text);
        return true;
    } catch (...) {
        return false;
    }
}

std::string mountModeToString(asdevlab::hardware::MountMode mode) {
    switch (mode) {
        case asdevlab::hardware::MountMode::AltAz:
            return "ALTAZM";
        case asdevlab::hardware::MountMode::Equatorial:
            return "EQUATORIAL";
    }
    return "UNKNOWN";
}

// local context builder removed

std::optional<asdevlab::catalog::CatalogObject> resolveCatalogObject(const asdevlab::catalog::CatalogEngine& catalog,
                                                                      const std::string& query) {
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

std::string coordinatesString(double ra_hours, double dec_degrees) {
    if (ra_hours == 0.0 && dec_degrees == 0.0) {
        return "N/A";
    }
    return formatRaHours(ra_hours) + " " + formatDecDegrees(dec_degrees);
}

bool isSolarSystemObject(const asdevlab::catalog::CatalogObject& object) {
    const auto lower_type = [&]() {
        std::string normalized;
        normalized.reserve(object.type.size());
        for (char ch : object.type) {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return normalized;
    }();
    return lower_type == "planet" || lower_type == "moon" || lower_type == "sun" || lower_type == "comet" || lower_type == "asteroid" || lower_type == "satellite";
}

void printUsage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " <command> [args]\n";
    std::cout << "Commands:\n";
    std::cout << "  status\n";
    std::cout << "  search <query>\n";
    std::cout << "  preview <query>\n";
    std::cout << "  goto <query> [--dry-run]\n";
    std::cout << "  goto-radec <HH:MM:SS> <+DD*MM:SS> [--dry-run]\n";
    std::cout << "  goto-altaz <altitude_deg> <azimuth_deg> [--dry-run]\n";
    std::cout << "  move <north|south|east|west|stop>\n";
    std::cout << "  raw <lx200 command>\n";
}

} // namespace

int main(int argc, char** argv) {
    const std::string program_name = argc > 0 ? argv[0] : "asdevlab_lx200_terminal";
    if (argc < 2) {
        printUsage(program_name);
        return 1;
    }

    const std::string command = argv[1];
    const std::string catalog_path = getEnv("ASDEVLAB_CATALOG_PATH", "../data/catalog");
    asdevlab::catalog::CatalogEngine catalog;
    if (!catalog.loadCatalog(catalog_path)) {
        std::cerr << "Failed to load catalog from: " << catalog_path << "\n";
        return 1;
    }

    asdevlab::catalog::CoordinateResolver resolver;
    asdevlab::astronomy::AstronomyService astronomy_service(std::make_shared<asdevlab::astronomy::LibnovaEphemerisProvider>());
    asdevlab::hardware::OnStepMountClient mount_client(asdevlab::hardware::MountConfig::fromEnvironment());
    asdevlab::SafetyService safety_service;
    asdevlab::StateMachine state_machine;
    asdevlab::ObservationService observation_service;
    asdevlab::MotionService motion_service(safety_service, state_machine, mount_client, observation_service, astronomy_service);
    asdevlab::TargetService target_service(catalog, resolver, motion_service);
    const auto context = observation_service.makeObservationContext();

    if (command == "status") {
        const bool connected = mount_client.connect();
        std::cout << "[Connection] " << (connected ? "connected" : "failed") << "\n";

        const auto status_response = mount_client.getStatus(asdevlab::hardware::MountStatusRequest{});
        std::cout << "[Firmware] ";
        if (status_response.ok && status_response.status.valid) {
            std::cout << status_response.status.raw << "\n";
        } else {
            std::cout << status_response.message << "\n";
        }

        const auto mode_response = mount_client.getMountMode(asdevlab::hardware::MountModeRequest{});
        std::cout << "[Mount mode] ";
        if (mode_response.ok && mode_response.supported) {
            std::cout << mountModeToString(mode_response.mode) << "\n";
        } else {
            std::cout << "unknown\n";
        }

        std::cout << "[Tracking] ";
        if (status_response.ok && status_response.status.valid) {
            std::cout << (status_response.status.tracking_enabled ? "enabled" : "disabled") << "\n";
        } else {
            std::cout << "unknown\n";
        }

        const auto coordinates_response = mount_client.getCoordinates(asdevlab::hardware::MountCoordinatesRequest{});
        if (coordinates_response.ok && coordinates_response.coordinates.valid) {
            const auto& coords = coordinates_response.coordinates;
            std::cout << "[RA] " << std::setw(2) << std::setfill('0') << coords.ra.hours << ":"
                      << std::setw(2) << std::setfill('0') << coords.ra.minutes << ":"
                      << std::fixed << std::setprecision(2) << coords.ra.seconds << "\n";
            std::cout << "[DEC] " << (coords.dec.negative ? '-' : '+')
                      << std::setw(2) << std::setfill('0') << coords.dec.degrees << "*"
                      << std::setw(2) << std::setfill('0') << coords.dec.minutes << ":"
                      << std::fixed << std::setprecision(2) << coords.dec.seconds << "\n";

            asdevlab::catalog::CatalogObject fake;
            fake.ra = coords.ra.hours + coords.ra.minutes / 60.0 + coords.ra.seconds / 3600.0;
            fake.dec = (coords.dec.negative ? -1.0 : 1.0) * (coords.dec.degrees + coords.dec.minutes / 60.0 + coords.dec.seconds / 3600.0);
            fake.coordinate_source = "catalog";
            fake.provider = "static";
            fake.type = "star";
            fake.epoch = "J2000";
            const auto resolved = resolver.resolveTarget(fake, context);
            std::cout << "[ALT] " << std::fixed << std::setprecision(2) << resolved.alt_degrees << "\n";
            std::cout << "[AZ] " << std::fixed << std::setprecision(2) << resolved.az_degrees << "\n";
        } else {
            std::cout << "[RA] unknown\n";
            std::cout << "[DEC] unknown\n";
            std::cout << "[ALT] unknown\n";
            std::cout << "[AZ] unknown\n";
        }

        return 0;
    }

    if (command == "search") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const std::string query = argv[2];
        const auto results = target_service.search(query).results;
        if (results.empty()) {
            std::cout << "No results\n";
            return 0;
        }

        std::cout << "ID\tName\tType\tMagnitude\tRA\tDEC\n";
        for (const auto& object : results) {
            std::cout << object.id << '\t'
                      << object.name << '\t'
                      << object.type << '\t'
                      << object.magnitude << '\t'
                      << formatRaHours(object.ra) << ' '
                      << formatDecDegrees(object.dec) << '\n';
        }
        return 0;
    }

    if (command == "preview") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const std::string query = argv[2];
        const auto object_opt = resolveCatalogObject(catalog, query);
        if (!object_opt.has_value()) {
            std::cerr << "Object not found\n";
            return 1;
        }
        const auto object = *object_opt;
        const auto resolved = resolver.resolveTarget(object, context);
        const auto libnova_coords = astronomy_service.resolve(object.name, context);

        std::cout << "Object: " << object.id << " - " << object.name << "\n";
        std::cout << "Static coordinates: " << formatRaHours(object.ra) << ' ' << formatDecDegrees(object.dec) << "\n";
        std::cout << "Resolved coordinates: " << formatRaHours(resolved.ra_hours) << ' ' << formatDecDegrees(resolved.dec_degrees) << "\n";

        if (isSolarSystemObject(object) || object.coordinate_source == "ephemeris") {
            if (libnova_coords.ra_hours != 0.0 || libnova_coords.dec_degrees != 0.0) {
                std::cout << "Libnova coordinates: " << formatRaHours(libnova_coords.ra_hours) << ' ' << formatDecDegrees(libnova_coords.dec_degrees) << "\n";
            }
        }

        std::cout << "ALT " << std::fixed << std::setprecision(2) << resolved.alt_degrees << "\n";
        std::cout << "AZ " << std::fixed << std::setprecision(2) << resolved.az_degrees << "\n";
        return 0;
    }

    if (command == "goto") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const bool dry_run = (argc == 4 && std::string(argv[3]) == "--dry-run");
        const std::string query = argv[2];
        const auto object_opt = resolveCatalogObject(catalog, query);
        if (!object_opt.has_value()) {
            std::cerr << "Object not found\n";
            return 1;
        }
        const auto object = *object_opt;
        const auto resolved = resolver.resolveTarget(object, context);
        const auto astro_result = astronomy_service.resolve(object.name, context);

        if (!dry_run) {
            const bool connected = mount_client.connect();
            if (!connected) {
                std::cerr << "Mount connection failed\n";
                return 1;
            }
        }

        const auto mount_mode = mount_client.getDetectedMountMode();
        std::vector<std::string> lx200_commands;
        std::string log;
        if (dry_run) {
            // Preview commands are a mount concern; ask the concrete
            // OnStepMountClient for a command preview for CLI debugging.
            const auto cmds = mount_client.buildGotoCommands(asdevlab::hardware::MountGotoRequest{resolved.ra_hours, resolved.dec_degrees, mount_mode == asdevlab::hardware::MountMode::AltAz, resolved.alt_degrees, resolved.az_degrees});
            lx200_commands.insert(lx200_commands.end(), cmds.begin(), cmds.end());
        }
        const auto result = motion_service.goto_target(resolved.ra_hours,
                                                      resolved.dec_degrees,
                                                      mount_mode == asdevlab::hardware::MountMode::AltAz,
                                                      resolved.alt_degrees,
                                                      resolved.az_degrees,
                                                      dry_run,
                                                      lx200_commands,
                                                      log);

        std::cout << "-------------------------\n";
        std::cout << "Catalog object: " << object.name << "\n";
        std::cout << "Coordinate source: " << object.coordinate_source << "\n";
        std::cout << "Resolved RA: " << formatRaHours(resolved.ra_hours) << "\n";
        std::cout << "Resolved DEC: " << formatDecDegrees(resolved.dec_degrees) << "\n";
        std::cout << "Resolved ALT: " << std::fixed << std::setprecision(2) << resolved.alt_degrees << "\n";
        std::cout << "Resolved AZ: " << std::fixed << std::setprecision(2) << resolved.az_degrees << "\n";
        std::cout << "Mount mode: " << mountModeToString(mount_mode) << "\n";
        std::cout << "Tracking state: " << (dry_run ? "dry-run" : "unknown") << "\n";
        for (size_t i = 0; i < lx200_commands.size(); ++i) {
            std::cout << "LX200 command " << (i + 1) << ": " << lx200_commands[i] << "\n";
        }
        std::cout << "Mount response: " << log << "\n";
        std::cout << "-------------------------\n";
        return result.ok ? 0 : 1;
    }

    if (command == "goto-radec") {
        if (argc < 4) {
            printUsage(program_name);
            return 1;
        }
        const bool dry_run = (argc == 5 && std::string(argv[4]) == "--dry-run");
        double ra_hours = 0.0;
        double dec_degrees = 0.0;
        if (!parseRa(argv[2], ra_hours) || !parseDec(argv[3], dec_degrees)) {
            std::cerr << "Invalid RA/DEC format\n";
            return 1;
        }
        if (!dry_run) {
            const bool connected = mount_client.connect();
            if (!connected) {
                std::cerr << "Mount connection failed\n";
                return 1;
            }
        }
        const auto mount_mode = mount_client.getDetectedMountMode();
        std::vector<std::string> lx200_commands;
        std::string log;
        const auto result = motion_service.goto_target(ra_hours,
                                                      dec_degrees,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      dry_run,
                                                      lx200_commands,
                                                      log);
        std::cout << "-------------------------\n";
        std::cout << "Catalog object: direct RA/DEC\n";
        std::cout << "Coordinate source: direct\n";
        std::cout << "Resolved RA: " << formatRaHours(ra_hours) << "\n";
        std::cout << "Resolved DEC: " << formatDecDegrees(dec_degrees) << "\n";
        std::cout << "Resolved ALT: N/A\n";
        std::cout << "Resolved AZ: N/A\n";
        std::cout << "Mount mode: " << mountModeToString(mount_mode) << "\n";
        std::cout << "Tracking state: " << (dry_run ? "dry-run" : "unknown") << "\n";
        for (size_t i = 0; i < lx200_commands.size(); ++i) {
            std::cout << "LX200 command " << (i + 1) << ": " << lx200_commands[i] << "\n";
        }
        std::cout << "Mount response: " << log << "\n";
        std::cout << "-------------------------\n";
        return result.ok ? 0 : 1;
    }

    if (command == "goto-altaz") {
        if (argc < 4) {
            printUsage(program_name);
            return 1;
        }
        const bool dry_run = (argc == 5 && std::string(argv[4]) == "--dry-run");
        double alt = 0.0;
        double az = 0.0;
        if (!parseDouble(argv[2], alt) || !parseDouble(argv[3], az)) {
            std::cerr << "Invalid alt/az format\n";
            return 1;
        }
        if (!dry_run) {
            const bool connected = mount_client.connect();
            if (!connected) {
                std::cerr << "Mount connection failed\n";
                return 1;
            }
        }
        const auto mount_mode = mount_client.getDetectedMountMode();
        std::vector<std::string> lx200_commands;
        std::string log;
        const auto result = motion_service.goto_target(0.0,
                                                      0.0,
                                                      true,
                                                      alt,
                                                      az,
                                                      dry_run,
                                                      lx200_commands,
                                                      log);
        std::cout << "-------------------------\n";
        std::cout << "Catalog object: direct ALT/AZ\n";
        std::cout << "Coordinate source: direct\n";
        std::cout << "Resolved RA: N/A\n";
        std::cout << "Resolved DEC: N/A\n";
        std::cout << "Resolved ALT: " << std::fixed << std::setprecision(2) << alt << "\n";
        std::cout << "Resolved AZ: " << std::fixed << std::setprecision(2) << az << "\n";
        std::cout << "Mount mode: " << mountModeToString(mount_mode) << "\n";
        std::cout << "Tracking state: " << (dry_run ? "dry-run" : "unknown") << "\n";
        for (size_t i = 0; i < lx200_commands.size(); ++i) {
            std::cout << "LX200 command " << (i + 1) << ": " << lx200_commands[i] << "\n";
        }
        std::cout << "Mount response: " << log << "\n";
        std::cout << "-------------------------\n";
        return result.ok ? 0 : 1;
    }

    if (command == "move") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const std::string direction = argv[2];
        const bool connected = mount_client.connect();
        if (!connected) {
            std::cerr << "Mount connection failed\n";
            return 1;
        }
        std::cout << "Move: " << direction << "\n";
        std::cout << motion_service.move(direction, 0) << "\n";
        return 0;
    }

    if (command == "raw") {
        if (argc < 3) {
            printUsage(program_name);
            return 1;
        }
        const std::string raw_command = argv[2];
        const bool connected = mount_client.connect();
        if (!connected) {
            std::cerr << "Mount connection failed\n";
            return 1;
        }
        std::string response;
        if (!mount_client.sendRawCommand(raw_command, response)) {
            std::cerr << "Failed to send raw command\n";
            return 1;
        }
        std::cout << "command " << raw_command << "\n";
        std::cout << "response " << response << "\n";
        return 0;
    }

    printUsage(program_name);
    return 1;
}
