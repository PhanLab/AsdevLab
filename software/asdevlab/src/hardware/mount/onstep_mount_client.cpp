#include "asdevlab/hardware/mount/onstep_mount_client.hpp"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace asdevlab {
namespace hardware {
namespace {

std::string formatRaHours(double ra_hours) {
    const int hours = static_cast<int>(std::floor(ra_hours));
    const double minutes_f = (ra_hours - hours) * 60.0;
    const int minutes = static_cast<int>(std::floor(minutes_f));
    const double seconds = (minutes_f - minutes) * 60.0;

    std::ostringstream oss;
    oss << ":Sr" << std::setw(2) << std::setfill('0') << hours
        << ':' << std::setw(2) << std::setfill('0') << minutes
        << ':' << std::fixed << std::setprecision(2) << seconds << '#';
    return oss.str();
}

std::string formatDecDegrees(double dec_deg) {
    const bool negative = dec_deg < 0.0;
    const double abs_dec = std::abs(dec_deg);
    const int degrees = static_cast<int>(std::floor(abs_dec));
    const double minutes_f = (abs_dec - degrees) * 60.0;
    const int minutes = static_cast<int>(std::floor(minutes_f));
    const double seconds = (minutes_f - minutes) * 60.0;

    std::ostringstream oss;
    oss << ":Sd" << (negative ? '-' : '+') << std::setw(2) << std::setfill('0') << degrees
        << '*' << std::setw(2) << std::setfill('0') << minutes
        << ':' << std::fixed << std::setprecision(2) << seconds << '#';
    return oss.str();
}

std::pair<std::string, std::string> formatAltAzCommands(double alt_deg, double az_deg) {
    std::ostringstream alt;
    alt << ":Sa" << std::fixed << std::setprecision(2) << alt_deg << '#';
    std::ostringstream az;
    az << ":Sz" << std::fixed << std::setprecision(2) << az_deg << '#';
    return {alt.str(), az.str()};
}

std::string formatHorizonLimitCommand(double horizon_deg) {
    const int degrees = static_cast<int>(std::lround(horizon_deg));
    return std::string(":Sh") + std::to_string(degrees) + "#";
}

std::string formatOverheadLimitCommand(double overhead_deg) {
    const int degrees = static_cast<int>(std::lround(overhead_deg));
    return std::string(":So") + std::to_string(degrees) + "#";
}

std::string formatMeridianLimitCommand(double meridian_minutes) {
    const long minutes = std::lround(meridian_minutes);
    return std::string(":SXE9,") + std::to_string(minutes) + "#";
}

std::string formatTrackingRate(TrackingRatePreset preset, double rate) {
    switch (preset) {
        case TrackingRatePreset::Lunar:
            return ":TL#";
        case TrackingRatePreset::Solar:
            return ":TS#";
        case TrackingRatePreset::Custom:
            return ":ST" + std::to_string(rate) + "#";
        case TrackingRatePreset::Sidereal:
        default:
            return ":TQ#";
    }
}

std::string formatGuideRate(GuideRatePreset preset) {
    switch (preset) {
        case GuideRatePreset::Medium:
            return ":RM#";
        case GuideRatePreset::Fast:
            return ":RF#";
        case GuideRatePreset::Slow:
            return ":RG#";
        case GuideRatePreset::Custom:
        default:
            return ":RS#";
    }
}

std::string formatSlewRate(SlewRatePreset preset) {
    switch (preset) {
        case SlewRatePreset::Fast:
            return ":SX9,3#";
        case SlewRatePreset::Slow:
            return ":SX9,2#";
        case SlewRatePreset::Custom:
            return ":SX9,2#";
        case SlewRatePreset::Center:
        default:
            return ":SX9,1#";
    }
}

std::string formatManualCommand(ManualMotionAxis axis, bool start) {
    if (!start) {
        switch (axis) {
            case ManualMotionAxis::North:
                return ":Qn#";
            case ManualMotionAxis::South:
                return ":Qs#";
            case ManualMotionAxis::East:
                return ":Qe#";
            case ManualMotionAxis::West:
                return ":Qw#";
            default:
                return ":Q#";
        }
    }
    switch (axis) {
        case ManualMotionAxis::North:
            return ":Mn#";
        case ManualMotionAxis::South:
            return ":Ms#";
        case ManualMotionAxis::East:
            return ":Me#";
        case ManualMotionAxis::West:
            return ":Mw#";
        default:
            return ":Q#";
    }
}

std::string formatPulseGuideCommand(GuideDirection direction, int duration_ms) {
    std::ostringstream oss;
    oss << ":Mgd";
    switch (direction) {
        case GuideDirection::West:
            oss << 'w';
            break;
        case GuideDirection::East:
            oss << 'e';
            break;
        case GuideDirection::South:
            oss << 's';
            break;
        case GuideDirection::North:
        default:
            oss << 'n';
            break;
    }
    oss << duration_ms << "#";
    return oss.str();
}

std::string formatAuxCommand(const MountAuxRequest& request) {
    if (request.enable_buzzer) {
        return ":SX97,1#";
    }
    if (request.disable_buzzer) {
        return ":SX97,0#";
    }
    return ":D#";
}

std::string readEnvOrDefault(const char* name, const std::string& default_value) {
    if (const char* value = std::getenv(name)) {
        return std::string(value);
    }
    return default_value;
}

struct HostSiteTimeValues {
    double latitude_deg = 0.0;
    double longitude_deg = 0.0;
    double elevation_m = 0.0;
    std::string local_date;
    std::string local_time;
    double utc_offset_hours = 0.0;
};

HostSiteTimeValues readHostSiteTimeValues() {
    HostSiteTimeValues values;
    values.latitude_deg = std::atof(readEnvOrDefault("ASDEVLAB_LATITUDE", "0").c_str());
    values.longitude_deg = std::atof(readEnvOrDefault("ASDEVLAB_LONGITUDE", "0").c_str());
    values.elevation_m = std::atof(readEnvOrDefault("ASDEVLAB_ELEVATION", "0").c_str());
    values.local_date = readEnvOrDefault("ASDEVLAB_LOCAL_DATE", "");
    values.local_time = readEnvOrDefault("ASDEVLAB_LOCAL_TIME", "");
    values.utc_offset_hours = std::atof(readEnvOrDefault("ASDEVLAB_UTC_OFFSET", "0").c_str());
    return values;
}

std::string formatLatitudeCommand(double latitude_deg) {
    const bool negative = latitude_deg < 0.0;
    const double abs_lat = std::abs(latitude_deg);
    const int degrees = static_cast<int>(std::floor(abs_lat));
    const double minutes_f = (abs_lat - degrees) * 60.0;
    const int minutes = static_cast<int>(std::floor(minutes_f));
    const int seconds = static_cast<int>(std::lround((minutes_f - minutes) * 60.0));

    std::ostringstream oss;
    oss << ":St" << (negative ? '-' : '+') << std::setw(2) << std::setfill('0') << degrees
        << '*' << std::setw(2) << std::setfill('0') << minutes
        << ':' << std::setw(2) << std::setfill('0') << seconds << '#';
    return oss.str();
}

std::string formatLongitudeCommand(double longitude_deg) {
    const bool negative = longitude_deg < 0.0;
    const double abs_lon = std::abs(longitude_deg);
    const int degrees = static_cast<int>(std::floor(abs_lon));
    const double minutes_f = (abs_lon - degrees) * 60.0;
    const int minutes = static_cast<int>(std::floor(minutes_f));
    const int seconds = static_cast<int>(std::lround((minutes_f - minutes) * 60.0));

    std::ostringstream oss;
    oss << ":Sg" << (negative ? '-' : '+') << std::setw(2) << std::setfill('0') << degrees
        << '*' << std::setw(2) << std::setfill('0') << minutes
        << ':' << std::setw(2) << std::setfill('0') << seconds << '#';
    return oss.str();
}

std::string formatElevationCommand(double elevation_m) {
    std::ostringstream oss;
    oss << ":Sv" << std::lround(elevation_m) << "#";
    return oss.str();
}

std::string formatDateCommand(const std::string& date) {
    std::ostringstream oss;
    oss << ":SC" << date << "#";
    return oss.str();
}

std::string formatTimeCommand(const std::string& time) {
    std::ostringstream oss;
    oss << ":SL" << time << "#";
    return oss.str();
}

std::string formatUtcOffsetCommand(double utc_offset_hours) {
    const long total_minutes = std::lround(utc_offset_hours * 60.0);
    const bool negative = total_minutes < 0;
    const long abs_minutes = std::llabs(total_minutes);
    const long hours = abs_minutes / 60;
    const long minutes = abs_minutes % 60;

    std::ostringstream oss;
    oss << ":SG" << (negative ? '-' : '+')
        << std::setw(2) << std::setfill('0') << hours
        << ':' << std::setw(2) << std::setfill('0') << minutes << '#';
    return oss.str();
}

bool sendInitializationCommands(const std::function<bool(const std::string&, std::string&)>& sender) {
    std::string response;
    const HostSiteTimeValues values = readHostSiteTimeValues();

    if (!sender(formatLatitudeCommand(values.latitude_deg), response)) return false;
    if (!sender(formatLongitudeCommand(values.longitude_deg), response)) return false;
    if (!sender(formatElevationCommand(values.elevation_m), response)) return false;
    if (!values.local_date.empty() && !sender(formatDateCommand(values.local_date), response)) return false;
    if (!values.local_time.empty() && !sender(formatTimeCommand(values.local_time), response)) return false;
    if (!sender(formatUtcOffsetCommand(values.utc_offset_hours), response)) return false;
    return true;
}

} // namespace

OnStepMountClient::OnStepMountClient(std::string host, int port)
    : connection_manager_(nullptr)
    , transport_(nullptr) {
    MountConfig config = MountConfig::fromEnvironment();
    if (!host.empty()) {
        config.host = std::move(host);
    }
    if (port != 0) {
        config.port = port;
    }
    connection_manager_ = std::make_unique<Lx200ConnectionManager>(std::move(config));
}

OnStepMountClient::OnStepMountClient(MountConfig config)
    : connection_manager_(std::make_unique<Lx200ConnectionManager>(std::move(config)))
    , transport_(nullptr) {}

OnStepMountClient::OnStepMountClient(std::unique_ptr<TransportInterface> transport)
    : connection_manager_(nullptr)
    , transport_(std::move(transport)) {}

bool OnStepMountClient::setFlipMirrorCamera() {
    std::string ignored;
    const bool sent = sendCommand(":FM1#", ignored);
    if (!sent) {
        return false;
    }
    return true;
}

bool OnStepMountClient::setFlipMirrorEyepiece() {
    std::string ignored;
    const bool sent = sendCommand(":FM0#", ignored);
    if (!sent) {
        return false;
    }
    return true;
}

FlipMirrorPosition OnStepMountClient::getFlipMirrorState() {
    std::string raw;
    const bool sent = sendCommand(":FM?#", raw);
    if (!sent) {
        return FlipMirrorPosition::Unknown;
    }

    const auto parsed = LX200Parser::parseFlipMirrorState(raw);
    if (!parsed) {
        return FlipMirrorPosition::Unknown;
    }
    return parsed->position;
}

bool OnStepMountClient::connect() {
    if (connection_manager_) {
        if (!connection_manager_->connect()) {
            return false;
        }
        return sendInitializationCommands([this](const std::string& command, std::string& response_out) {
            return connection_manager_->sendCommand(command, response_out);
        });
    }
    if (!transport_) {
        return false;
    }
    return sendInitializationCommands([this](const std::string& command, std::string& response_out) {
        return transport_->send(command, response_out);
    });
}

std::optional<MountStatus> OnStepMountClient::readMountStatus() {
    std::string response;
    if (sendCommand(":Gu#", response)) {
        const auto packed = LX200Parser::parsePackedMountStatus(response);
        if (packed) {
            return packed;
        }
    }
    if (!sendCommand(":GU#", response)) {
        return std::nullopt;
    }
    return LX200Parser::parseMountStatus(response);
}

std::optional<RightAscension> OnStepMountClient::readRightAscension() {
    std::string response;
    if (!sendCommand(":GR#", response)) {
        return std::nullopt;
    }
    return LX200Parser::parseRightAscension(response);
}

std::optional<Declination> OnStepMountClient::readDeclination() {
    std::string response;
    if (!sendCommand(":GD#", response)) {
        return std::nullopt;
    }
    return LX200Parser::parseDeclination(response);
}

std::optional<Coordinates> OnStepMountClient::readCoordinates() {
    std::string ra_response;
    std::string dec_response;
    if (!sendCommand(":GR#", ra_response) || !sendCommand(":GD#", dec_response)) {
        return std::nullopt;
    }
    return LX200Parser::parseCoordinates(ra_response, dec_response);
}

std::optional<GotoResult> OnStepMountClient::readGotoState() {
    std::string response;
    if (!sendCommand(":MS#", response)) {
        return std::nullopt;
    }
    return LX200Parser::parseGotoState(response);
}

bool OnStepMountClient::gotoRaDec(double ra_hours, double dec_deg) {
    std::string response;
    if (!sendCommand(formatRaHours(ra_hours), response)) {
        return false;
    }

    if (!sendCommand(formatDecDegrees(dec_deg), response)) {
        return false;
    }

    return sendCommand(":MS#", response);
}

bool OnStepMountClient::syncRaDec(double ra_hours, double dec_deg) {
    std::string response;
    if (!sendCommand(formatRaHours(ra_hours), response)) {
        return false;
    }

    if (!sendCommand(formatDecDegrees(dec_deg), response)) {
        return false;
    }

    return sendCommand(":CM#", response);
}

bool OnStepMountClient::startTracking() {
    std::string response;
    return sendCommand(":Te#", response);
}

bool OnStepMountClient::stopTracking() {
    std::string response;
    return sendCommand(":Td#", response);
}

bool OnStepMountClient::park() {
    std::string response;
    return sendCommand(":hP#", response);
}

bool OnStepMountClient::home() {
    std::string response;
    return sendCommand(":hF#", response);
}

bool OnStepMountClient::abort() {
    std::string response;
    return sendCommand(":Q#", response);
}

std::string OnStepMountClient::getStatus() {
    const auto status = readMountStatus();
    if (!status || !status->valid) {
        return "error";
    }
    return status->raw;
}

std::vector<MountCapability> OnStepMountClient::capabilities() const {
    return {
        {MountCapabilityId::Status, true, "Status and simple mount state"},
        {MountCapabilityId::Coordinates, true, "RA/Dec coordinate readback"},
        {MountCapabilityId::Slew, true, "Goto and sync"},
        {MountCapabilityId::Tracking, true, "Tracking enable/disable"},
        {MountCapabilityId::Guide, true, "Guide support"},
        {MountCapabilityId::Motion, true, "Manual motion support"},
        {MountCapabilityId::Park, true, "Park support"},
        {MountCapabilityId::Home, true, "Home support"},
        {MountCapabilityId::Alignment, true, "Manual alignment and star acceptance"},
        {MountCapabilityId::Site, true, "Site configuration"},
        {MountCapabilityId::Time, true, "Time and date configuration"},
        {MountCapabilityId::Pec, true, "PEC control"},
        {MountCapabilityId::Limits, true, "Limits configuration"},
        {MountCapabilityId::Focuser, true, "Focuser support"},
        {MountCapabilityId::FlipMirror, true, "Flip mirror support"},
        {MountCapabilityId::TrackingRate, true, "Tracking rate presets"},
        {MountCapabilityId::GuideRate, true, "Guide rate controls"},
        {MountCapabilityId::SlewRate, true, "Slew rate controls"},
        {MountCapabilityId::Auxiliary, true, "Buzzer and auxiliary status"},
        {MountCapabilityId::Orientation, true, "Mount orientation support"},
        {MountCapabilityId::Environmental, true, "Environmental sensor data"},
        {MountCapabilityId::Rotator, true, "Rotator availability and driver state"},
        {MountCapabilityId::MountMode, true, "Mount mode (AltAz / Equatorial)"},
    };
}

bool OnStepMountClient::isCapabilitySupported(MountCapabilityId id) const {
    for (const auto& capability : capabilities()) {
        if (capability.id == id) {
            return capability.supported;
        }
    }
    return false;
}

MountStatusResponse OnStepMountClient::getStatus(const MountStatusRequest& request) {
    MountStatusResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Status);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "status capability unsupported";
        return response;
    }
    const auto status = readMountStatus();
    if (!status || !status->valid) {
        response.error = MountError::InvalidResponse;
        response.message = "failed to parse mount status";
        return response;
    }
    response.status = *status;
    response.ok = true;
    response.raw_response = status->raw;
    if (request.include_raw) {
        response.message = "status available";
    }
    return response;
}

