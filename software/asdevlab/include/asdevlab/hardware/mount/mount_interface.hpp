#pragma once

#include "asdevlab/hardware/mount/lx200_parser.hpp"
#include "asdevlab/hardware/mount/mount_mode.hpp"

#include <optional>
#include <string>
#include <vector>

namespace asdevlab {
namespace hardware {

// High-level mount errors returned by the typed mount API.
enum class MountError {
    None,
    Unsupported,
    Communication,
    InvalidResponse,
    InvalidRequest,
    NotImplemented
};

enum class MountCapabilityId {
    Status,
    Coordinates,
    Slew,
    Tracking,
    Guide,
    Motion,
    Park,
    Home,
    Alignment,
    Site,
    Time,
    Pec,
    Limits,
    Focuser,
    FlipMirror,
    TrackingRate,
    GuideRate,
    SlewRate,
    Auxiliary,
    Orientation,
    Environmental,
    Rotator,
    MountMode
};

// Describes whether a mount feature is supported by the current backend.
struct MountCapability {
    MountCapabilityId id = MountCapabilityId::Status;
    bool supported = false;
    std::string description;
};

struct MountResponseBase {
    bool ok = false;
    bool supported = true;
    MountError error = MountError::None;
    std::string message;
    std::string raw_response;
};

struct MountStatusRequest {
    bool include_raw = true;
};

struct MountStatusResponse : MountResponseBase {
    MountStatus status;
};

struct MountCoordinatesRequest {
    bool include_raw = true;
};

struct MountCoordinatesResponse : MountResponseBase {
    Coordinates coordinates;
};

enum class SlewMode {
    Goto,
    Sync
};

struct MountSlewRequest {
    double ra_hours = 0.0;
    double dec_deg = 0.0;
    double alt_deg = 0.0;
    double az_deg = 0.0;
    bool use_alt_az = false;
    SlewMode mode = SlewMode::Goto;
};

struct MountSlewResponse : MountResponseBase {
    GotoResult goto_state;
};

struct MountTrackingRequest {};

struct MountTrackingResponse : MountResponseBase {
    TrackingStatus tracking;
};

enum class GuideDirection {
    North,
    South,
    East,
    West
};

struct MountGuideRequest {
    GuideDirection direction = GuideDirection::North;
    int duration_ms = 100;
    bool continuous = false;
};

struct MountGuideResponse : MountResponseBase {
    GuideStatus guide;
};

struct MountParkRequest {
    bool set_park_position = false;
    bool unpark = false;
    bool include_raw = true;
};

struct MountParkResponse : MountResponseBase {
    ParkStatus park;
};

struct MountHomeRequest {
    bool move_to_home = false;
    bool include_raw = true;
};

struct MountHomeResponse : MountResponseBase {
    HomeStatus home;
};

struct MountHomeConfigurationRequest {
    bool query = true;
    bool enable_auto_home = false;
    bool disable_auto_home = false;
    bool apply_axis1_offset = false;
    bool apply_axis2_offset = false;
    int axis1_offset_arcsec = 0;
    int axis2_offset_arcsec = 0;
    bool save = false;
    bool include_raw = true;
};

struct MountHomeConfigurationResponse : MountResponseBase {
    HomeConfiguration configuration;
    bool persisted = false;
};

struct MountSiteRequest {};

struct MountSiteResponse : MountResponseBase {
    SiteInfo site;
};

struct MountTimeRequest {};

struct MountTimeResponse : MountResponseBase {
    MountTimeInfo time;
};

struct MountPecRequest {
    bool query = true;
    bool enable = false;
    bool disable = false;
};

struct MountPecResponse : MountResponseBase {
    PecStatus pec;
};

struct MountLimitsRequest {
    bool query = true;
    std::optional<double> horizon_deg;
    std::optional<double> overhead_deg;
    std::optional<double> meridian_deg;
};

struct MountLimitsResponse : MountResponseBase {
    LimitsStatus limits;
};

enum class FocuserControlAction {
    Query,
    MoveInward,
    MoveOutward,
    Stop,
    GotoPosition
};

struct MountFocuserRequest {
    FocuserControlAction action = FocuserControlAction::Query;
    int target_position = 0;
};

struct MountFocuserResponse : MountResponseBase {
    FocuserState focuser;
};

struct MountFlipMirrorRequest {
    FlipMirrorPosition position = FlipMirrorPosition::Unknown;
};

struct MountFlipMirrorResponse : MountResponseBase {
    FlipMirrorStatus flip_mirror;
};

struct MountOrientationRequest {};

struct MountOrientationResponse : MountResponseBase {
    MountOrientationInfo orientation;
};

struct MountEnvironmentRequest {};

struct MountEnvironmentResponse : MountResponseBase {
    EnvironmentInfo environment;
};

enum class RotatorControlAction {
    Query,
    GetAngle,
    AbsoluteGoto,
    Relative,
    ContinuousCW,
    ContinuousCCW,
    Stop,
    SetZero,
    SetHalfTravel,
    MoveHalfTravelOrHome,
    SetRate,
    GetWorkingSlewRate,
    DerotateEnable,
    DerotateDisable,
    DerotateReverse,
    Park,
    Unpark
};

struct MountRotatorRequest {
    RotatorControlAction action = RotatorControlAction::Query;
    bool query_availability = true;
    bool query_driver_status = false;
    // parameters used depending on action
    double target_angle_deg = 0.0; // for AbsoluteGoto
    double relative_angle_deg = 0.0; // for Relative
    int rate_index = 0; // 1..9 for SetRate
};

struct MountRotatorResponse : MountResponseBase {
    RotatorStatus rotator;
};

struct MountModeRequest {
    // ASDEVLAB exposes only the user-visible mount modes ALT-AZ and EQ.
    // Changing the firmware mount type requires a firmware rebuild and upload,
    // so adapters should treat any set request as a configuration change request
    // rather than a runtime switch.
    std::optional<MountMode> mode;
    bool apply = false;
    bool include_raw = true;
};

struct MountModeResponse : MountResponseBase {
    MountMode mode = MountMode::Equatorial;
};

enum class MountAlignmentMode {
    Manual,
    AcceptStar,
    SaveAlignment
};

struct MountAlignmentRequest {
    MountAlignmentMode mode = MountAlignmentMode::Manual;
    int target_index = 0;
};

struct MountAlignmentResponse : MountResponseBase {
    bool aligned = false;
};

enum class TrackingRatePreset {
    Sidereal,
    Lunar,
    Solar,
    Custom
};

struct MountTrackingRateRequest {
    TrackingRatePreset preset = TrackingRatePreset::Sidereal;
    double rate = 0.0;
    bool readback = false;
};

struct MountTrackingRateResponse : MountResponseBase {
    double rate = 0.0;
};

enum class GuideRatePreset {
    Slow,
    Medium,
    Fast,
    Custom
};

struct MountGuideRateRequest {
    GuideRatePreset preset = GuideRatePreset::Slow;
    double rate = 0.0;
    bool readback = false;
};

struct MountGuideRateResponse : MountResponseBase {
    double rate = 0.0;
};

enum class SlewRatePreset {
    Center,
    Fast,
    Slow,
    Custom
};

struct MountSlewRateRequest {
    SlewRatePreset preset = SlewRatePreset::Center;
    double rate = 0.0;
    bool readback = false;
};

struct MountSlewRateResponse : MountResponseBase {
    double rate = 0.0;
};

enum class ManualMotionAxis {
    None,
    North,
    South,
    East,
    West
};

struct MountManualMotionRequest {
    ManualMotionAxis axis = ManualMotionAxis::None;
    bool start = false;
    bool stop = false;
};

struct MountManualMotionResponse : MountResponseBase {
    bool active = false;
};

struct MountAuxRequest {
    bool enable_buzzer = false;
    bool disable_buzzer = false;
    bool read_status = false;
};

struct MountAuxResponse : MountResponseBase {
    bool buzzer_enabled = false;
    bool busy = false;
    int distance_bar = 0;
};

struct MountGotoRequest {
    double ra_hours = 0.0;
    double dec_deg = 0.0;
    bool use_alt_az = false;
    double alt_deg = 0.0;
    double az_deg = 0.0;
};

struct MountGotoResponse : MountResponseBase {
    GotoResult goto_state;
};

struct MountSyncRequest {
    double ra_hours = 0.0;
    double dec_deg = 0.0;
    bool use_alt_az = false;
    double alt_deg = 0.0;
    double az_deg = 0.0;
};

struct MountSyncResponse : MountResponseBase {
    GotoResult goto_state;
};

struct MountSafetyRequest {
    bool abort = false;
};

struct MountSafetyResponse : MountResponseBase {
    bool aborted = false;
};

class StatusAPI {
public:
    virtual ~StatusAPI() = default;
    virtual MountStatusResponse status(const MountStatusRequest& request) = 0;
};

class SiteAPI {
public:
    virtual ~SiteAPI() = default;
    virtual MountSiteResponse site(const MountSiteRequest& request) = 0;
};

class TimeAPI {
public:
    virtual ~TimeAPI() = default;
    virtual MountTimeResponse time(const MountTimeRequest& request) = 0;
};

class AlignmentAPI {
public:
    virtual ~AlignmentAPI() = default;
    virtual MountAlignmentResponse alignment(const MountAlignmentRequest& request) = 0;
};

class GotoAPI {
public:
    virtual ~GotoAPI() = default;
    virtual MountGotoResponse gotoTarget(const MountGotoRequest& request) = 0;
};

class SyncAPI {
public:
    virtual ~SyncAPI() = default;
    virtual MountSyncResponse syncTarget(const MountSyncRequest& request) = 0;
};

class CoordinateAPI {
public:
    virtual ~CoordinateAPI() = default;
    virtual MountCoordinatesResponse coordinates(const MountCoordinatesRequest& request) = 0;
};

class TrackingAPI {
public:
    virtual ~TrackingAPI() = default;
    virtual MountTrackingResponse tracking(const MountTrackingRequest& request) = 0;
};

class TrackingRateAPI {
public:
    virtual ~TrackingRateAPI() = default;
    virtual MountTrackingRateResponse trackingRate(const MountTrackingRateRequest& request) = 0;
};

class GuideAPI {
public:
    virtual ~GuideAPI() = default;
    virtual MountGuideResponse guide(const MountGuideRequest& request) = 0;
};

class MotionAPI {
public:
    virtual ~MotionAPI() = default;
    virtual MountManualMotionResponse motion(const MountManualMotionRequest& request) = 0;
};

class SlewRateAPI {
public:
    virtual ~SlewRateAPI() = default;
    virtual MountSlewRateResponse slewRate(const MountSlewRateRequest& request) = 0;
};

class HomeAPI {
public:
    virtual ~HomeAPI() = default;
    virtual MountHomeResponse homeStatus(const MountHomeRequest& request) = 0;
};

class ParkAPI {
public:
    virtual ~ParkAPI() = default;
    virtual MountParkResponse parkStatus(const MountParkRequest& request) = 0;
};

class PecAPI {
public:
    virtual ~PecAPI() = default;
    virtual MountPecResponse pec(const MountPecRequest& request) = 0;
};

class LimitAPI {
public:
    virtual ~LimitAPI() = default;
    virtual MountLimitsResponse limits(const MountLimitsRequest& request) = 0;
};

class AuxiliaryAPI {
public:
    virtual ~AuxiliaryAPI() = default;
    virtual MountAuxResponse auxiliary(const MountAuxRequest& request) = 0;
};

class FocuserAPI {
public:
    virtual ~FocuserAPI() = default;
    virtual MountFocuserResponse focuser(const MountFocuserRequest& request) = 0;
};

class FlipMirrorAPI {
public:
    virtual ~FlipMirrorAPI() = default;
    virtual MountFlipMirrorResponse flipMirror(const MountFlipMirrorRequest& request) = 0;
};

class OrientationAPI {
public:
    virtual ~OrientationAPI() = default;
    virtual MountOrientationResponse orientation(const MountOrientationRequest& request) = 0;
};

class EnvironmentAPI {
public:
    virtual ~EnvironmentAPI() = default;
    virtual MountEnvironmentResponse environment(const MountEnvironmentRequest& request) = 0;
};

class RotatorAPI {
public:
    virtual ~RotatorAPI() = default;
    virtual MountRotatorResponse rotator(const MountRotatorRequest& request) = 0;
};

class SafetyAPI {
public:
    virtual ~SafetyAPI() = default;
    virtual MountSafetyResponse safety(const MountSafetyRequest& request) = 0;
};

// Single abstraction layer for all mount operations. Upper layers should use this
// rather than relying on raw LX200 commands directly.
class MountInterface : public StatusAPI,
                       public SiteAPI,
                       public TimeAPI,
                       public AlignmentAPI,
                       public GotoAPI,
                       public SyncAPI,
                       public CoordinateAPI,
                       public TrackingAPI,
                       public TrackingRateAPI,
                       public GuideAPI,
                       public MotionAPI,
                       public SlewRateAPI,
                       public HomeAPI,
                       public ParkAPI,
                       public PecAPI,
                       public LimitAPI,
                       public AuxiliaryAPI,
                       public FocuserAPI,
                       public FlipMirrorAPI,
                       public OrientationAPI,
                       public EnvironmentAPI,
                       public RotatorAPI,
                       public SafetyAPI {
public:
    virtual ~MountInterface() = default;

    virtual bool setFlipMirrorCamera() = 0;
    virtual bool setFlipMirrorEyepiece() = 0;
    virtual FlipMirrorPosition getFlipMirrorState() = 0;

    virtual bool connect() = 0;
    virtual bool gotoRaDec(double ra_hours, double dec_deg) = 0;
    virtual bool syncRaDec(double ra_hours, double dec_deg) = 0;
    virtual bool startTracking() = 0;
    virtual bool stopTracking() = 0;
    virtual bool park() = 0;
    virtual bool home() = 0;
    virtual bool abort() = 0;
    virtual std::string getStatus() = 0;

    /* `buildGotoCommands` removed from MountInterface: protocol-specific command
       construction belongs to concrete mount implementations (e.g. OnStepMountClient)
       or debug adapters. MotionService must not depend on protocol formatting. */

    virtual std::vector<MountCapability> capabilities() const = 0;
    virtual bool isCapabilitySupported(MountCapabilityId id) const = 0;

    virtual MountStatusResponse getStatus(const MountStatusRequest& request) = 0;
    virtual MountStatusResponse status(const MountStatusRequest& request) override {
        return getStatus(request);
    }
    virtual MountCoordinatesResponse getCoordinates(const MountCoordinatesRequest& request) = 0;
    virtual MountCoordinatesResponse coordinates(const MountCoordinatesRequest& request) override {
        return getCoordinates(request);
    }
    virtual MountSlewResponse slew(const MountSlewRequest& request) = 0;
    virtual MountTrackingResponse getTracking(const MountTrackingRequest& request) = 0;
    virtual MountTrackingResponse tracking(const MountTrackingRequest& request) override {
        return getTracking(request);
    }
    virtual MountGuideResponse guide(const MountGuideRequest& request) = 0;
    virtual MountParkResponse getParkStatus(const MountParkRequest& request) = 0;
    virtual MountParkResponse parkStatus(const MountParkRequest& request) override {
        return getParkStatus(request);
    }
    virtual MountHomeResponse getHomeStatus(const MountHomeRequest& request) = 0;
    virtual MountHomeResponse homeStatus(const MountHomeRequest& request) override {
        return getHomeStatus(request);
    }
    virtual MountHomeConfigurationResponse configureHome(const MountHomeConfigurationRequest& request) {
        (void)request;
        MountHomeConfigurationResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "home configuration capability unsupported";
        return response;
    }
    virtual MountHomeConfigurationResponse homeConfiguration(const MountHomeConfigurationRequest& request) {
        return configureHome(request);
    }
    virtual MountSiteResponse getSite(const MountSiteRequest& request) = 0;
    virtual MountSiteResponse site(const MountSiteRequest& request) override {
        return getSite(request);
    }
    virtual MountTimeResponse getTime(const MountTimeRequest& request) = 0;
    virtual MountTimeResponse time(const MountTimeRequest& request) override {
        return getTime(request);
    }
    virtual MountPecResponse getPec(const MountPecRequest& request) = 0;
    virtual MountPecResponse pec(const MountPecRequest& request) override {
        return getPec(request);
    }
    virtual MountLimitsResponse getLimits(const MountLimitsRequest& request) = 0;
    virtual MountLimitsResponse limits(const MountLimitsRequest& request) override {
        return getLimits(request);
    }
    virtual MountFocuserResponse getFocuser(const MountFocuserRequest& request) = 0;
    virtual MountFocuserResponse focuser(const MountFocuserRequest& request) override {
        return getFocuser(request);
    }
    virtual MountFlipMirrorResponse getFlipMirror(const MountFlipMirrorRequest& request) = 0;
    virtual MountFlipMirrorResponse moveFlipMirror(const MountFlipMirrorRequest& request) {
        return getFlipMirror(request);
    }
    virtual MountFlipMirrorResponse flipMirror(const MountFlipMirrorRequest& request) override {
        return moveFlipMirror(request);
    }
    virtual MountOrientationResponse getOrientation(const MountOrientationRequest& request) {
        (void)request;
        MountOrientationResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "orientation capability unsupported";
        return response;
    }
    virtual MountOrientationResponse orientation(const MountOrientationRequest& request) override {
        return getOrientation(request);
    }
    virtual MountEnvironmentResponse getEnvironment(const MountEnvironmentRequest& request) {
        (void)request;
        MountEnvironmentResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "environment capability unsupported";
        return response;
    }
    virtual MountEnvironmentResponse environment(const MountEnvironmentRequest& request) override {
        return getEnvironment(request);
    }
    virtual MountRotatorResponse getRotator(const MountRotatorRequest& request) {
        (void)request;
        MountRotatorResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "rotator capability unsupported";
        return response;
    }
    virtual MountRotatorResponse rotator(const MountRotatorRequest& request) override {
        return getRotator(request);
    }
    virtual MountModeResponse getMountMode(const MountModeRequest& request) {
        (void)request;
        MountModeResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "mount mode capability unsupported";
        return response;
    }
    virtual MountModeResponse setMountMode(const MountModeRequest& request) {
        (void)request;
        MountModeResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "mount mode capability unsupported";
        return response;
    }
    virtual MountAlignmentResponse align(const MountAlignmentRequest& request) {
        (void)request;
        MountAlignmentResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "alignment capability unsupported";
        return response;
    }
    virtual MountAlignmentResponse alignment(const MountAlignmentRequest& request) override {
        return align(request);
    }
    virtual MountTrackingRateResponse setTrackingRate(const MountTrackingRateRequest& request) {
        (void)request;
        MountTrackingRateResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "tracking rate capability unsupported";
        return response;
    }
    virtual MountTrackingRateResponse trackingRate(const MountTrackingRateRequest& request) override {
        return setTrackingRate(request);
    }
    virtual MountGuideRateResponse setGuideRate(const MountGuideRateRequest& request) {
        (void)request;
        MountGuideRateResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "guide rate capability unsupported";
        return response;
    }
    virtual MountSlewRateResponse setSlewRate(const MountSlewRateRequest& request) {
        (void)request;
        MountSlewRateResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "slew rate capability unsupported";
        return response;
    }
    virtual MountSlewRateResponse slewRate(const MountSlewRateRequest& request) override {
        return setSlewRate(request);
    }
    virtual MountManualMotionResponse manualMotion(const MountManualMotionRequest& request) {
        (void)request;
        MountManualMotionResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "manual motion capability unsupported";
        return response;
    }
    virtual MountManualMotionResponse motion(const MountManualMotionRequest& request) override {
        return manualMotion(request);
    }
    virtual MountAuxResponse controlAux(const MountAuxRequest& request) {
        (void)request;
        MountAuxResponse response;
        response.supported = false;
        response.error = MountError::Unsupported;
        response.message = "auxiliary capability unsupported";
        return response;
    }
    virtual MountAuxResponse auxiliary(const MountAuxRequest& request) override {
        return controlAux(request);
    }
    virtual MountSafetyResponse safety(const MountSafetyRequest& request) override {
        MountSafetyResponse response;
        response.ok = abort();
        response.aborted = response.ok;
        response.supported = true;
        response.error = response.ok ? MountError::None : MountError::Communication;
        response.message = request.abort ? (response.ok ? "abort command accepted" : "abort command failed") : "safety request accepted";
        return response;
    }

    virtual std::optional<MountStatus> readMountStatus() = 0;
    virtual std::optional<RightAscension> readRightAscension() = 0;
    virtual std::optional<Declination> readDeclination() = 0;
    virtual std::optional<Coordinates> readCoordinates() = 0;
    virtual std::optional<GotoResult> readGotoState() = 0;
};

} // namespace hardware
} // namespace asdevlab
