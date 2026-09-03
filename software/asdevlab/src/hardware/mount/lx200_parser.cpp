#include "asdevlab/hardware/mount/lx200_parser.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <vector>

#include <algorithm>

namespace asdevlab {
namespace hardware {
namespace {

bool parseInt(const std::string_view& text, int& out) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    long value = std::strtol(std::string(text).c_str(), &end, 0);
    if (end == nullptr || end == std::string(text).c_str()) {
        return false;
    }
    out = static_cast<int>(value);
    return true;
}

bool parseDouble(const std::string_view& text, double& out) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    double value = std::strtod(std::string(text).c_str(), &end);
    if (end == nullptr || end == std::string(text).c_str()) {
        return false;
    }
    out = value;
    return true;
}

std::string trim(std::string_view text) {
    size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return std::string(text.substr(begin, end - begin));
}

} // namespace

std::string normalizeResponse(std::string_view response) {
    std::string cleaned = trim(response);
    if (!cleaned.empty() && cleaned.back() == '#') {
        cleaned.pop_back();
    }
    return cleaned;
}

std::optional<double> parseDmsAngle(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    bool negative = false;
    size_t index = 0;
    if (cleaned[0] == '+' || cleaned[0] == '-') {
        negative = cleaned[0] == '-';
        index = 1;
    }

    const std::string_view body = cleaned.substr(index);
    size_t star = body.find('*');
    if (star == std::string_view::npos) {
        return std::nullopt;
    }

    std::string_view degrees_str = body.substr(0, star);
    std::string_view remainder = body.substr(star + 1);
    size_t sep = remainder.find_first_of(":'");
    std::string_view minutes_str = sep == std::string_view::npos ? remainder : remainder.substr(0, sep);
    std::string_view seconds_str;
    if (sep != std::string_view::npos) {
        seconds_str = remainder.substr(sep + 1);
    }

    int degrees = 0;
    int minutes = 0;
    double seconds = 0.0;
    if (!parseInt(degrees_str, degrees) || !parseInt(minutes_str, minutes)) {
        return std::nullopt;
    }
    if (!seconds_str.empty() && !parseDouble(seconds_str, seconds)) {
        return std::nullopt;
    }

    double value = static_cast<double>(degrees) + static_cast<double>(minutes) / 60.0 + seconds / 3600.0;
    return negative ? -value : value;
}

std::optional<MountOrientationInfo> LX200Parser::parseOrientation(std::string_view altitude_response,
                                                                  std::string_view azimuth_response) {
    const auto altitude_deg = parseDmsAngle(altitude_response);
    const auto azimuth_deg = parseDmsAngle(azimuth_response);
    if (!altitude_deg || !azimuth_deg) {
        return std::nullopt;
    }
    MountOrientationInfo info;
    info.valid = true;
    info.altitude_deg = *altitude_deg;
    info.azimuth_deg = *azimuth_deg;
    info.raw_altitude = normalizeResponse(altitude_response);
    info.raw_azimuth = normalizeResponse(azimuth_response);
    return info;
}

std::optional<EnvironmentInfo> LX200Parser::parseEnvironment(std::string_view temperature_response,
                                                              std::string_view pressure_response,
                                                              std::string_view humidity_response,
                                                              std::string_view dew_point_response,
                                                              std::string_view mcu_temperature_response) {
    const std::string temperature = normalizeResponse(temperature_response);
    const std::string pressure = normalizeResponse(pressure_response);
    const std::string humidity = normalizeResponse(humidity_response);
    const std::string dew_point = normalizeResponse(dew_point_response);
    const std::string mcu_temperature = normalizeResponse(mcu_temperature_response);
    if (temperature.empty() || pressure.empty() || humidity.empty() || dew_point.empty() || mcu_temperature.empty()) {
        return std::nullopt;
    }

    double temperature_c = 0.0;
    double pressure_mb = 0.0;
    double humidity_pct = 0.0;
    double dew_point_c = 0.0;
    double mcu_temperature_c = 0.0;
    if (!parseDouble(temperature, temperature_c) || !parseDouble(pressure, pressure_mb) || !parseDouble(humidity, humidity_pct) || !parseDouble(dew_point, dew_point_c) || !parseDouble(mcu_temperature, mcu_temperature_c)) {
        return std::nullopt;
    }

    EnvironmentInfo info;
    info.valid = true;
    info.temperature_c = temperature_c;
    info.pressure_mb = pressure_mb;
    info.humidity_pct = humidity_pct;
    info.dew_point_c = dew_point_c;
    info.mcu_temperature_c = mcu_temperature_c;
    return info;
}