std::vector<std::string> OnStepMountClient::buildGotoCommands(const MountGotoRequest& request) {
    std::vector<std::string> commands;
    if (request.use_alt_az) {
        const auto [alt_cmd, az_cmd] = formatAltAzCommands(request.alt_deg, request.az_deg);
        commands.push_back(alt_cmd);
        commands.push_back(az_cmd);
        commands.push_back(":MS#");
    } else {
        commands.push_back(formatRaHours(request.ra_hours));
        commands.push_back(formatDecDegrees(request.dec_deg));
        commands.push_back(":MS#");
    }
    return commands;
}

MountGotoResponse OnStepMountClient::gotoTarget(const MountGotoRequest& request) {
    MountGotoResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Slew);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "goto capability unsupported";
        return response;
    }
    bool ok = false;
    if (request.use_alt_az) {
        std::string raw;
        const auto [alt_cmd, az_cmd] = formatAltAzCommands(request.alt_deg, request.az_deg);
        if (sendCommand(alt_cmd, raw) && sendCommand(az_cmd, raw)) {
            ok = sendCommand(":MS#", raw);
        }
    } else {
        ok = gotoRaDec(request.ra_hours, request.dec_deg);
    }
    response.ok = ok;
    response.error = ok ? MountError::None : MountError::Communication;
    response.message = ok ? "goto command accepted" : "goto command failed";
    const auto goto_state = readGotoState();
    if (goto_state) {
        response.goto_state = *goto_state;
    }
    return response;
}

