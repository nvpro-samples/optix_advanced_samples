
#pragma once

#include <optixu/optixu_math_namespace.h>
#include <vector>

struct VoxelModel {
    int dims[3];
    std::vector< optix::uchar4 > voxels;
};

bool readVox( const char* filename, std::vector< VoxelModel >& models, optix::uchar4 palette[256] );


