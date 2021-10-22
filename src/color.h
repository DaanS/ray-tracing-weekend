#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include "vec3.h"

void write_color(std::ostream& os, vec3 const& color) {
    os << static_cast<int>(255.99 * color.x) << ' '
       << static_cast<int>(255.99 * color.y) << ' '
       << static_cast<int>(255.99 * color.z) << '\n';
}

#endif