MountSyncResponse OnStepMountClient::syncTarget(const MountSyncRequest& request) {
    MountSyncResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Slew);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "sync capability unsupported";
        return response;
    }
    bool ok = false;
    if (request.use_alt_az) {
        std::string raw;
        const auto [alt_cmd, az_cmd] = formatAltAzCommands(request.alt_deg, request.az_deg);
        if (sendCommand(alt_cmd, raw) && sendCommand(az_cmd, raw)) {
            ok = sendCommand(":CM#", raw);
        }
    } else {
        ok = syncRaDec(request.ra_hours, request.dec_deg);
    }
    response.ok = ok;
    response.error = ok ? MountError::None : MountError::Communication;
    response.message = ok ? "sync command accepted" : "sync command failed";
    const auto goto_state = readGotoState();
    if (goto_state) {
        response.goto_state = *goto_state;
    }
    return response;
}

MountSafetyResponse OnStepMountClient::safety(const MountSafetyRequest& request) {
    MountSafetyResponse response;
    response.supported = true;
    const bool ok = abort();
    response.ok = ok;
    response.aborted = ok;
    response.error = ok ? MountError::None : MountError::Communication;
    response.message = request.abort ? (ok ? "abort command accepted" : "abort command failed") : "safety request accepted";
    return response;
}

