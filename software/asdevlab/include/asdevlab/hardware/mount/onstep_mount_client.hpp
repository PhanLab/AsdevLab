#pragma once

#include "asdevlab/hardware/mount/lx200_connection_manager.hpp"
#include "asdevlab/hardware/mount/lx200_parser.hpp"
#include "asdevlab/hardware/mount/mount_config.hpp"
#include "asdevlab/hardware/mount/mount_interface.hpp"
#include "asdevlab/hardware/mount/transport_interface.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace asdevlab {
namespace hardware {

// Concrete LX200/OnStepX mount adapter that implements the typed MountInterface.
class OnStepMountClient : public MountInterface {
public:
    // Construct with host and TCP port (convenience) — will create an Lx200ConnectionManager.
    explicit OnStepMountClient(std::string host = "",
                               int port = 0);

    explicit OnStepMountClient(MountConfig config);

    // Construct with a custom transport (DI friendly).
    explicit OnStepMountClient(std::unique_ptr<TransportInterface> transport);

    bool setFlipMirrorCamera() override;
    bool setFlipMirrorEyepiece() override;
    FlipMirrorPosition getFlipMirrorState() override;

    bool connect() override;
    bool gotoRaDec(double ra_hours, double dec_deg) override;
    bool syncRaDec(double ra_hours, double dec_deg) override;
    bool startTracking() override;
    bool stopTracking() override;
    bool park() override;
    bool home() override;
    bool abort() override;
    std::string getStatus() override;
    bool sendRawCommand(const std::string& command, std::string& response_out);

    std::vector<MountCapability> capabilities() const override;
    bool isCapabilitySupported(MountCapabilityId id) const override;

    MountStatusResponse getStatus(const MountStatusRequest& request) override;
    MountCoordinatesResponse getCoordinates(const MountCoordinatesRequest& request) override;
    MountSlewResponse slew(const MountSlewRequest& request) override;
    MountTrackingResponse getTracking(const MountTrackingRequest& request) override;
    MountGuideResponse guide(const MountGuideRequest& request) override;
    MountParkResponse getParkStatus(const MountParkRequest& request) override;
    MountHomeResponse getHomeStatus(const MountHomeRequest& request) override;
    MountHomeConfigurationResponse configureHome(const MountHomeConfigurationRequest& request) override;
    // Diagnostic-only compatibility API. Normal runtime should not depend on it.
    MountSiteResponse getSite(const MountSiteRequest& request) override;
    // Diagnostic-only compatibility API. Normal runtime should not depend on it.
    MountTimeResponse getTime(const MountTimeRequest& request) override;
    MountOrientationResponse getOrientation(const MountOrientationRequest& request) override;
    MountEnvironmentResponse getEnvironment(const MountEnvironmentRequest& request) override;
    MountRotatorResponse getRotator(const MountRotatorRequest& request) override;
    MountModeResponse getMountMode(const MountModeRequest& request) override;
    MountModeResponse setMountMode(const MountModeRequest& request) override;

    MountMode queryMountMode();
    MountMode getDetectedMountMode() const;
    void updateDetectedMountMode();
    MountPecResponse getPec(const MountPecRequest& request) override;
    MountLimitsResponse getLimits(const MountLimitsRequest& request) override;
    MountFocuserResponse getFocuser(const MountFocuserRequest& request) override;
    MountFlipMirrorResponse getFlipMirror(const MountFlipMirrorRequest& request) override;
    MountFlipMirrorResponse moveFlipMirror(const MountFlipMirrorRequest& request) override;
    MountAlignmentResponse align(const MountAlignmentRequest& request) override;
    MountTrackingRateResponse setTrackingRate(const MountTrackingRateRequest& request) override;
    MountGuideRateResponse setGuideRate(const MountGuideRateRequest& request) override;
    MountSlewRateResponse setSlewRate(const MountSlewRateRequest& request) override;
    MountManualMotionResponse manualMotion(const MountManualMotionRequest& request) override;
    MountAuxResponse controlAux(const MountAuxRequest& request) override;
    MountGotoResponse gotoTarget(const MountGotoRequest& request) override;
    MountSyncResponse syncTarget(const MountSyncRequest& request) override;
    MountSafetyResponse safety(const MountSafetyRequest& request) override;

    std::vector<std::string> buildGotoCommands(const MountGotoRequest& request);

    std::optional<MountStatus> readMountStatus() override;
    std::optional<RightAscension> readRightAscension() override;
    std::optional<Declination> readDeclination() override;
    std::optional<Coordinates> readCoordinates() override;
    std::optional<GotoResult> readGotoState() override;

private:
    std::unique_ptr<Lx200ConnectionManager> connection_manager_;
    std::unique_ptr<TransportInterface> transport_;
    MountMode detectedMode = MountMode::AltAz;
    bool sendCommand(const std::string& command, std::string& response_out) const;
};

} // namespace hardware
} // namespace asdevlab
