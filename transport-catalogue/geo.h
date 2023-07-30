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

    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = M_PI / 180.;
        return acos(sin(from.latitude * dr) * sin(to.latitude * dr)
            + cos(from.latitude * dr) * cos(to.latitude * dr) * cos(abs(from.longitude - to.longitude) * dr))
            * EARTH_RADIUS;
    }
} // namespace geo
