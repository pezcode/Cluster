#pragma once

#include <cstdint>

class Samplers
{
    // these should be the same as in samplers.sh for shaders

public:
    static const uint8_t PBR_BASECOLOR = 0;
    static const uint8_t PBR_METALROUGHNESS = 1;
    static const uint8_t PBR_NORMAL = 2;

    static const uint8_t LIGHTS_POINTLIGHTS = 3;

    static const uint8_t CLUSTERS_CLUSTERS = 4;
    static const uint8_t CLUSTERS_LIGHTINDICES = 5;
    static const uint8_t CLUSTERS_LIGHTGRID = 6;
    static const uint8_t CLUSTERS_ATOMICINDEX = 7;

    static const uint8_t DEFERRED_DIFFUSE_A = 4;
    static const uint8_t DEFERRED_NORMAL = 5;
    static const uint8_t DEFERRED_F0_METALLIC = 6;
    static const uint8_t DEFERRED_DEPTH = 7;
};