MountCoordinatesResponse OnStepMountClient::getCoordinates(const MountCoordinatesRequest& request) {
    MountCoordinatesResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Coordinates);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "coordinate capability unsupported";
        return response;
    }
    const auto coordinates = readCoordinates();
    if (!coordinates || !coordinates->valid) {
        response.error = MountError::InvalidResponse;
        response.message = "failed to parse coordinates";
        return response;
    }
    response.coordinates = *coordinates;
    response.ok = true;
    response.raw_response = request.include_raw ? "coordinates parsed" : "";
    return response;
}

MountSlewResponse OnStepMountClient::slew(const MountSlewRequest& request) {
    MountSlewResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Slew);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "slew capability unsupported";
        return response;
    }
    bool ok = false;
    if (request.mode == SlewMode::Sync) {
        ok = syncRaDec(request.ra_hours, request.dec_deg);
    } else {
        ok = gotoRaDec(request.ra_hours, request.dec_deg);
    }
    response.ok = ok;
    response.error = ok ? MountError::None : MountError::Communication;
    response.message = ok ? "slew command accepted" : "slew command failed";
    const auto goto_state = readGotoState();
    if (goto_state) {
        response.goto_state = *goto_state;
    }
    return response;
}

MountTrackingResponse OnStepMountClient::getTracking(const MountTrackingRequest& request) {
    MountTrackingResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Tracking);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "tracking capability unsupported";
        return response;
    }
    std::string raw;
    sendCommand(":GW#", raw);
    const auto tracking = LX200Parser::parseTracking(raw);
    response.tracking = tracking.value_or(TrackingStatus{});
    response.ok = tracking.has_value();
    response.error = response.ok ? MountError::None : MountError::InvalidResponse;
    response.message = response.ok ? "tracking state parsed" : "tracking state unavailable";
    return response;
}

MountGuideResponse OnStepMountClient::guide(const MountGuideRequest& request) {
    MountGuideResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Guide);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "guide capability unsupported";
        return response;
    }
    std::string raw;
    std::string command;
    if (request.continuous) {
        switch (request.direction) {
            case GuideDirection::West:
                command = ":Mw#";
                break;
            case GuideDirection::East:
                command = ":Me#";
                break;
            case GuideDirection::South:
                command = ":Ms#";
                break;
            case GuideDirection::North:
            default:
                command = ":Mn#";
                break;
        }
    } else {
        command = formatPulseGuideCommand(request.direction, request.duration_ms);
    }
    const bool sent = sendCommand(command, raw);
    const auto guide = LX200Parser::parseGuide(raw);
    response.guide = guide.value_or(GuideStatus{});
    response.ok = sent && guide.has_value();
    if (!sent) {
        response.error = MountError::Communication;
        response.message = "guide command failed";
    } else if (!guide.has_value()) {
        response.error = MountError::InvalidResponse;
        response.message = "guide response invalid";
    } else {
        response.error = MountError::None;
        response.message = "guide command accepted";
    }
    return response;
}

MountParkResponse OnStepMountClient::getParkStatus(const MountParkRequest& request) {
    MountParkResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Park);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "park capability unsupported";
        return response;
    }
    std::string raw;
    bool sent = false;
    if (request.set_park_position) {
        sent = sendCommand(":hQ#", raw);
    } else if (request.unpark) {
        sent = sendCommand(":hR#", raw);
    } else {
        sent = sendCommand(":h?#", raw);
    }
    const auto park = LX200Parser::parseParkStatus(raw);
    response.park = park.value_or(ParkStatus{});
    response.ok = sent && (request.set_park_position || request.unpark || park.has_value());
    response.error = response.ok ? MountError::None : MountError::InvalidResponse;
    response.message = response.ok ? "park status parsed" : "park status unavailable";
    return response;
}

MountHomeResponse OnStepMountClient::getHomeStatus(const MountHomeRequest& request) {
    MountHomeResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Home);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "home capability unsupported";
        return response;
    }
    std::string raw;
    const bool sent = request.move_to_home ? sendCommand(":hC#", raw) : sendCommand(":h?#", raw);
    const auto home = LX200Parser::parseHomeStatus(raw);
    response.home = home.value_or(HomeStatus{});
    response.ok = sent && (request.move_to_home || home.has_value());
    response.error = response.ok ? MountError::None : MountError::InvalidResponse;
    response.message = response.ok ? "home status parsed" : "home status unavailable";
    return response;
}

MountHomeConfigurationResponse OnStepMountClient::configureHome(const MountHomeConfigurationRequest& request) {
    MountHomeConfigurationResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Home);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "home configuration capability unsupported";
        return response;
    }

    std::string raw;
    bool sent = true;
    bool has_action = false;
    if (request.enable_auto_home) {
        sent = sendCommand(":hA1#", raw) && sent;
        has_action = true;
    }
    if (request.disable_auto_home) {
        sent = sendCommand(":hA0#", raw) && sent;
        has_action = true;
    }
    if (request.apply_axis1_offset) {
        std::ostringstream command;
        command << ":hC1," << request.axis1_offset_arcsec << "#";
        sent = sendCommand(command.str(), raw) && sent;
        has_action = true;
    }
    if (request.apply_axis2_offset) {
        std::ostringstream command;
        command << ":hC2," << request.axis2_offset_arcsec << "#";
        sent = sendCommand(command.str(), raw) && sent;
        has_action = true;
    }
    if (!has_action) {
        sent = sendCommand(":h?#", raw);
    }

    if (!sent) {
        response.error = MountError::Communication;
        response.message = "home configuration command failed";
        return response;
    }

    response.persisted = request.save || request.enable_auto_home || request.disable_auto_home || request.apply_axis1_offset || request.apply_axis2_offset;
    response.ok = true;
    response.error = MountError::None;
    response.message = response.persisted ? "home configuration saved" : "home configuration parsed";

    if (request.query || !has_action) {
        std::string config_raw;
        const bool query_sent = sendCommand(":h?#", config_raw);
        if (!query_sent) {
            response.ok = false;
            response.error = MountError::Communication;
            response.message = "home configuration readback failed";
            return response;
        }
        const auto configuration = LX200Parser::parseHomeConfiguration(config_raw);
        if (!configuration) {
            response.ok = false;
            response.error = MountError::InvalidResponse;
            response.message = "home configuration unavailable";
            return response;
        }
        response.configuration = *configuration;
        response.ok = true;
        response.error = MountError::None;
        response.message = "home configuration parsed";
    } else {
        response.configuration.valid = true;
        response.configuration.auto_home_enabled = request.enable_auto_home;
        response.configuration.axis1_offset_arcsec = request.apply_axis1_offset ? request.axis1_offset_arcsec : 0;
        response.configuration.axis2_offset_arcsec = request.apply_axis2_offset ? request.axis2_offset_arcsec : 0;
    }
    return response;
}

