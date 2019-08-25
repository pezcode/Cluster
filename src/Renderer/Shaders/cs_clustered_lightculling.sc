#define WRITE_CLUSTERS

#include <bgfx_compute.sh>
#include "lights.sh"
#include "clusters.sh"

// point lights only for now

bool pointLightAffectsCluster(PointLight light, Cluster cluster);
float distsqToCluster(vec3 pos, Cluster cluster);

// bgfx doesn't define this in shaders
#define gl_WorkGroupSize uvec3(CLUSTERS_X, CLUSTERS_Y, CLUSTERS_Z_THREADS)
#define GROUP_SIZE (CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z_THREADS)

// light cache for the current work group
SHARED PointLight lights[GROUP_SIZE];

// work group size
// each thread handles one cluster
// D3D compute shaders only seem to allow 1024 threads
NUM_THREADS(CLUSTERS_X, CLUSTERS_Y, CLUSTERS_Z_THREADS)
void main()
{
    // local thread variables
    // hold the result of light culling for this cluster
    uint visibleLights[MAX_LIGHTS_PER_CLUSTER];
    uint visibleCount = 0;

    // the way we calculate the index doesn't really matter here since we write to the same index as we read from the cluster buffer
    // it only matters that the cluster buildung and fragment shader calculate the cluster index the same way
    const uint clusterIndex = gl_GlobalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
                              gl_GlobalInvocationID.y * gl_WorkGroupSize.x +
                              gl_GlobalInvocationID.x;

    // we have a cache of GROUP_SIZE lights
    // have to run this loop several times if we have more than GROUP_SIZE lights
    uint lightCount = pointLightCount();
    uint lightOffset = 0;
    while(lightOffset < lightCount)
    {
        // read GROUP_SIZE lights into shared memory
        // each thread copies one light
        uint batchSize = min(GROUP_SIZE, lightCount - lightOffset);

        if(uint(gl_LocalInvocationIndex) < batchSize)
        {
            uint lightIndex = lightOffset + gl_LocalInvocationIndex;
            lights[gl_LocalInvocationIndex] = getPointLight(lightIndex);
        }

        // wait for all threads to finish copying
        barrier();

        // each thread is one cluster and checks against all lights in the cache
        for(uint j = 0; j < batchSize; j++)
        {
            uint lightIndex = lightOffset + j;
            Cluster cluster = getCluster(clusterIndex);
            if(visibleCount < MAX_LIGHTS_PER_CLUSTER && pointLightAffectsCluster(lights[lightIndex], cluster))
            {
                visibleLights[visibleCount] = lightIndex;
                visibleCount++;
            }
        }

        lightOffset += batchSize;
    }

    // wait for all threads to finish checking lights
    barrier();

    // get a unique index into the light index list where we can write this cluster's lights
    uint offset = 0;
    atomicFetchAndAdd(b_globalIndex[0], visibleCount, offset);
    // copy indices of lights
    for(uint i = 0; i < visibleCount; i++)
    {
        b_clusterLightIndices[offset + i] = visibleLights[i];
    }

    // write light grid for this cluster
    //b_clusterLightGrid[clusterIndex] = uvec4(offset, visibleCount, 0, 0);
    b_clusterLightGrid[4 * clusterIndex + 0] = offset;
    b_clusterLightGrid[4 * clusterIndex + 1] = visibleCount;
    b_clusterLightGrid[4 * clusterIndex + 2] = 0;
    b_clusterLightGrid[4 * clusterIndex + 3] = 0;
}

// check point light radius against cluster bounds
bool pointLightAffectsCluster(PointLight light, Cluster cluster)
{
    vec3 pos = mul(u_view, vec4(light.position, 1.0)).xyz;
    return distsqToCluster(pos, cluster) <= (light.radius * light.radius);
    //return true; // DEBUG
}

// squared distance of the point to the bounding planes
float distsqToCluster(vec3 pos, Cluster cluster)
{
    float distsq = 0.0;
    for(uint i = 0; i < 3; ++i)
    {
        float v = pos[i];
        if(v < cluster.minPos[i])
        {
            distsq += (cluster.minPos[i] - v) * (cluster.minPos[i] - v);
        }
        else if(v > cluster.maxPos[i])
        {
            distsq += (v - cluster.maxPos[i]) * (v - cluster.maxPos[i]);
        }
    }

    return distsq;
}