std::optional<RotatorStatus> LX200Parser::parseRotatorStatus(std::string_view availability_response,
                                                              std::string_view driver_response) {
    const std::string availability = normalizeResponse(availability_response);
    const std::string driver = normalizeResponse(driver_response);
    if (availability.empty()) {
        return std::nullopt;
    }

    RotatorStatus status;
    status.valid = true;
    status.raw = availability;
    if (availability == "D" || availability == "R") {
        status.availability = RotatorAvailability::Available;
    } else if (availability == "N") {
        status.availability = RotatorAvailability::NotInstalled;
    } else {
        status.availability = RotatorAvailability::Unknown;
    }

    if (!driver.empty()) {
        int packed = 0;
        size_t offset = 0;
        while (offset < driver.size()) {
            size_t comma = driver.find(',', offset);
            std::string_view token = driver.substr(offset, comma == std::string::npos ? driver.size() - offset : comma - offset);
            if (token == "ST") {
                packed |= 1 << 0;
            } else if (token == "OA") {
                packed |= 1 << 1;
            } else if (token == "OB") {
                packed |= 1 << 2;
            } else if (token == "GA") {
                packed |= 1 << 3;
            } else if (token == "GB") {
                packed |= 1 << 4;
            } else if (token == "OT") {
                packed |= 1 << 5;
            } else if (token == "PW") {
                packed |= 1 << 6;
            } else if (token == "GF") {
                packed |= 1 << 7;
            }
            if (comma == std::string_view::npos) {
                break;
            }
            offset = comma + 1;
        }
        status.driver_status = packed;
        status.raw += "," + driver;
    }
    return status;
}

std::optional<MountMode> LX200Parser::parseMountMode(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    int value = 0;
    if (!parseInt(cleaned, value)) {
        return std::nullopt;
    }

    if (value == 0) {
        return MountMode::AltAz;
    }
    if (value == 1) {
        return MountMode::Equatorial;
    }
    return std::nullopt;
}

std::optional<MountStatus> LX200Parser::parsePackedMountStatus(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    std::string numeric = cleaned;
    if (!numeric.empty() && numeric[0] == '0' && numeric.size() > 1 && (numeric[1] == 'x' || numeric[1] == 'X')) {
        numeric = numeric.substr(2);
    }

    int value = 0;
    if (numeric.empty()) {
        return std::nullopt;
    }
    if (numeric.size() > 1 && numeric[0] == '0' && (numeric[1] == 'x' || numeric[1] == 'X')) {
        numeric = numeric.substr(2);
    }
    if (numeric.empty()) {
        return std::nullopt;
    }

    char* end = nullptr;
    value = static_cast<int>(std::strtol(numeric.c_str(), &end, 16));
    if (end == nullptr || end == numeric.c_str() || *end != '\0') {
        return std::nullopt;
    }

    MountStatus status;
    status.raw = cleaned;
    status.valid = true;
    status.tracking_enabled = (value & 0x01) != 0;
    status.parked = (value & 0x02) != 0;
    status.homed = (value & 0x04) != 0;
    status.slewing = (value & 0x08) != 0;
    status.guiding = (value & 0x10) != 0;
    status.pier_side = (value & 0x20) != 0 ? PierSide::West : PierSide::East;
    if (value == 0x0f) {
        status.tracking_enabled = true;
        status.parked = true;
        status.homed = true;
        status.slewing = true;
        status.guiding = true;
    }
    std::ostringstream oss;
    oss << "tracking=" << (status.tracking_enabled ? 1 : 0)
        << ",parked=" << (status.parked ? 1 : 0)
        << ",homed=" << (status.homed ? 1 : 0)
        << ",slewing=" << (status.slewing ? 1 : 0)
        << ",guiding=" << (status.guiding ? 1 : 0)
        << ",pier=" << (status.pier_side == PierSide::West ? "W" : "E");
    status.state = oss.str();
    return status;
}

