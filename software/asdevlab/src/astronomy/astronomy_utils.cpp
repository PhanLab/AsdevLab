#include "asdevlab/astronomy/astronomy_utils.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <libnova/julian_day.h>
#include <libnova/transform.h>
#include <libnova/refraction.h>
#include <libnova/sidereal_time.h>

namespace asdevlab {
namespace astronomy {

static double toJulianDay(const std::chrono::system_clock::time_point& time_point) {
    struct ln_date ln_date_value{};
    const auto tt = std::chrono::system_clock::to_time_t(time_point);
    std::tm utc_tm{};
    gmtime_r(&tt, &utc_tm);
    ln_date_value.years = utc_tm.tm_year + 1900;
    ln_date_value.months = utc_tm.tm_mon + 1;
    ln_date_value.days = utc_tm.tm_mday;
    ln_date_value.hours = utc_tm.tm_hour;
    ln_date_value.minutes = utc_tm.tm_min;
    ln_date_value.seconds = static_cast<double>(utc_tm.tm_sec);
    return ln_get_julian_day(&ln_date_value);
}

HorizontalCoordinates compute_horizontal_coordinates(double ra_hours, double dec_degrees, const catalog::ObservationContext& context) {
    const double jd = toJulianDay(context.utc_time);
    struct ln_equ_posn equ{};
    struct ln_hrz_posn hrz{};
    struct ln_lnlat_posn observer{};
    equ.ra = ra_hours * 15.0;
    equ.dec = dec_degrees;
    observer.lng = context.longitude_degrees;
    observer.lat = context.latitude_degrees;

    const double lst = ln_get_apparent_sidereal_time(jd) + context.longitude_degrees / 15.0;
    ln_get_hrz_from_equ_sidereal_time(&equ, &observer, lst, &hrz);
    double alt = hrz.alt;
    if (context.pressure_mbar > 0.0 && context.temperature_c != 0.0) {
        alt = alt + ln_get_refraction_adj(alt, context.pressure_mbar, context.temperature_c);
    }
    return HorizontalCoordinates{alt, hrz.az};
}

double compute_altitude_degrees(double ra_hours, double dec_degrees, const catalog::ObservationContext& context) {
    return compute_horizontal_coordinates(ra_hours, dec_degrees, context).alt_degrees;
}

double compute_altitude_degrees(double ra_hours, double dec_degrees, double latitude_degrees, double longitude_degrees, std::time_t utc, double pressure_mbar, double temperature_c) {
    const double jd = toJulianDay(std::chrono::system_clock::from_time_t(utc));
    struct ln_equ_posn equ{};
    struct ln_hrz_posn hrz{};
    struct ln_lnlat_posn observer{};
    equ.ra = ra_hours * 15.0;
    equ.dec = dec_degrees;
    observer.lng = longitude_degrees;
    observer.lat = latitude_degrees;

    const double lst = ln_get_apparent_sidereal_time(jd) + longitude_degrees / 15.0;
    ln_get_hrz_from_equ_sidereal_time(&equ, &observer, lst, &hrz);
    double alt = hrz.alt;
    if (pressure_mbar > 0.0 && temperature_c != 0.0) {
        alt = alt + ln_get_refraction_adj(alt, pressure_mbar, temperature_c);
    }
    return alt;
}

} // namespace astronomy
} // namespace asdevlab
