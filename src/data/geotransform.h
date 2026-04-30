#pragma once
#include <QPointF>
#include <QtMath>

/// Web Mercator (EPSG:3857) projection + Haversine distance.
/// All static methods — no instantiation needed.
class GeoTransform {
public:
    GeoTransform() = delete;

    static constexpr int    TILE_SIZE = 256;
    static constexpr double EARTH_RADIUS = 6371000.0;    // metres
    static constexpr double MAX_LAT = 85.0511287798066;  // Mercator lat limit

    /// World map width/height in pixels at a given zoom.
    static double mapSize(int zoom) {
        return static_cast<double>(TILE_SIZE) * (1LL << zoom);
    }

    // ---- Web Mercator projection ------------------------------------------

    /// lat/lon (degrees) → absolute pixel coordinate at zoom z.
    static QPointF latLonToPixel(double lat, double lon, int zoom) {
        double s = mapSize(zoom);
        double x = (lon + 180.0) / 360.0 * s;
        double latRad = qDegreesToRadians(qBound(-MAX_LAT, lat, MAX_LAT));
        double y = (0.5 - qLn(qTan(M_PI_4 + latRad * 0.5)) / (2.0 * M_PI)) * s;
        return {x, y};
    }

    /// Pixel coordinate at zoom z → lat/lon in degrees.
    static void pixelToLatLon(QPointF pixel, int zoom, double& lat, double& lon) {
        double s = mapSize(zoom);
        lon = pixel.x() / s * 360.0 - 180.0;
        double n = M_PI - 2.0 * M_PI * pixel.y() / s;
        lat = qRadiansToDegrees(qAtan(0.5 * (qExp(n) - qExp(-n))));
    }

    // ---- Haversine distance -----------------------------------------------

    /// Great-circle distance in metres between two lat/lon pairs (degrees).
    static double haversine(double lat1, double lon1, double lat2, double lon2) {
        double dLat = qDegreesToRadians(lat2 - lat1);
        double dLon = qDegreesToRadians(lon2 - lon1);
        double a = qSin(dLat * 0.5) * qSin(dLat * 0.5)
                 + qCos(qDegreesToRadians(lat1)) * qCos(qDegreesToRadians(lat2))
                 * qSin(dLon * 0.5) * qSin(dLon * 0.5);
        return EARTH_RADIUS * 2.0 * qAtan2(qSqrt(a), qSqrt(1.0 - a));
    }
};
