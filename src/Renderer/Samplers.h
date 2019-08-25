#pragma once

#include <cstdint>

class Samplers
{
public:
    static const uint8_t PBR_BASECOLOR = 0;
    static const uint8_t PBR_METALROUGHNESS = 1;
    static const uint8_t PBR_NORMAL = 2;

    static const uint8_t LIGHTS_POINTLIGHT_POSITION = 3;
    static const uint8_t LIGHTS_POINTLIGHT_POWER = 4;

    static const uint8_t CLUSTERS_CLUSTERS = 5;
    static const uint8_t CLUSTERS_LIGHTINDICES = 6;
    static const uint8_t CLUSTERS_LIGHTGRID = 7;
    static const uint8_t CLUSTERS_ATOMICINDEX = 8;
};
