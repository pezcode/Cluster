#define WRITE_CLUSTERS

#include <bgfx_compute.sh>
#include "clusters.sh"
#include "util.sh"

vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);

// bgfx doesn't define this in shaders
#define gl_WorkGroupSize uvec3(1, 1, 1)
#define gl_NumWorkGroups uvec3(CLUSTERS_X, CLUSTERS_Y, CLUSTERS_Z)

// each thread handles one cluster
NUM_THREADS(1, 1, 1)
void main()
{
    // Per Tile variables
    uint tileSizePx = uint(u_clusterSizes[3]);
    const uint clusterIndex = gl_WorkGroupID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y +
                              gl_WorkGroupID.y * gl_NumWorkGroups.x +
                              gl_WorkGroupID.x;

    // Calculating the min and max point in screen space
    vec4 maxPoint_sS = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * tileSizePx, 1.0, 1.0); // Top Right
    vec4 minPoint_sS = vec4(gl_WorkGroupID.xy * tileSizePx, 1.0, 1.0); // Bottom left

    // Pass min and max to view space
    vec3 maxPoint_vS = screen2Eye(maxPoint_sS).xyz;
    vec3 minPoint_vS = screen2Eye(minPoint_sS).xyz;

    // Near and far values of the cluster in view space
    float tileNear  = u_zNear * pow(u_zFar / u_zNear,  gl_WorkGroupID.z      / float(gl_NumWorkGroups.z));
    float tileFar   = u_zNear * pow(u_zFar / u_zNear, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

    const vec3 eyePos = vec3_splat(0.0);
    // Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
    vec3 minPointNear = lineIntersectionToZPlane(eyePos, minPoint_vS, tileNear);
    vec3 minPointFar  = lineIntersectionToZPlane(eyePos, minPoint_vS, tileFar);
    vec3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileNear);
    vec3 maxPointFar  = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileFar);

    vec3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
    vec3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

    b_clusters[2 * clusterIndex + 0] = vec4(minPointAABB, 0.0);
    b_clusters[2 * clusterIndex + 1] = vec4(maxPointAABB, 0.0);

    // reset the atomic counter for the light culling shader
    // writable compute buffers can't be updated by CPU so do it here
    if(clusterIndex == 0)
    {
        b_globalIndex[0] = 0;
    }
}

// Creates a line from the eye to the screenpoint, then finds its intersection
// With a z oriented plane located at the given distance to the origin
vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance){
    // Because this is a Z based normal this is fixed
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 ab =  B - A;

    // Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    // Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}
