#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

#include <iostream>

namespace geo {

bool Coordinates::operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
}
bool Coordinates::operator!=(const Coordinates& other) const {
        return !(*this == other);
}

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    
    if (from == to) {
        return 0;
    }
    
    static const double dr = 3.1415926535 / 180.;
    const int radius_of_the_earth = 6371000;
    
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * radius_of_the_earth;
}

}  // namespace geo
