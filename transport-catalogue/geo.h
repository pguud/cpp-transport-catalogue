#pragma once

namespace geo {

struct Coordinates {
    double lat; // широта
    double lng; // долгота

    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
