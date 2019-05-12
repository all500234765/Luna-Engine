#include "shMarchTablesCS.hlsl"

static const int numThreads = 8;

struct Triangle {
    float3 vertexC;
    float3 vertexB;
    float3 vertexA;
};

AppendStructuredBuffer<Triangle> triangles;
RWStructuredBuffer<float4> points;

int numPointsPerAxis;
float isoLevel;

float3 interpolateVerts(float4 v1, float4 v2) {
    float t = (isoLevel - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

int indexFromCoord(int x, int y, int z) {
    return z * numPointsPerAxis * numPointsPerAxis + y * numPointsPerAxis + x;
}

[numthreads(numThreads, numThreads, numThreads)]
void main(int3 id : SV_DispatchThreadID) {
    // Stop one point before the end because voxel includes neighbouring points
    if( id.x >= numPointsPerAxis - 1 || id.y >= numPointsPerAxis - 1 || id.z >= numPointsPerAxis - 1 ) {
        return;
    }

    // 8 corners of the current cube
    float4 cubeCorners[8] = {
        points[indexFromCoord(id.x, id.y, id.z)],
        points[indexFromCoord(id.x + 1, id.y, id.z)],
        points[indexFromCoord(id.x + 1, id.y, id.z + 1)],
        points[indexFromCoord(id.x, id.y, id.z + 1)],
        points[indexFromCoord(id.x, id.y + 1, id.z)],
        points[indexFromCoord(id.x + 1, id.y + 1, id.z)],
        points[indexFromCoord(id.x + 1, id.y + 1, id.z + 1)],
        points[indexFromCoord(id.x, id.y + 1, id.z + 1)]
    };

    // Calculate unique index for each cube configuration.
    // There are 256 possible values
    // A value of 0 means cube is entirely inside surface; 255 entirely outside.
    // The value is used to look up the edge table, which indicates which edges of the cube are cut by the isosurface.
    int cubeIndex = 0;
    if( cubeCorners[0].w < isoLevel ) cubeIndex |= 1;
    if( cubeCorners[1].w < isoLevel ) cubeIndex |= 2;
    if( cubeCorners[2].w < isoLevel ) cubeIndex |= 4;
    if( cubeCorners[3].w < isoLevel ) cubeIndex |= 8;
    if( cubeCorners[4].w < isoLevel ) cubeIndex |= 16;
    if( cubeCorners[5].w < isoLevel ) cubeIndex |= 32;
    if( cubeCorners[6].w < isoLevel ) cubeIndex |= 64;
    if( cubeCorners[7].w < isoLevel ) cubeIndex |= 128;

    // Create triangles for current cube configuration
    for( int i = 0; triangulation[cubeIndex][i] != -1; i += 3 ) {
        // Get indices of corner points A and B for each of the three edges
        // of the cube that need to be joined to form the triangle.
        int a0 = cornerIndexAFromEdge[triangulation[cubeIndex][i]];
        int b0 = cornerIndexBFromEdge[triangulation[cubeIndex][i]];

        int a1 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 1]];
        int b1 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 1]];

        int a2 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 2]];
        int b2 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 2]];

        Triangle tri;
        tri.vertexA = interpolateVerts(cubeCorners[a0], cubeCorners[b0]);
        tri.vertexB = interpolateVerts(cubeCorners[a1], cubeCorners[b1]);
        tri.vertexC = interpolateVerts(cubeCorners[a2], cubeCorners[b2]);
        triangles.Append(tri);
    }


}