MountSiteResponse OnStepMountClient::getSite(const MountSiteRequest& request) {
    MountSiteResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Site);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "site capability unsupported";
        return response;
    }

    const HostSiteTimeValues values = readHostSiteTimeValues();

    SiteInfo site;
    site.valid = true;
    site.latitude_deg = values.latitude_deg;
    site.longitude_deg = values.longitude_deg;
    site.elevation_m = values.elevation_m;

    response.site = site;
    response.ok = true;
    response.error = MountError::None;
    response.message = "site from local configuration";
    return response;
}

MountTimeResponse OnStepMountClient::getTime(const MountTimeRequest& request) {
    MountTimeResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Time);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "time capability unsupported";
        return response;
    }

    const HostSiteTimeValues values = readHostSiteTimeValues();

    MountTimeInfo mount_time;
    mount_time.valid = !values.local_date.empty() && !values.local_time.empty();
    mount_time.local_date = values.local_date;
    mount_time.local_time = values.local_time;
    mount_time.sidereal_time = values.local_time;

    response.time = mount_time;
    response.ok = mount_time.valid;
    response.error = mount_time.valid ? MountError::None : MountError::InvalidResponse;
    response.message = mount_time.valid ? "time from local configuration" : "time unavailable";
    return response;
}

MountTrackingRateResponse OnStepMountClient::setTrackingRate(const MountTrackingRateRequest& request) {
    MountTrackingRateResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::TrackingRate);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "tracking rate capability unsupported";
        return response;
    }

    std::string raw;
    if (request.readback) {
        const bool sent = sendCommand(":TQ#", raw);
        const auto rate = LX200Parser::parseTrackingRate(raw);
        response.ok = sent && rate.has_value();
        response.error = response.ok ? MountError::None : MountError::InvalidResponse;
        response.message = response.ok ? "tracking rate parsed" : "tracking rate unavailable";
        if (rate.has_value()) {
            response.rate = *rate;
        }
        return response;
    }

    const std::string command = formatTrackingRate(request.preset, request.rate);
    const bool sent = sendCommand(command, raw);
    response.ok = sent;
    response.error = sent ? MountError::None : MountError::Communication;
    response.message = sent ? "tracking rate applied" : "tracking rate command failed";
    if (sent) {
        if (request.preset == TrackingRatePreset::Sidereal) {
            response.rate = 0.0;
        } else if (request.preset == TrackingRatePreset::Lunar) {
            response.rate = 0.75;
        } else if (request.preset == TrackingRatePreset::Solar) {
            response.rate = 0.99;
        } else {
            response.rate = request.rate;
        }
    }
    return response;
}

MountPecResponse OnStepMountClient::getPec(const MountPecRequest& request) {
    MountPecResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Pec);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "PEC capability unsupported";
        return response;
    }

    if (request.enable && request.disable) {
        response.error = MountError::InvalidRequest;
        response.message = "PEC request cannot enable and disable simultaneously";
        return response;
    }

    std::string raw;
    bool command_ok = true;
    if (request.enable) {
        command_ok = sendCommand(":$QZ+#", raw);
        response.message = command_ok ? "PEC enable command accepted" : "PEC enable command failed";
        response.ok = command_ok;
        response.error = command_ok ? MountError::None : MountError::Communication;
        if (!request.query) {
            return response;
        }
    } else if (request.disable) {
        command_ok = sendCommand(":$QZ-#", raw);
        response.message = command_ok ? "PEC disable command accepted" : "PEC disable command failed";
        response.ok = command_ok;
        response.error = command_ok ? MountError::None : MountError::Communication;
        if (!request.query) {
            return response;
        }
    }

    const bool status_ok = sendCommand(":$QZ?#", raw);
    const auto pec = LX200Parser::parsePecStatus(raw);
    response.pec = pec.value_or(PecStatus{});
    response.ok = command_ok && status_ok && pec.has_value();
    response.error = response.ok ? MountError::None : MountError::InvalidResponse;
    response.message = response.ok ? "PEC status parsed" : "PEC unavailable";
    return response;
}

MountLimitsResponse OnStepMountClient::getLimits(const MountLimitsRequest& request) {
    MountLimitsResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Limits);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "limits capability unsupported";
        return response;
    }

    std::string raw;
    bool command_ok = true;
    bool wrote = false;
    LimitsStatus written;

    if (request.horizon_deg.has_value()) {
        command_ok = sendCommand(formatHorizonLimitCommand(*request.horizon_deg), raw) && command_ok;
        wrote = true;
        written.horizon_deg = *request.horizon_deg;
    }
    if (request.overhead_deg.has_value()) {
        command_ok = sendCommand(formatOverheadLimitCommand(*request.overhead_deg), raw) && command_ok;
        wrote = true;
        written.overhead_deg = *request.overhead_deg;
    }
    if (request.meridian_deg.has_value()) {
        command_ok = sendCommand(formatMeridianLimitCommand(*request.meridian_deg), raw) && command_ok;
        wrote = true;
        written.meridian_deg = *request.meridian_deg;
    }

    if (!request.query && wrote) {
        response.ok = command_ok;
        response.error = command_ok ? MountError::None : MountError::Communication;
        response.message = command_ok ? "limits write accepted" : "limits write failed";
        response.limits = written;
        response.limits.valid = command_ok;
        return response;
    }

    std::string horizon;
    std::string overhead;
    std::string meridian;
    const bool horizon_ok = sendCommand(":Gh#", horizon);
    const bool overhead_ok = sendCommand(":Go#", overhead);
    const bool meridian_ok = sendCommand(":GXE9#", meridian);
    const auto limits = LX200Parser::parseLimits(horizon, overhead, meridian);
    response.limits = limits.value_or(LimitsStatus{});

    const bool read_ok = horizon_ok && overhead_ok && meridian_ok && limits.has_value();
    response.ok = command_ok && read_ok;
    if (!response.ok) {
        response.error = command_ok ? MountError::InvalidResponse : MountError::Communication;
        response.message = command_ok ? "limits unavailable" : "limits command failed";
    } else {
        response.error = MountError::None;
        response.message = "limits parsed";
    }
    return response;
}

