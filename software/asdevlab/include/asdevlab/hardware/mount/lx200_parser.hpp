#pragma once

#include "asdevlab/hardware/mount/mount_mode.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace asdevlab {
namespace hardware {

// Parsed LX200 right-ascension value in hours/minutes/seconds.
struct RightAscension {
    int hours = 0;
    int minutes = 0;
    double seconds = 0.0;
    bool valid = false;
};

struct Declination {
    int degrees = 0;
    int minutes = 0;
    double seconds = 0.0;
    bool negative = false;
    bool valid = false;
};

struct Coordinates {
    RightAscension ra;
    Declination dec;
    bool valid = false;
};

enum class PierSide {
    Unknown,
    East,
    West
};

struct MountStatus {
    std::string raw;
    bool valid = false;
    bool tracking_enabled = false;
    bool parked = false;
    bool homed = false;
    bool slewing = false;
    bool guiding = false;
    PierSide pier_side = PierSide::Unknown;
    std::string state;
};

enum class GotoState {
    Success,
    InProgress,
    BelowHorizon,
    AboveOverhead,
    ControllerStandby,
    MountParked,
    OutsideLimits,
    HardwareFault,
    AlreadyInMotion,
    UnspecifiedError,
    CommunicationError
};

struct GotoResult {
    bool ok = false;
    GotoState state = GotoState::CommunicationError;
    std::string message;
};

enum class TrackingState {
    Unknown,
    Enabled,
    Disabled
};

struct TrackingStatus {
    bool valid = false;
    bool enabled = false;
    TrackingState state = TrackingState::Unknown;
    std::string raw;
};

enum class GuideState {
    Idle,
    Active
};

struct GuideStatus {
    bool valid = false;
    bool active = false;
    GuideState state = GuideState::Idle;
    std::string raw;
};

enum class ParkState {
    Unknown,
    Parked,
    Unparked,
    Parking,
    Unparking
};

struct ParkStatus {
    bool valid = false;
    ParkState state = ParkState::Unknown;
    std::string raw;
};

enum class HomeState {
    Unknown,
    AtHome,
    NotAtHome
};

struct HomeStatus {
    bool valid = false;
    HomeState state = HomeState::Unknown;
    std::string raw;
};

struct HomeConfiguration {
    bool valid = false;
    bool has_sense = false;
    bool auto_home_enabled = false;
    int axis1_offset_arcsec = 0;
    int axis2_offset_arcsec = 0;
    std::string raw;
};

struct SiteInfo {
    bool valid = false;
    double latitude_deg = 0.0;
    double longitude_deg = 0.0;
    double elevation_m = 0.0;
    std::string name;
};

struct MountTimeInfo {
    bool valid = false;
    std::string local_time;
    std::string local_date;
    std::string sidereal_time;
};

enum class PecState {
    Unknown,
    Idle,
    Recording,
    Playing
};

struct PecStatus {
    bool valid = false;
    PecState state = PecState::Unknown;
    std::string raw;
};

struct LimitsStatus {
    bool valid = false;
    double horizon_deg = 0.0;
    double overhead_deg = 0.0;
    double meridian_deg = 0.0;
};

struct FocuserState {
    bool valid = false;
    int position = 0;
    bool moving = false;
    bool busy = false;
    int goto_rate = 0;
    std::string raw;
};

enum class FlipMirrorState {
    Unknown,
    Camera,
    Eyepiece
};

using FlipMirrorPosition = FlipMirrorState;

struct FlipMirrorStatus {
    bool valid = false;
    FlipMirrorPosition position = FlipMirrorPosition::Unknown;
    std::string raw;
};

struct MountOrientationInfo {
    bool valid = false;
    double altitude_deg = 0.0;
    double azimuth_deg = 0.0;
    std::string raw_altitude;
    std::string raw_azimuth;
};

struct EnvironmentInfo {
    bool valid = false;
    double temperature_c = 0.0;
    double pressure_mb = 0.0;
    double humidity_pct = 0.0;
    double dew_point_c = 0.0;
    double mcu_temperature_c = 0.0;
};

enum class RotatorAvailability {
    Unknown,
    Available,
    Unavailable,
    NotInstalled
};

struct RotatorStatus {
    bool valid = false;
    RotatorAvailability availability = RotatorAvailability::Unknown;
    std::optional<int> driver_status;
    std::optional<double> current_angle_deg;
    std::string raw;
};

// parse DMS angle returned by `:rG#`
class LX200Parser;

// Parser for converting raw LX200 responses into typed mount structures.
class LX200Parser {
public:
    static std::optional<MountStatus> parseMountStatus(std::string_view response);
    static std::optional<MountStatus> parsePackedMountStatus(std::string_view response);
    static std::optional<RightAscension> parseRightAscension(std::string_view response);
    static std::optional<Declination> parseDeclination(std::string_view response);
    static std::optional<Coordinates> parseCoordinates(std::string_view ra_response,
                                                      std::string_view dec_response);
    static std::optional<GotoResult> parseGotoState(std::string_view response);
    static std::optional<TrackingStatus> parseTracking(std::string_view response);
    static std::optional<GuideStatus> parseGuide(std::string_view response);
    static std::optional<ParkStatus> parseParkStatus(std::string_view response);
    static std::optional<HomeStatus> parseHomeStatus(std::string_view response);
    static std::optional<HomeConfiguration> parseHomeConfiguration(std::string_view response);
    static std::optional<SiteInfo> parseSite(std::string_view response);
    static std::optional<SiteInfo> parseSiteInfo(std::string_view latitude_response,
                                                 std::string_view longitude_response,
                                                 std::string_view elevation_response);
    static std::optional<MountTimeInfo> parseTime(std::string_view time_response,
                                                   std::string_view date_response);
    static std::optional<MountTimeInfo> parseTimeInfo(std::string_view local_time_response,
                                                      std::string_view local_date_response,
                                                      std::string_view sidereal_time_response);
    static std::optional<double> parseTrackingRate(std::string_view response);
    static std::optional<double> parseGuideRate(std::string_view response);
    static std::optional<double> parseSlewRate(std::string_view response);
    static std::optional<double> parseRotatorAngle(std::string_view response);
    static std::optional<MountOrientationInfo> parseOrientation(std::string_view altitude_response,
                                                               std::string_view azimuth_response);
    static std::optional<EnvironmentInfo> parseEnvironment(std::string_view temperature_response,
                                                           std::string_view pressure_response,
                                                           std::string_view humidity_response,
                                                           std::string_view dew_point_response,
                                                           std::string_view mcu_temperature_response);
    static std::optional<RotatorStatus> parseRotatorStatus(std::string_view availability_response,
                                                           std::string_view driver_response);
    static std::optional<MountMode> parseMountMode(std::string_view response);
    static std::optional<PecStatus> parsePecStatus(std::string_view response);
    static std::optional<LimitsStatus> parseLimits(std::string_view horizon_response,
                                                   std::string_view overhead_response,
                                                   std::string_view meridian_response);
    static std::optional<FocuserState> parseFocuserState(std::string_view response);
    static std::optional<FlipMirrorStatus> parseFlipMirrorState(std::string_view response);
};

} // namespace hardware
} // namespace asdevlab