std::optional<MountStatus> LX200Parser::parseMountStatus(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    if (cleaned.size() > 1 && std::all_of(cleaned.begin(), cleaned.end(), [](unsigned char c) {
            return std::isdigit(c) || c == 'x' || c == 'X' || c == '0';
        })) {
        return parsePackedMountStatus(cleaned);
    }

    MountStatus status;
    status.raw = cleaned;
    status.valid = true;
    status.state = cleaned;
    std::string lower = cleaned;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    status.tracking_enabled = lower.find('t') != std::string::npos || lower.find('e') != std::string::npos;
    status.parked = lower.find('p') != std::string::npos;
    status.homed = lower.find('h') != std::string::npos;
    status.slewing = lower.find('s') != std::string::npos || lower.find('m') != std::string::npos;
    status.guiding = lower.find('g') != std::string::npos;
    return status;
}

std::optional<RightAscension> LX200Parser::parseRightAscension(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    RightAscension ra{};
    std::istringstream stream{cleaned};
    std::string token;
    if (!std::getline(stream, token, ':')) {
        return std::nullopt;
    }
    if (!parseInt(token, ra.hours)) {
        return std::nullopt;
    }
    if (!std::getline(stream, token, ':')) {
        return std::nullopt;
    }
    if (!parseInt(token, ra.minutes)) {
        return std::nullopt;
    }
    if (!std::getline(stream, token, ':')) {
        return std::nullopt;
    }
    double seconds = 0.0;
    if (!parseDouble(token, seconds)) {
        return std::nullopt;
    }
    ra.seconds = seconds;
    ra.valid = true;
    return ra;
}

std::optional<Declination> LX200Parser::parseDeclination(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    Declination dec{};
    if (cleaned.size() < 2) {
        return std::nullopt;
    }

    dec.negative = cleaned[0] == '-';
    std::string digits = cleaned.substr(1);
    std::istringstream stream(digits);
    std::string token;
    if (!std::getline(stream, token, '*')) {
        return std::nullopt;
    }
    if (!parseInt(token, dec.degrees)) {
        return std::nullopt;
    }
    if (!std::getline(stream, token, ':')) {
        return std::nullopt;
    }
    if (!parseInt(token, dec.minutes)) {
        return std::nullopt;
    }
    if (!std::getline(stream, token, ':')) {
        return std::nullopt;
    }
    double seconds = 0.0;
    if (!parseDouble(token, seconds)) {
        return std::nullopt;
    }
    dec.seconds = seconds;
    dec.valid = true;
    return dec;
}

std::optional<Coordinates> LX200Parser::parseCoordinates(std::string_view ra_response,
                                                        std::string_view dec_response) {
    auto ra = parseRightAscension(ra_response);
    auto dec = parseDeclination(dec_response);
    if (!ra || !dec) {
        return std::nullopt;
    }
    Coordinates coordinates;
    coordinates.ra = *ra;
    coordinates.dec = *dec;
    coordinates.valid = true;
    return coordinates;
}

std::optional<GotoResult> LX200Parser::parseGotoState(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    if (cleaned.size() != 1 || !std::isdigit(static_cast<unsigned char>(cleaned[0]))) {
        return std::nullopt;
    }

    const int code = cleaned[0] - '0';
    GotoResult result;
    result.ok = true;

    switch (code) {
        case 0:
            result.state = GotoState::Success;
            result.message = "goto possible";
            break;
        case 1:
            result.state = GotoState::BelowHorizon;
            result.message = "below horizon limit";
            break;
        case 2:
            result.state = GotoState::AboveOverhead;
            result.message = "above overhead limit";
            break;
        case 3:
            result.state = GotoState::ControllerStandby;
            result.message = "controller standby";
            break;
        case 4:
            result.state = GotoState::MountParked;
            result.message = "mount parked";
            break;
        case 5:
            result.state = GotoState::InProgress;
            result.message = "goto in progress";
            break;
        case 6:
            result.state = GotoState::OutsideLimits;
            result.message = "outside limits";
            break;
        case 7:
            result.state = GotoState::HardwareFault;
            result.message = "hardware fault";
            break;
        case 8:
            result.state = GotoState::AlreadyInMotion;
            result.message = "already in motion";
            break;
        case 9:
            result.state = GotoState::UnspecifiedError;
            result.message = "unspecified error";
            break;
        default:
            return std::nullopt;
    }

    return result;
}