MountFocuserResponse OnStepMountClient::getFocuser(const MountFocuserRequest& request) {
    MountFocuserResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Focuser);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "focuser capability unsupported";
        return response;
    }

    switch (request.action) {
        case FocuserControlAction::MoveInward: {
            std::string raw;
            const bool sent = sendCommand(":F-#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "focuser move inward accepted" : "focuser move inward failed";
            return response;
        }
        case FocuserControlAction::MoveOutward: {
            std::string raw;
            const bool sent = sendCommand(":F+#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "focuser move outward accepted" : "focuser move outward failed";
            return response;
        }
        case FocuserControlAction::Stop: {
            std::string raw;
            const bool sent = sendCommand(":FQ#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "focuser stop accepted" : "focuser stop failed";
            return response;
        }
        case FocuserControlAction::GotoPosition: {
            std::string raw;
            const std::string command = ":FS" + std::to_string(request.target_position) + "#";
            const bool sent = sendCommand(command, raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "focuser goto accepted" : "focuser goto failed";
            return response;
        }
        case FocuserControlAction::Query:
        default: {
            std::string pos_raw;
            std::string status_raw;
            const bool pos_ok = sendCommand(":FG#", pos_raw);
            const bool status_ok = sendCommand(":FT#", status_raw);
            const auto position_state = pos_ok ? LX200Parser::parseFocuserState(pos_raw) : std::optional<FocuserState>{};
            const auto status_state = status_ok ? LX200Parser::parseFocuserState(status_raw) : std::optional<FocuserState>{};

            FocuserState combined;
            combined.valid = (position_state || status_state);
            if (position_state) {
                combined.position = position_state->position;
                combined.valid = true;
            }
            if (status_state) {
                combined.moving = status_state->moving;
                combined.busy = status_state->busy;
                combined.goto_rate = status_state->goto_rate;
                combined.valid = true;
            }
            response.focuser = combined;
            response.ok = combined.valid && (pos_ok || status_ok);
            response.error = response.ok ? MountError::None : MountError::InvalidResponse;
            response.message = response.ok ? "focuser state parsed" : "focuser unavailable";
            return response;
        }
    }
}

namespace {
MountFlipMirrorResponse makeFlipMirrorResponse(const std::string& raw, bool sent) {
    MountFlipMirrorResponse response;
    if (!sent) {
        response.error = MountError::Communication;
        response.message = "flip mirror command failed";
        return response;
    }

    auto flip = LX200Parser::parseFlipMirrorState(raw);
    if (flip) {
        response.flip_mirror = *flip;
        response.ok = true;
        response.error = MountError::None;
        response.message = "flip mirror parsed";
    } else {
        response.flip_mirror = FlipMirrorStatus{};
        response.ok = false;
        response.error = MountError::InvalidResponse;
        response.message = "flip mirror unavailable";
    }
    return response;
}
}  // namespace

MountFlipMirrorResponse OnStepMountClient::getFlipMirror(const MountFlipMirrorRequest& request) {
    MountFlipMirrorResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::FlipMirror);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "flip mirror capability unsupported";
        return response;
    }

    std::string raw;
    const bool sent = sendCommand(":FM?#", raw);
    response = makeFlipMirrorResponse(raw, sent);
    response.supported = true;
    return response;
}

MountFlipMirrorResponse OnStepMountClient::moveFlipMirror(const MountFlipMirrorRequest& request) {
    MountFlipMirrorResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::FlipMirror);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "flip mirror capability unsupported";
        return response;
    }

    if (request.position == FlipMirrorPosition::Unknown) {
        return getFlipMirror(request);
    }

    const std::string command = request.position == FlipMirrorPosition::Camera ? ":FM1#" : ":FM0#";
    std::string ignored;
    const bool sent = sendCommand(command, ignored);
    std::string query_raw;
    const bool query_sent = sendCommand(":FM?#", query_raw);
    response = makeFlipMirrorResponse(query_raw, sent && query_sent);
    response.supported = true;
    if (!response.ok) {
        response.message = "flip mirror move failed";
    } else {
        response.message = "flip mirror moved";
    }
    return response;
}

MountOrientationResponse OnStepMountClient::getOrientation(const MountOrientationRequest& request) {
    MountOrientationResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Orientation);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "orientation capability unsupported";
        return response;
    }
    std::string altitude_raw;
    std::string azimuth_raw;
    const bool alt_ok = sendCommand(":GA#", altitude_raw);
    const bool az_ok = sendCommand(":GZ#", azimuth_raw);
    const auto orientation = LX200Parser::parseOrientation(altitude_raw, azimuth_raw);
    response.orientation = orientation.value_or(MountOrientationInfo{});
    response.ok = alt_ok && az_ok && orientation.has_value();
    response.error = response.ok ? MountError::None : MountError::InvalidResponse;
    response.message = response.ok ? "orientation parsed" : "orientation unavailable";
    return response;
}

MountEnvironmentResponse OnStepMountClient::getEnvironment(const MountEnvironmentRequest& request) {
    MountEnvironmentResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Environmental);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "environment capability unsupported";
        return response;
    }
    std::string temperature_raw;
    std::string pressure_raw;
    std::string humidity_raw;
    std::string dew_point_raw;
    std::string mcu_temperature_raw;
    const bool temp_ok = sendCommand(":GX9A#", temperature_raw);
    const bool pressure_ok = sendCommand(":GX9B#", pressure_raw);
    const bool humidity_ok = sendCommand(":GX9C#", humidity_raw);
    const bool dew_ok = sendCommand(":GX9E#", dew_point_raw);
    const bool mcu_ok = sendCommand(":GX9F#", mcu_temperature_raw);
    const auto environment = LX200Parser::parseEnvironment(
        temperature_raw,
        pressure_raw,
        humidity_raw,
        dew_point_raw,
        mcu_temperature_raw);
    response.environment = environment.value_or(EnvironmentInfo{});
    response.ok = temp_ok && pressure_ok && humidity_ok && dew_ok && mcu_ok && environment.has_value();
    response.error = response.ok ? MountError::None : MountError::InvalidResponse;
    response.message = response.ok ? "environment parsed" : "environment unavailable";
    return response;
}

