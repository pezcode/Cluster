#define WRITE_CLUSTERS

#include <bgfx_compute.sh>
#include "clusters.sh"
#include "util.sh"

// compute shader to calculate light cluster min/max AABB in eye space
// largely inspired by http://www.aortiz.me/2018/12/21/CG.html
// z-subdivision concept from http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf

// bgfx doesn't define this in shaders
#define gl_WorkGroupSize uvec3(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)

// each thread handles one cluster
NUM_THREADS(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)
void main()
{
    // index calculation must match the inverse operation in the fragment shader (see getClusterIndex)
    uint clusterIndex = gl_GlobalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
                        gl_GlobalInvocationID.y * gl_WorkGroupSize.x +
                        gl_GlobalInvocationID.x;

    // calculate min (bottom left) and max (top right) xy in screen coordinates
    vec4 minScreen = vec4( gl_GlobalInvocationID.xy               * u_clusterSizes.xy, 1.0, 1.0);
    vec4 maxScreen = vec4((gl_GlobalInvocationID.xy + vec2(1, 1)) * u_clusterSizes.xy, 1.0, 1.0);

    // -> eye coordinates
    // z is the camera far plane (1 in screen coordinates)
    vec3 minEye = screen2Eye(minScreen).xyz;
    vec3 maxEye = screen2Eye(maxScreen).xyz;

    // calculate near and far depth edges of the cluster
    float clusterNear = u_zNear * pow(u_zFar / u_zNear,  gl_GlobalInvocationID.z      / float(CLUSTERS_Z));
    float clusterFar  = u_zNear * pow(u_zFar / u_zNear, (gl_GlobalInvocationID.z + 1) / float(CLUSTERS_Z));

    // this calculates the intersection between:
    // - a line from the camera (origin) to the eye point (at the camera's far plane)
    // - the cluster's z-planes (near + far)
    // we could divide by u_zFar as well
    vec3 minNear = minEye * clusterNear / minEye.z;
    vec3 minFar  = minEye * clusterFar  / minEye.z;
    vec3 maxNear = maxEye * clusterNear / maxEye.z;
    vec3 maxFar  = maxEye * clusterFar  / maxEye.z;

    // get extent of the cluster in all dimensions (axis-aligned bounding box)
    // there is some overlap here but it's easier to calculate intersections with AABB
    vec3 minBounds = min(min(minNear, minFar), min(maxNear, maxFar));
    vec3 maxBounds = max(max(minNear, minFar), max(maxNear, maxFar));

    b_clusters[2 * clusterIndex + 0] = vec4(minBounds, 1.0);
    b_clusters[2 * clusterIndex + 1] = vec4(maxBounds, 1.0);
}
