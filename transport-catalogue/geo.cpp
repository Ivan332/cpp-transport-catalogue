#include "geo.h"

namespace geo {
    bool Coordinates::operator==(const Coordinates& other) const {
        return latitude == other.latitude && longitude == other.longitude;
    }
    bool Coordinates::operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
} // namespace geo