MountRotatorResponse OnStepMountClient::getRotator(const MountRotatorRequest& request) {
    MountRotatorResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Rotator);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "rotator capability unsupported";
        return response;
    }

    // Always query availability first if requested
    std::string availability_raw;
    std::string driver_raw;
    bool avail_ok = true;
    if (request.query_availability) {
        avail_ok = sendCommand(":GX98#", availability_raw);
    }

    // Basic query action: return availability and driver info
    if (request.action == RotatorControlAction::Query) {
        const bool driver_ok = request.query_driver_status ? sendCommand(":GXU3#", driver_raw) : true;
        const auto rotator = LX200Parser::parseRotatorStatus(availability_raw, driver_raw);
        response.rotator = rotator.value_or(RotatorStatus{});
        response.ok = avail_ok && driver_ok && rotator.has_value();
        response.error = response.ok ? MountError::None : MountError::InvalidResponse;
        response.message = response.ok ? "rotator parsed" : "rotator unavailable";
        if (response.ok) response.raw_response = response.rotator.raw;
        return response;
    }

    // Helper to format degrees->DMS string (e.g. +123*45:30)
    auto format_dms = [](double degrees)->std::string{
        bool negative = degrees < 0.0;
        double absd = std::abs(degrees);
        int deg = static_cast<int>(std::floor(absd));
        double rem = (absd - deg) * 60.0;
        int minutes = static_cast<int>(std::floor(rem));
        double seconds = (rem - minutes) * 60.0;
        std::ostringstream oss;
        oss << (negative ? '-' : '+') << deg << '*' << std::setw(2) << std::setfill('0') << minutes
            << ':' << std::setw(2) << std::setfill('0') << static_cast<int>(std::round(seconds));
        return oss.str();
    };

    std::string raw;
    bool sent = false;
    switch (request.action) {
        case RotatorControlAction::GetAngle: {
            sent = sendCommand(":rG#", raw);
            const auto angle = sent ? LX200Parser::parseRotatorAngle(raw) : std::optional<double>{};
            if (sent && angle) {
                response.rotator.current_angle_deg = *angle;
                response.rotator.valid = true;
                response.ok = true;
                response.error = MountError::None;
                response.message = "rotator angle parsed";
                response.raw_response = raw;
                return response;
            }
            response.ok = false;
            response.error = sent ? MountError::InvalidResponse : MountError::Communication;
            response.message = sent ? "rotator angle parse failed" : "rotator angle command failed";
            return response;
        }
        case RotatorControlAction::AbsoluteGoto: {
            const std::string dms = format_dms(request.target_angle_deg);
            const std::string cmd = std::string(":rS") + dms + "#";
            sent = sendCommand(cmd, raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator goto accepted" : "rotator goto failed";
            response.raw_response = raw;
            return response;
        }
        case RotatorControlAction::Relative: {
            const std::string dms = format_dms(request.relative_angle_deg);
            const std::string cmd = std::string(":rr") + dms + "#";
            sent = sendCommand(cmd, raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator relative accepted" : "rotator relative failed";
            response.raw_response = raw;
            return response;
        }
        case RotatorControlAction::ContinuousCW: {
            sent = sendCommand(":r>#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator move cw accepted" : "rotator move cw failed";
            return response;
        }
        case RotatorControlAction::ContinuousCCW: {
            sent = sendCommand(":r<#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator move ccw accepted" : "rotator move ccw failed";
            return response;
        }
        case RotatorControlAction::Stop: {
            sent = sendCommand(":rQ#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator stop accepted" : "rotator stop failed";
            return response;
        }
        case RotatorControlAction::SetZero: {
            sent = sendCommand(":rZ#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator zero set" : "rotator zero failed";
            return response;
        }
        case RotatorControlAction::SetHalfTravel: {
            sent = sendCommand(":rF#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator half-travel set" : "rotator half-travel failed";
            return response;
        }
        case RotatorControlAction::MoveHalfTravelOrHome: {
            sent = sendCommand(":rC#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator move to half/home accepted" : "rotator move to half/home failed";
            return response;
        }
        case RotatorControlAction::SetRate: {
            if (request.rate_index < 1 || request.rate_index > 9) {
                response.ok = false;
                response.error = MountError::InvalidRequest;
                response.message = "invalid rate index";
                return response;
            }
            std::string cmd = ":r" + std::to_string(request.rate_index) + "#";
            sent = sendCommand(cmd, raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator rate set" : "rotator rate failed";
            response.raw_response = raw;
            return response;
        }
        case RotatorControlAction::GetWorkingSlewRate: {
            sent = sendCommand(":rW#", raw);
            const auto rate = sent ? LX200Parser::parseSlewRate(raw) : std::optional<double>{};
            response.ok = sent && rate.has_value();
            response.error = response.ok ? MountError::None : (sent ? MountError::InvalidResponse : MountError::Communication);
            response.message = response.ok ? "rotator working slew parsed" : "rotator working slew unavailable";
            response.raw_response = raw;
            return response;
        }
        case RotatorControlAction::DerotateEnable: {
            sent = sendCommand(":r+#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator derotate enabled" : "rotator derotate enable failed";
            return response;
        }
        case RotatorControlAction::DerotateDisable: {
            sent = sendCommand(":r-#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator derotate disabled" : "rotator derotate disable failed";
            return response;
        }
        case RotatorControlAction::DerotateReverse: {
            sent = sendCommand(":rR#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator derotate reversed" : "rotator derotate reverse failed";
            return response;
        }
        case RotatorControlAction::Park: {
            sent = sendCommand(":hP#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator parked" : "rotator park failed";
            return response;
        }
        case RotatorControlAction::Unpark: {
            sent = sendCommand(":hR#", raw);
            response.ok = sent;
            response.error = sent ? MountError::None : MountError::Communication;
            response.message = sent ? "rotator unparked" : "rotator unpark failed";
            return response;
        }
        default: {
            response.ok = false;
            response.error = MountError::NotImplemented;
            response.message = "rotator action not implemented";
            return response;
        }
    }
    return response;
}

MountMode OnStepMountClient::queryMountMode() {
    std::string raw;
    if (!sendCommand(":GXM#", raw)) {
        return detectedMode;
    }

    const auto mode = LX200Parser::parseMountMode(raw);
    if (!mode.has_value()) {
        return detectedMode;
    }
    return *mode;
}

MountMode OnStepMountClient::getDetectedMountMode() const {
    return detectedMode;
}

void OnStepMountClient::updateDetectedMountMode() {
    std::string raw;
    if (!sendCommand(":GXM#", raw)) {
        return;
    }

    const auto mode = LX200Parser::parseMountMode(raw);
    if (mode.has_value()) {
        detectedMode = *mode;
    }
}

MountModeResponse OnStepMountClient::getMountMode(const MountModeRequest& request) {
    MountModeResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::MountMode);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "mount mode capability unsupported";
        return response;
    }

    std::string raw;
    if (!sendCommand(":GXM#", raw)) {
        response.error = MountError::Communication;
        response.message = "failed to query firmware for mount mode";
        return response;
    }
    const auto mode = LX200Parser::parseMountMode(raw);
    if (!mode.has_value()) {
        response.error = MountError::InvalidResponse;
        response.message = "invalid mount mode response";
        return response;
    }
    response.mode = *mode;
    response.ok = true;
    response.raw_response = raw;
    response.message = "mount mode parsed";
    return response;
}

MountModeResponse OnStepMountClient::setMountMode(const MountModeRequest& request) {
    MountModeResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::MountMode);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "mount mode capability unsupported";
        return response;
    }
    if (!request.mode.has_value()) {
        response.error = MountError::InvalidRequest;
        response.message = "no mount mode specified";
        return response;
    }

    const MountMode requested_mode = request.mode.value();

    if (!request.apply) {
        response.ok = true;
        response.mode = requested_mode;
        response.message = "mount mode selected; firmware rebuild and upload required";
        return response;
    }

    if (!connect()) {
        response.error = MountError::Communication;
        response.message = "failed to reconnect after firmware upload";
        return response;
    }

    updateDetectedMountMode();
    response.ok = detectedMode == requested_mode;
    response.mode = detectedMode;
    response.message = response.ok ? "mount mode verified after firmware upload" : "firmware mismatch after upload";
    return response;
}

MountAlignmentResponse OnStepMountClient::align(const MountAlignmentRequest& request) {
    MountAlignmentResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Alignment);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "alignment capability unsupported";
        return response;
    }

    std::string raw;
    std::string command;
    switch (request.mode) {
        case MountAlignmentMode::AcceptStar:
            command = ":A+#";
            break;
        case MountAlignmentMode::SaveAlignment:
            command = ":AW#";
            break;
        case MountAlignmentMode::Manual:
        default:
            command = ":A1#";
            break;
    }

    const bool sent = sendCommand(command, raw);
    response.ok = sent;
    response.aligned = sent;
    response.error = sent ? MountError::None : MountError::Communication;
    response.message = sent ? "alignment command accepted" : "alignment command failed";
    return response;
}


MountGuideRateResponse OnStepMountClient::setGuideRate(const MountGuideRateRequest& request) {
    MountGuideRateResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::GuideRate);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "guide rate capability unsupported";
        return response;
    }
    std::string raw;
    const std::string command = formatGuideRate(request.preset);
    const bool sent = sendCommand(command, raw);
    response.ok = sent;
    response.rate = request.rate;
    if (!sent) {
        response.error = MountError::Communication;
        response.message = "guide rate command failed";
        return response;
    }
    if (request.readback) {
        std::string readback_raw;
        const bool readback = sendCommand(":GX90#", readback_raw);
        if (!readback) {
            response.ok = false;
            response.error = MountError::Communication;
            response.message = "guide rate readback unavailable";
            return response;
        }
        const auto parsed = LX200Parser::parseGuideRate(readback_raw);
        if (!parsed) {
            response.ok = false;
            response.error = MountError::InvalidResponse;
            response.message = "guide rate readback invalid";
            return response;
        }
        response.rate = *parsed;
    }
    response.error = MountError::None;
    response.message = "guide rate command accepted";
    return response;
}

MountSlewRateResponse OnStepMountClient::setSlewRate(const MountSlewRateRequest& request) {
    MountSlewRateResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::SlewRate);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "slew rate capability unsupported";
        return response;
    }
    std::string raw;
    const std::string command = formatSlewRate(request.preset);
    const bool sent = sendCommand(command, raw);
    response.ok = sent;
    response.rate = request.rate;
    if (!sent) {
        response.error = MountError::Communication;
        response.message = "slew rate command failed";
        return response;
    }
    if (request.readback) {
        std::string readback_raw;
        const bool readback = sendCommand(":GX93#", readback_raw);
        if (!readback) {
            response.ok = false;
            response.error = MountError::Communication;
            response.message = "slew rate readback unavailable";
            return response;
        }
        const auto parsed = LX200Parser::parseSlewRate(readback_raw);
        if (!parsed) {
            response.ok = false;
            response.error = MountError::InvalidResponse;
            response.message = "slew rate readback invalid";
            return response;
        }
        response.rate = *parsed;
    }
    response.error = MountError::None;
    response.message = "slew rate command accepted";
    return response;
}

MountManualMotionResponse OnStepMountClient::manualMotion(const MountManualMotionRequest& request) {
    MountManualMotionResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Motion);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "manual motion capability unsupported";
        return response;
    }
    std::string raw;
    const std::string command = formatManualCommand(request.axis, request.start);
    const bool sent = sendCommand(command, raw);
    response.ok = sent;
    response.active = sent && request.start;
    response.error = sent ? MountError::None : MountError::Communication;
    response.message = sent ? "manual motion command accepted" : "manual motion command failed";
    return response;
}