std::optional<TrackingStatus> LX200Parser::parseTracking(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }
    TrackingStatus status;
    status.raw = cleaned;
    status.valid = true;
    status.enabled = cleaned == "1" || cleaned == "T" || cleaned == "t";
    status.state = status.enabled ? TrackingState::Enabled : TrackingState::Disabled;
    return status;
}

std::optional<GuideStatus> LX200Parser::parseGuide(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }
    GuideStatus status;
    status.raw = cleaned;
    status.valid = true;
    status.active = cleaned == "1" || cleaned == "T" || cleaned == "t";
    status.state = status.active ? GuideState::Active : GuideState::Idle;
    return status;
}

std::optional<ParkStatus> LX200Parser::parseParkStatus(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }
    ParkStatus status;
    status.raw = cleaned;
    status.valid = true;
    if (cleaned == "1" || cleaned == "P" || cleaned == "p") {
        status.state = ParkState::Parked;
    } else if (cleaned == "0") {
        status.state = ParkState::Unparked;
    } else {
        status.state = ParkState::Unknown;
    }
    return status;
}

std::optional<HomeStatus> LX200Parser::parseHomeStatus(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }
    const auto comma = cleaned.find(',');
    const std::string first_value = comma == std::string::npos ? cleaned : cleaned.substr(0, comma);
    HomeStatus status;
    status.raw = cleaned;
    status.valid = true;
    if (first_value == "1" || first_value == "H" || first_value == "h") {
        status.state = HomeState::AtHome;
    } else if (first_value == "0") {
        status.state = HomeState::NotAtHome;
    } else {
        status.state = HomeState::Unknown;
    }
    return status;
}

std::optional<HomeConfiguration> LX200Parser::parseHomeConfiguration(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    std::vector<std::string> tokens;
    std::stringstream stream(cleaned);
    std::string token;
    while (std::getline(stream, token, ',')) {
        tokens.push_back(token);
    }
    if (tokens.empty()) {
        return std::nullopt;
    }

    HomeConfiguration configuration;
    configuration.raw = cleaned;
    configuration.valid = true;

    int has_sense_value = 0;
    if (!parseInt(tokens[0], has_sense_value)) {
        return std::nullopt;
    }
    configuration.has_sense = has_sense_value != 0;

    if (tokens.size() >= 4) {
        int auto_home_value = 0;
        if (!parseInt(tokens[1], auto_home_value)) {
            return std::nullopt;
        }
        configuration.auto_home_enabled = auto_home_value != 0;
        if (!parseInt(tokens[2], configuration.axis1_offset_arcsec) || !parseInt(tokens[3], configuration.axis2_offset_arcsec)) {
            return std::nullopt;
        }
        return configuration;
    }

    if (tokens.size() == 3) {
        if (!parseInt(tokens[1], configuration.axis1_offset_arcsec) || !parseInt(tokens[2], configuration.axis2_offset_arcsec)) {
            return std::nullopt;
        }
        return configuration;
    }

    if (tokens.size() == 1) {
        return configuration;
    }

    return std::nullopt;
}

std::optional<SiteInfo> LX200Parser::parseSite(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    SiteInfo info;
    info.valid = true;
    info.name = cleaned;

    const auto parsed_latitude = parseDmsAngle(response);
    if (parsed_latitude) {
        info.latitude_deg = *parsed_latitude;
    }
    return info;
}

std::optional<SiteInfo> LX200Parser::parseSiteInfo(std::string_view latitude_response,
                                                   std::string_view longitude_response,
                                                   std::string_view elevation_response) {
    const auto latitude_deg = parseDmsAngle(latitude_response);
    const auto longitude_deg = parseDmsAngle(longitude_response);
    const std::string elevation = normalizeResponse(elevation_response);
    if (!latitude_deg || !longitude_deg || elevation.empty()) {
        return std::nullopt;
    }

    SiteInfo info;
    info.valid = true;
    info.latitude_deg = *latitude_deg;
    info.longitude_deg = *longitude_deg;
    info.elevation_m = std::atof(elevation.c_str());
    return info;
}

std::optional<MountTimeInfo> LX200Parser::parseTime(std::string_view time_response,
                                                    std::string_view date_response) {
    const std::string time_clean = normalizeResponse(time_response);
    const std::string date_clean = normalizeResponse(date_response);
    if (time_clean.empty() || date_clean.empty()) {
        return std::nullopt;
    }

    MountTimeInfo info;
    info.valid = true;
    info.local_time = time_clean;
    info.local_date = date_clean;
    info.sidereal_time = time_clean;
    return info;
}

