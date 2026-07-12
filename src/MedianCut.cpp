#include "MedianCut.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <cmath>

std::vector<Color> MedianCut::extractPalette(const std::vector<Color>& pixels, int k) {
    if (pixels.empty() || k <= 0) {
        return {};
    }
    
    std::cout << "Examining " << pixels.size() << " pixels..." << std::endl;
    
    if ((int)pixels.size() <= k) {
        std::vector<Color> unique = pixels;
        std::sort(unique.begin(), unique.end());
        unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
        return unique;
    }
    
    MedianCut::ColorCube initialCube;
    initialCube.colors = pixels;
    initialCube.updateBounds();
    
    
    
    std::vector<MedianCut::ColorCube> cubes;
    cubes.push_back(initialCube);
    cubes = splitCubes(cubes, k);
    
    std::vector<Color> palette;
    for (const auto& cube : cubes) {
        if (!cube.colors.empty()) {
            palette.push_back(cube.getAverageColor());
        }
    }
    std::cout << "Extracted " << palette.size() << " colors!" << std::endl;
    return palette;
}




int MedianCut::ColorCube::getLargestChannel() const {
    int rRange = rMax - rMin;
    int gRange = gMax - gMin;
    int bRange = bMax - bMin;
    if (rRange >= gRange && rRange >= bRange) return 0;  // Red
    if (gRange >= rRange && gRange >= bRange) return 1;  // Green
    return 2;  // Blue
}



std::pair<MedianCut::ColorCube, MedianCut::ColorCube> 
MedianCut::ColorCube::split() const {
    ColorCube cube1, cube2;
    
    if (colors.empty()) {
        return {cube1, cube2};
    }
    
    int channel = getLargestChannel();
    std::vector<Color> sortedColors = colors;
    
    if (channel == 0) {
        std::sort(sortedColors.begin(), sortedColors.end(),
                  [](const Color& a, const Color& b) { return a.r < b.r; });
    } else if (channel == 1) {
        std::sort(sortedColors.begin(), sortedColors.end(),
                  [](const Color& a, const Color& b) { return a.g < b.g; });
    } else {
        std::sort(sortedColors.begin(), sortedColors.end(),
                  [](const Color& a, const Color& b) { return a.b < b.b; });
    }
    
    size_t median = sortedColors.size() / 2;
    cube1.colors.assign(sortedColors.begin(), sortedColors.begin() + median);
    cube2.colors.assign(sortedColors.begin() + median, sortedColors.end());
    
    cube1.updateBounds();
    cube2.updateBounds();
    
    return {cube1, cube2};
}



Color MedianCut::ColorCube::getAverageColor() const {
    if (colors.empty()) {
        return Color(0, 0, 0);
    }
    long long sumR = 0, sumG = 0, sumB = 0;
    for (const auto& color : colors) {
        sumR += color.r;
        sumG += color.g;
        sumB += color.b;
    }
    size_t count = colors.size();
    return Color(
        (uint8_t)((sumR + count/2) / count),
        (uint8_t)((sumG + count/2) / count),
        (uint8_t)((sumB + count/2) / count)
    );
}



void MedianCut::ColorCube::updateBounds() {
    if (colors.empty()) {
        rMin = rMax = gMin = gMax = bMin = bMax = 0;
        return;
    }
    rMin = rMax = colors[0].r;
    gMin = gMax = colors[0].g;
    bMin = bMax = colors[0].b;
    
    
    
    for (const auto& color : colors) {
        if (color.r < rMin) rMin = color.r;
        if (color.r > rMax) rMax = color.r;
        if (color.g < gMin) gMin = color.g;
        if (color.g > gMax) gMax = color.g;
        if (color.b < bMin) bMin = color.b;
        if (color.b > bMax) bMax = color.b;
    }
}

std::vector<MedianCut::ColorCube> 
MedianCut::splitCubes(const std::vector<ColorCube>& initialCubes, int k) {
    auto compareCubes = [](const MedianCut::ColorCube& a, const MedianCut::ColorCube& b) {
        int volumeA = a.getVolume();
        int volumeB = b.getVolume();
        if (volumeA != volumeB) {
            return volumeA < volumeB;  // this will help determine the color with the biggest amount 
        }
        return a.colors.size() < b.colors.size();  // this will help between colors that have close amounts and to choose between them!!!
    };



    
    std::priority_queue<MedianCut::ColorCube, std::vector<MedianCut::ColorCube>, decltype(compareCubes)> 
        cubeQueue(compareCubes);
    for (const auto& cube : initialCubes) {
        if (!cube.colors.empty()) {
            cubeQueue.push(cube);
        }
    }
    while ((int)cubeQueue.size() < k && !cubeQueue.empty()) {
        MedianCut::ColorCube cube = cubeQueue.top();
        cubeQueue.pop();
        if (cube.colors.size() <= 1) {
            cubeQueue.push(cube);
            break;
        }
        auto [cube1, cube2] = cube.split();
        if (!cube1.colors.empty()) cubeQueue.push(cube1);
        if (!cube2.colors.empty()) cubeQueue.push(cube2);
    }
    
    
    std::vector<MedianCut::ColorCube> result;
    while (!cubeQueue.empty()) {
        result.push_back(cubeQueue.top());
        cubeQueue.pop();
    }
    
    return result;
}