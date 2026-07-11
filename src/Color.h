#ifndef COLOR_H
#define COLOR_H
#include <string>
#include <cstdint>

struct Color {
    uint8_t r, g, b;
    Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : r(r), g(g), b(b) {}
    std::string toHex() const;
    
    double distance(const Color& other) const;
    bool operator==(const Color& other) const;
    bool operator<(const Color& other) const;
};

#endif