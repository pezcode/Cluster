#define WRITE_CLUSTERS

#include "common.sh"
#include <bgfx_compute.sh>
#include "lights.sh"
#include "clusters.sh"

// compute shader to cull lights against cluster bounds
// builds a light grid that holds indices of lights for each cluster
// largely inspired by http://www.aortiz.me/2018/12/21/CG.html

// point lights only for now
bool pointLightIntersectsCluster(PointLight light, Cluster cluster);

#define gl_WorkGroupSize uvec3(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)
#define GROUP_SIZE (CLUSTERS_X_THREADS * CLUSTERS_Y_THREADS * CLUSTERS_Z_THREADS)

// light cache for the current workgroup
// group shared memory has lower latency than global memory

// there's no guarantee on the available shared memory
// as a guideline the minimum value of GL_MAX_COMPUTE_SHARED_MEMORY_SIZE is 32KB
// with a workgroup size of 16*8*4 this is 64 bytes per light
// however, using all available memory would limit the compute shader invocation to only 1 workgroup
SHARED PointLight lights[GROUP_SIZE];

// each thread handles one cluster
NUM_THREADS(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)
void main()
{
    // local thread variables
    // hold the result of light culling for this cluster
    uint visibleLights[MAX_LIGHTS_PER_CLUSTER];
    uint visibleCount = 0;

    // the way we calculate the index doesn't really matter here since we write to the same index in the light grid as we read from the cluster buffer
    uint clusterIndex = gl_GlobalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
                        gl_GlobalInvocationID.y * gl_WorkGroupSize.x +
                        gl_GlobalInvocationID.x;

    Cluster cluster = getCluster(clusterIndex);
    
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
            PointLight light = getPointLight(lightIndex);
            // transform to view space (expected by pointLightAffectsCluster)
            // do it here once rather than for each cluster later
            light.position = mul(u_view, vec4(light.position, 1.0)).xyz;
            lights[gl_LocalInvocationIndex] = light;
        }

        // wait for all threads to finish copying
        barrier();

        // each thread is one cluster and checks against all lights in the cache
        for(uint i = 0; i < batchSize; i++)
        {
            Cluster cluster = getCluster(clusterIndex);
            if(visibleCount < MAX_LIGHTS_PER_CLUSTER && pointLightIntersectsCluster(lights[i], cluster))
            {
                visibleLights[visibleCount] = lightOffset + i;
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
    b_clusterLightGrid[clusterIndex] = uvec4(offset, visibleCount, 0, 0);
}

// check if light radius extends into the cluster
bool pointLightIntersectsCluster(PointLight light, Cluster cluster)
{
    // NOTE: expects light.position to be in view space like the cluster bounds
    // global light list has world space coordinates, but we transform the
    // coordinates in the shared array of lights after copying

    // get closest point to sphere center
    vec3 closest = max(cluster.minBounds, min(light.position, cluster.maxBounds));
    // check if point is inside the sphere
    vec3 dist = closest - light.position;
    return dot(dist, dist) <= (light.radius * light.radius);    
}