MountAuxResponse OnStepMountClient::controlAux(const MountAuxRequest& request) {
    MountAuxResponse response;
    response.supported = isCapabilitySupported(MountCapabilityId::Auxiliary);
    if (!response.supported) {
        response.error = MountError::Unsupported;
        response.message = "auxiliary capability unsupported";
        return response;
    }
    std::string raw;
    const std::string command = formatAuxCommand(request);
    const bool sent = sendCommand(command, raw);
    if (!sent) {
        response.error = MountError::Communication;
        response.message = "auxiliary command failed";
        return response;
    }
    response.ok = true;
    response.buzzer_enabled = request.enable_buzzer;
    response.busy = raw == "1";
    response.distance_bar = raw.empty() ? 0 : std::atoi(raw.c_str());
    response.error = MountError::None;
    response.message = "auxiliary command accepted";
    return response;
}

bool OnStepMountClient::sendCommand(const std::string& command, std::string& response_out) const {
    if (connection_manager_) {
        return connection_manager_->sendCommand(command, response_out);
    }
    if (!transport_) {
        return false;
    }
    return transport_->send(command, response_out);
}

bool OnStepMountClient::sendRawCommand(const std::string& command, std::string& response_out) {
    return sendCommand(command, response_out);
}

} // namespace hardware
} // namespace asdevlab
