#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"

#include <libnova/julian_day.h>
#include <libnova/solar.h>
#include <libnova/lunar.h>
#include <libnova/mars.h>
#include <libnova/mercury.h>
#include <libnova/venus.h>
#include <libnova/jupiter.h>
#include <libnova/saturn.h>
#include <libnova/uranus.h>
#include <libnova/neptune.h>
#include <libnova/pluto.h>
#include <libnova/transform.h>
#include <libnova/refraction.h>
#include <libnova/rise_set.h>
#include <libnova/sidereal_time.h>
#include <libnova/ln_types.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <ctime>
#include <string>

namespace asdevlab {
namespace astronomy {
namespace {

bool isSupportedEphemerisId(const std::string& value) {
    const auto lowered = [] (const std::string& target_name) {
        std::string normalized;
        normalized.reserve(target_name.size());
        for (char ch : target_name) {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return normalized;
    }(value);

    return lowered == "sun" || lowered == "moon" || lowered == "mercury" || lowered == "venus"
        || lowered == "mars" || lowered == "jupiter" || lowered == "saturn" || lowered == "uranus"
        || lowered == "neptune";
}

struct LnDate {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;
};

LnDate makeLnDate(const std::chrono::system_clock::time_point& time_point) {
    const auto tt = std::chrono::system_clock::to_time_t(time_point);
    std::tm utc_tm{};
    gmtime_r(&tt, &utc_tm);
    return {utc_tm.tm_year + 1900, utc_tm.tm_mon + 1, utc_tm.tm_mday, utc_tm.tm_hour, utc_tm.tm_min, static_cast<double>(utc_tm.tm_sec)};
}

double toJulianDay(const std::chrono::system_clock::time_point& time_point) {
    const auto date = makeLnDate(time_point);
    struct ln_date ln_date_value {};
    ln_date_value.years = date.year;
    ln_date_value.months = date.month;
    ln_date_value.days = date.day;
    ln_date_value.hours = date.hour;
    ln_date_value.minutes = date.minute;
    ln_date_value.seconds = date.second;
    return ln_get_julian_day(&ln_date_value);
}

double localSiderealTimeHours(const catalog::ObservationContext& context, double jd) {
    return ln_get_apparent_sidereal_time(jd) + context.longitude_degrees / 15.0;
}

double applyRefraction(const catalog::ObservationContext& context, double altitude_degrees) {
    if (context.pressure_mbar <= 0.0 || context.temperature_c == 0.0) {
        return altitude_degrees;
    }
    return altitude_degrees + ln_get_refraction_adj(altitude_degrees, context.pressure_mbar, context.temperature_c);
}

catalog::ResolvedTarget buildResolvedTarget(const std::string& target_name, const catalog::ObservationContext& context, const ln_equ_posn& equatorial) {
    catalog::ResolvedTarget target;
    target.ra_hours = equatorial.ra / 15.0;
    target.dec_degrees = equatorial.dec;
    target.epoch = "J2000";
    target.resolved = true;
    target.horizontal_computed = false;
    target.visibility = false;
    return target;
}

void populateRiseSet(const std::string& lower, const catalog::ObservationContext& context, double jd, struct ln_lnlat_posn& observer, catalog::ResolvedTarget& target) {
    struct ln_rst_time rst{};
    const auto setRise = [&](auto&& fn) {
        if (fn(jd, &observer, &rst) == 0) {
            target.rise = std::to_string(rst.rise);
            target.transit = std::to_string(rst.transit);
            target.set = std::to_string(rst.set);
        }
    };

    if (lower == "sun") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_solar_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "moon") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_lunar_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "mercury") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_mercury_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "venus") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_venus_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "mars") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_mars_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "jupiter") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_jupiter_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "saturn") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_saturn_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "uranus") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_uranus_rst(current_jd, current_observer, current_rst);
        });
    } else if (lower == "neptune") {
        setRise([](double current_jd, struct ln_lnlat_posn* current_observer, struct ln_rst_time* current_rst) {
            return ln_get_neptune_rst(current_jd, current_observer, current_rst);
        });
    }
}

} // namespace

bool LibnovaEphemerisProvider::supports(const catalog::CatalogObject& object) const {
    const auto ephemeris_id = object.ephemeris_id.empty() ? object.id : object.ephemeris_id;
    return object.coordinate_source == "ephemeris" && isSupportedEphemerisId(ephemeris_id);
}

catalog::ResolvedTarget LibnovaEphemerisProvider::resolve(const catalog::CatalogObject& object, const catalog::ObservationContext& context) const {
    const auto ephemeris_id = object.ephemeris_id.empty() ? object.id : object.ephemeris_id;
    return resolve(ephemeris_id, context);
}

catalog::ResolvedTarget LibnovaEphemerisProvider::resolve(const std::string& target_name, const catalog::ObservationContext& context) const {
    const auto jd = toJulianDay(context.utc_time);
    const auto lower = [&]() {
        std::string normalized;
        normalized.reserve(target_name.size());
        for (char ch : target_name) {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return normalized;
    }();

    if (!isSupportedEphemerisId(lower)) {
        return {};
    }

    struct ln_equ_posn equatorial{};
    struct ln_hrz_posn horizontal{};
    struct ln_lnlat_posn observer{};
    observer.lng = context.longitude_degrees;
    observer.lat = context.latitude_degrees;

    if (lower == "sun") {
        ln_get_solar_equ_coords(jd, &equatorial);
    } else if (lower == "moon") {
        ln_get_lunar_equ_coords(jd, &equatorial);
    } else if (lower == "mars") {
        ln_get_mars_equ_coords(jd, &equatorial);
    } else if (lower == "mercury") {
        ln_get_mercury_equ_coords(jd, &equatorial);
    } else if (lower == "venus") {
        ln_get_venus_equ_coords(jd, &equatorial);
    } else if (lower == "jupiter") {
        ln_get_jupiter_equ_coords(jd, &equatorial);
    } else if (lower == "saturn") {
        ln_get_saturn_equ_coords(jd, &equatorial);
    } else if (lower == "uranus") {
        ln_get_uranus_equ_coords(jd, &equatorial);
    } else if (lower == "neptune") {
        ln_get_neptune_equ_coords(jd, &equatorial);
    } else {
        return {};
    }

    auto target = buildResolvedTarget(target_name, context, equatorial);
    populateRiseSet(lower, context, jd, observer, target);
    return target;
}

} // namespace astronomy
} // namespace asdevlab
