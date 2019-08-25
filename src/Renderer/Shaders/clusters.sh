#ifndef CLUSTERS_SH_HEADER_GUARD
#define CLUSTERS_SH_HEADER_GUARD

#include "samplers.sh"

// taken from Doom
// http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf
#define CLUSTERS_X 16
#define CLUSTERS_Y 8
#define CLUSTERS_Z 24

#define CLUSTERS_Z_THREADS 8

#define MAX_LIGHTS_PER_CLUSTER 100

//uniform mat4 u_inverseProjection;
uniform vec4 u_clusterSizes;
//uniform vec4 u_screenDimensions;

uniform vec4 u_zNearFarVec;
#define u_zNear u_zNearFarVec.x
#define u_zFar u_zNearFarVec.y

#ifdef WRITE_CLUSTERS
    #define CLUSTER_BUFFER BUFFER_RW
#else
    #define CLUSTER_BUFFER BUFFER_RO
#endif

// light indices belonging to clusters
CLUSTER_BUFFER(b_clusterLightIndices, uint, SAMPLER_CLUSTERS_LIGHTINDICES);
// for each cluster: (start index in b_clusterLightIndices, number of point lights, empty, empty)
CLUSTER_BUFFER(b_clusterLightGrid, uint, SAMPLER_CLUSTERS_LIGHTGRID);
// uvec4 doesn't seem to work
//CLUSTER_BUFFER(b_clusterLightGrid, uvec4, SAMPLER_CLUSTERS_LIGHTGRID);

// these are only needed for building clusters and light culling, not in the fragment shader
#ifdef WRITE_CLUSTERS
// list of clusters (2 vec4's each, min + max pos)
CLUSTER_BUFFER(b_clusters, vec4, SAMPLER_CLUSTERS_CLUSTERS);
// atomic counter for building the light grid
// must be reset to 0 every frame
CLUSTER_BUFFER(b_globalIndex, uint, SAMPLER_CLUSTERS_ATOMICINDEX);
#endif

struct Cluster
{
    vec3 minPos;
    vec3 maxPos;
};

struct LightGrid
{
    uint offset;
    uint pointLights;
};

#ifdef WRITE_CLUSTERS
Cluster getCluster(uint index)
{
    Cluster cluster;
    cluster.minPos = b_clusters[2 * index + 0].xyz;
    cluster.maxPos = b_clusters[2 * index + 1].xyz;
    return cluster;
}
#endif

LightGrid getLightGrid(uint cluster)
{
    //uvec4 gridvec = b_clusterLightGrid[cluster];
    uvec4 gridvec = uvec4(b_clusterLightGrid[4 * cluster + 0], b_clusterLightGrid[4 * cluster + 1], 0, 0);
    LightGrid grid;
    grid.offset = gridvec.x;
    grid.pointLights = gridvec.y;
    return grid;
}

uint getGridLightIndex(uint start, uint offset)
{
    return b_clusterLightIndices[start + offset];
}

/*
uint clusterZIndex(float z)
{
    return uint(log(z) / CLUSTERS_Z);
}

uint clusterIndex(uint x, uint y, uint z)
{
    return (z * CLUSTERS_X * CLUSTERS_Y) + (y * CLUSTERS_X) + x;
}
*/

#endif // CLUSTERS_SH_HEADER_GUARD
