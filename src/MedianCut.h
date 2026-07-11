#ifndef MEDIANCUT_H
#define MEDIANCUT_H
#include <vector>
#include <utility> 
#include "Color.h"

class MedianCut {
public:
    static std::vector<Color> extractPalette(const std::vector<Color>& pixels, int k);
    
private:
    struct ColorCube {
        std::vector<Color> colors;
        int rMin, rMax;                    // red rangeee
        int gMin, gMax;                   // green rangee
        int bMin, bMax;                  // blue rangee
        
        ColorCube() : rMin(255), rMax(0), gMin(255), gMax(0), bMin(255), bMax(0) {}
        
        int getLargestChannel() const;
        std::pair<ColorCube, ColorCube> split() const;
        Color getAverageColor() const;    // this is to get the avg colorsss
        void updateBounds();
        
        int getVolume() const {
            return (rMax - rMin) * (gMax - gMin) * (bMax - bMin);
        }
    };
    
    static std::vector<ColorCube> splitCubes(const std::vector<ColorCube>& cubes, int k);
};

#endif