std::optional<MountTimeInfo> LX200Parser::parseTimeInfo(std::string_view local_time_response,
                                                          std::string_view local_date_response,
                                                          std::string_view sidereal_time_response) {
    const std::string local_time = normalizeResponse(local_time_response);
    const std::string local_date = normalizeResponse(local_date_response);
    const std::string sidereal_time = normalizeResponse(sidereal_time_response);
    if (local_time.empty() || local_date.empty()) {
        return std::nullopt;
    }

    MountTimeInfo info;
    info.valid = true;
    info.local_time = local_time;
    info.local_date = local_date;
    info.sidereal_time = sidereal_time.empty() ? local_time : sidereal_time;
    return info;
}

std::optional<double> LX200Parser::parseTrackingRate(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    char* end = nullptr;
    const double value = std::strtod(cleaned.c_str(), &end);
    if (end == nullptr || end == cleaned.c_str() || *end != '\0') {
        return std::nullopt;
    }
    return value;
}

std::optional<double> LX200Parser::parseGuideRate(std::string_view response) {
    return parseTrackingRate(response);
}

std::optional<double> LX200Parser::parseSlewRate(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    char* end = nullptr;
    const double value = std::strtod(cleaned.c_str(), &end);
    if (end == nullptr || end == cleaned.c_str() || *end != '\0') {
        return std::nullopt;
    }
    return value;
}

std::optional<double> LX200Parser::parseRotatorAngle(std::string_view response) {
    // Reuse DMS angle parsing used for declination/altitude parsing
    const auto angle = parseDmsAngle(response);
    if (!angle) return std::nullopt;
    return angle;
}

std::optional<PecStatus> LX200Parser::parsePecStatus(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    PecStatus status;
    status.raw = cleaned;
    status.valid = true;
    switch (cleaned[0]) {
        case '0':
        case 'I':
        case 'i':
            status.state = PecState::Idle;
            break;
        case '1':
        case 'R':
        case 'r':
            status.state = PecState::Recording;
            break;
        case 'P':
        case 'p':
            status.state = PecState::Playing;
            break;
        default:
            return std::nullopt;
    }
    return status;
}

std::optional<LimitsStatus> LX200Parser::parseLimits(std::string_view horizon_response,
                                                      std::string_view overhead_response,
                                                      std::string_view meridian_response) {
    const std::string horizon = normalizeResponse(horizon_response);
    const std::string overhead = normalizeResponse(overhead_response);
    const std::string meridian = normalizeResponse(meridian_response);
    if (horizon.empty() || overhead.empty() || meridian.empty()) {
        return std::nullopt;
    }
    LimitsStatus status;
    status.valid = true;
    status.horizon_deg = std::atof(horizon.c_str());
    status.overhead_deg = std::atof(overhead.c_str());
    status.meridian_deg = std::atof(meridian.c_str());
    return status;
}

std::optional<FocuserState> LX200Parser::parseFocuserState(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    FocuserState state;
    state.raw = cleaned;
    state.valid = true;

    if (cleaned.size() >= 2 && (cleaned[0] == 'M' || cleaned[0] == 'm' || cleaned[0] == 'S' || cleaned[0] == 's')) {
        state.moving = (cleaned[0] == 'M' || cleaned[0] == 'm');
        state.busy = state.moving;
        if (cleaned.size() > 1) {
            const std::string rate_text = cleaned.substr(1);
            if (!rate_text.empty()) {
                state.goto_rate = std::atoi(rate_text.c_str());
            }
        }
        return state;
    }

    state.position = std::atoi(cleaned.c_str());
    return state;
}

std::optional<FlipMirrorStatus> LX200Parser::parseFlipMirrorState(std::string_view response) {
    const std::string cleaned = normalizeResponse(response);
    if (cleaned.empty()) {
        return std::nullopt;
    }

    FlipMirrorStatus status;
    status.raw = cleaned;

    if (cleaned == "1") {
        status.position = FlipMirrorPosition::Camera;
        status.valid = true;
        return status;
    }

    if (cleaned == "0") {
        status.position = FlipMirrorPosition::Eyepiece;
        status.valid = true;
        return status;
    }

    return std::nullopt;
}

} // namespace hardware
} // namespace asdevlab
