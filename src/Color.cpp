#include "Color.h"
#include <cmath>
#include <sstream>
#include <iomanip>

std::string Color::toHex() const {
    std::stringstream ss;
    ss << "#" << std::hex << std::uppercase << std::setfill('0')
       << std::setw(2) << (int)r
       << std::setw(2) << (int)g
       << std::setw(2) << (int)b;
    return ss.str();
}



double Color::distance(const Color& other) const {
    int dr = r - other.r;
    int dg = g - other.g;
    int db = b - other.b;
    return std::sqrt(dr*dr + dg*dg + db*db);
}
bool Color::operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b;
}
bool Color::operator<(const Color& other) const {
    if (r != other.r) return r < other.r;
    if (g != other.g) return g < other.g;
    return b < other.b;
}