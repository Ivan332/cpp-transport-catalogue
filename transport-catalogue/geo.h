#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

namespace geo {
    struct Coordinates {
        double latitude;
        double longitude;

        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
    };

    inline static const double EARTH_RADIUS = 6371000.0;

    inline double compute_distance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = M_PI / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * EARTH_RADIUS;
    }
} // namespace geo
