#ifndef CLUSTERS_SH_HEADER_GUARD
#define CLUSTERS_SH_HEADER_GUARD

#define CLUSTER_WIDTH  32
#define CLUSTER_HEIGHT 32
#define DEPTH_SLICES 10

uint clusterZIndex(float z)
{
    return uint(log(z) / DEPTH_SLICES);
}

uint clusterIndex(float x, float y, float z)
{
    // TODO
    return 0;
}

#endif // CLUSTERS_SH_HEADER_GUARD
