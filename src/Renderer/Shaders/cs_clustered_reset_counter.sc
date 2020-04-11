#define WRITE_CLUSTERS

#include "common.sh"
#include <bgfx_compute.sh>
#include "clusters.sh"

NUM_THREADS(1, 1, 1)
void main()
{
    if(gl_GlobalInvocationID.x == 0)
    {
        // reset the atomic counter for the light grid generation
        // writable compute buffers can't be updated by CPU so do it here
        b_globalIndex[0] = 0;
    }
}
