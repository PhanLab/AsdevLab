#include "asdevlab/hardware/mount/lx200_parser.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace asdevlab::hardware;

    const auto status = LX200Parser::parseMountStatus("nNpH");
    assert(status.has_value());
    assert(status->valid);
    assert(status->raw == "nNpH");

    const auto packed_status = LX200Parser::parsePackedMountStatus("0x0f");
    assert(packed_status.has_value());
    assert(packed_status->valid);
    assert(packed_status->tracking_enabled);
    assert(packed_status->parked);
    assert(packed_status->homed);
    assert(packed_status->slewing);
    assert(packed_status->guiding);

    assert(!LX200Parser::parsePackedMountStatus("zz").has_value());
    assert(!LX200Parser::parsePackedMountStatus("").has_value());

    const auto ra = LX200Parser::parseRightAscension("23:59:59");
    assert(ra.has_value());
    assert(ra->hours == 23);
    assert(ra->minutes == 59);
    assert(ra->seconds == 59.0);

    assert(!LX200Parser::parseRightAscension("23:59").has_value());
    assert(!LX200Parser::parseRightAscension("bad").has_value());

    const auto dec = LX200Parser::parseDeclination("+12*34:56");
    assert(dec.has_value());
    assert(dec->degrees == 12);
    assert(dec->minutes == 34);
    assert(dec->seconds == 56.0);
    assert(!dec->negative);

    assert(!LX200Parser::parseDeclination("12*34").has_value());
    assert(!LX200Parser::parseDeclination("bad").has_value());

    const auto coords = LX200Parser::parseCoordinates("23:59:59", "+12*34:56");
    assert(coords.has_value());
    assert(coords->valid);

    const auto goto_ok = LX200Parser::parseGotoState("0");
    assert(goto_ok.has_value());
    assert(goto_ok->ok);
    assert(goto_ok->state == GotoState::Success);

    const auto tracking = LX200Parser::parseTracking("1");
    assert(tracking.has_value());
    assert(tracking->enabled);
    assert(tracking->valid);

    const auto guide = LX200Parser::parseGuide("1");
    assert(guide.has_value());
    assert(guide->active);
    assert(guide->valid);

    const auto park = LX200Parser::parseParkStatus("1");
    assert(park.has_value());
    assert(park->state == ParkState::Parked);

    const auto home = LX200Parser::parseHomeStatus("1");
    assert(home.has_value());
    assert(home->state == HomeState::AtHome);

    const auto home_config = LX200Parser::parseHomeConfiguration("1,1,180,240");
    assert(home_config.has_value());
    assert(home_config->valid);
    assert(home_config->has_sense);
    assert(home_config->auto_home_enabled);
    assert(home_config->axis1_offset_arcsec == 180);
    assert(home_config->axis2_offset_arcsec == 240);

    const auto tracking_rate = LX200Parser::parseTrackingRate("0.75");
    assert(tracking_rate.has_value());
    assert(*tracking_rate == 0.75);

    const auto guide_rate = LX200Parser::parseGuideRate("0.5");
    assert(guide_rate.has_value());
    assert(*guide_rate == 0.5);

    const auto slew_rate = LX200Parser::parseSlewRate("1.5");
    assert(slew_rate.has_value());
    assert(*slew_rate == 1.5);

    const auto pec = LX200Parser::parsePecStatus("1");
    assert(pec.has_value());
    assert(pec->state == PecState::Recording);

    const auto limits = LX200Parser::parseLimits("10", "20", "30");
    assert(limits.has_value());
    assert(limits->valid);

    const auto focuser = LX200Parser::parseFocuserState("100");
    assert(focuser.has_value());
    assert(focuser->position == 100);
    assert(!focuser->moving);
    assert(!focuser->busy);

    const auto moving_focuser = LX200Parser::parseFocuserState("M2");
    assert(moving_focuser.has_value());
    assert(moving_focuser->position == 0);
    assert(moving_focuser->moving);
    assert(moving_focuser->busy);
    assert(moving_focuser->goto_rate == 2);

    const auto stopped_focuser = LX200Parser::parseFocuserState("S5");
    assert(stopped_focuser.has_value());
    assert(!stopped_focuser->moving);
    assert(!stopped_focuser->busy);
    assert(stopped_focuser->goto_rate == 5);

    const auto goto_outside = LX200Parser::parseGotoState("6");
    assert(goto_outside.has_value());
    assert(goto_outside->ok);
    assert(goto_outside->state == GotoState::OutsideLimits);

    const auto goto_busy = LX200Parser::parseGotoState("5");
    assert(goto_busy.has_value());
    assert(goto_busy->ok);
    assert(goto_busy->state == GotoState::InProgress);

    const auto camera_flip = LX200Parser::parseFlipMirrorState("1");
    assert(camera_flip.has_value());
    assert(camera_flip->position == FlipMirrorPosition::Camera);

    const auto eyepiece_flip = LX200Parser::parseFlipMirrorState("0");
    assert(eyepiece_flip.has_value());
    assert(eyepiece_flip->position == FlipMirrorPosition::Eyepiece);

    assert(!LX200Parser::parseFlipMirrorState("2").has_value());
    assert(!LX200Parser::parseFlipMirrorState("Camera").has_value());
    assert(!LX200Parser::parseFlipMirrorState("X").has_value());

    assert(!LX200Parser::parseGotoState("A").has_value());
    assert(!LX200Parser::parseGotoState("10").has_value());
    assert(!LX200Parser::parseGotoState("").has_value());

    assert(!LX200Parser::parseTrackingRate("bad").has_value());
    assert(!LX200Parser::parseGuideRate("bad").has_value());
    assert(!LX200Parser::parseSlewRate("bad").has_value());
    assert(!LX200Parser::parsePecStatus("bad").has_value());
    assert(!LX200Parser::parseFocuserState("").has_value());
    assert(!LX200Parser::parseFlipMirrorState("").has_value());

    const auto mode_altaz = LX200Parser::parseMountMode("0#");
    assert(mode_altaz.has_value());
    assert(mode_altaz.value() == MountMode::AltAz);

    const auto mode_eq = LX200Parser::parseMountMode("1#");
    assert(mode_eq.has_value());
    assert(mode_eq.value() == MountMode::Equatorial);

    assert(!LX200Parser::parseMountMode("3").has_value());
    assert(!LX200Parser::parseMountMode("bad").has_value());

    std::cout << "lx200 parser tests passed\n";
    return 0;
}
