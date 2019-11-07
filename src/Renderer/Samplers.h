#pragma once

#include <cstdint>

class Samplers
{
    // these should be the same as in samplers.sh for shaders

public:
    static const uint8_t PBR_BASECOLOR = 0;
    static const uint8_t PBR_METALROUGHNESS = 1;
    static const uint8_t PBR_NORMAL = 2;
    static const uint8_t PBR_OCCLUSION = 3;
    static const uint8_t PBR_EMISSIVE = 4;

    static const uint8_t LIGHTS_POINTLIGHTS = 5;

    static const uint8_t CLUSTERS_CLUSTERS = 6;
    static const uint8_t CLUSTERS_LIGHTINDICES = 7;
    static const uint8_t CLUSTERS_LIGHTGRID = 8;
    static const uint8_t CLUSTERS_ATOMICINDEX = 9;

    static const uint8_t DEFERRED_DIFFUSE_A = 6;
    static const uint8_t DEFERRED_NORMAL = 7;
    static const uint8_t DEFERRED_F0_METALLIC = 8;
    static const uint8_t DEFERRED_EMISSIVE_OCCLUSION = 9;
    static const uint8_t DEFERRED_DEPTH = 10;
};
