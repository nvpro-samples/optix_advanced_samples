// NOTE: the '_shared' suffix is meant to convey that this header is used in 
// both host and device code.
// TODO: This file is at present shipped to the public headers folder alongside
// optixu_high_level.h.  That may be appropriate for much of its contents for
// users who wish to write their own OptiX functionality alongside what the
// API does for them, but at some point we need to make an actual design decision
// about what in here SHOULD be pushed to the public interface, and which are
// just for internal 'shared' functionality.

// TODO: Make this work for inclusion in both C and C++ files
#ifndef __optixu_high_level_shared_h__
#define __optixu_high_level_shared_h__

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <vector_types.h>


// TODO: Find any lingering magic numbers that should be replaced with these.
/**
 * RTH ray types
 */
const unsigned int RTH_RAY_TYPE_RADIANCE = 0;
const unsigned int RTH_RAY_TYPE_SHADOW = 1;

/**
  * Specifies a format for scanning a buffer of triangle data, with an offset
  * and stride for each vertex attribute: vertex (position), normal, color,
  * and texture coordinates
  */
struct RTHvertexBufferFormat {
  unsigned int      vertex_offset;
  unsigned int      vertex_stride;
  unsigned int      normal_offset;
  unsigned int      normal_stride;
  unsigned int      texcoords_offset;
  unsigned int      texcoords_stride;
  unsigned int      color_offset;
  unsigned int      color_stride;
};

/**
  * Set strides and offsets to this value to mark them as not per vertex.
  */
const unsigned int RTH_UNUSED = 0x7fffffffu;

struct RTHsphereBufferFormat {
  unsigned int center_offset;
  unsigned int center_stride;
  unsigned int radius_offset;
  unsigned int radius_stride;
  unsigned int color_offset;
  unsigned int color_stride;
};


// TODO: rename these structs with RTH prefix
/**
  * The per-ray payload used across optix-high-level-renderer
  */
struct RadianceRayPayload {
  optix::float3 result;
  float importance;
  int depth;
};

struct ShadowRayPayload {
  optix::float3 attenuation;
};

/* Lights */

/**
* \ingroup rthRender
* \brief Structure encapsulating a light
*/

typedef struct struct_RTHpointLight
{
  optix::float3 pos;
  optix::float3 color;
  int casts_shadow;
  int padding;  // Make this struct 32 bytes
} RTHpointLight;


// TODO: These helper functions might not make sense for inclusion in CUDA/device code,
// or might need revision in order to work
__host__ __device__ __inline__ int rthVertexBufferFormatUsesAttribute(
  unsigned int              attribute_offset,
  unsigned int              attribute_stride)
{
  return attribute_offset != RTH_UNUSED
      && attribute_stride != RTH_UNUSED;
}

__host__ __device__ __inline__ int rthVertexBufferFormatUsesColor(
  RTHvertexBufferFormat     format)
{
  return rthVertexBufferFormatUsesAttribute(format.color_offset, format.color_stride);
}

__host__ __device__ __inline__ int rthVertexBufferFormatUsesNormal(
  RTHvertexBufferFormat     format)
{
  return rthVertexBufferFormatUsesAttribute(format.normal_offset, format.normal_stride);
}

__host__ __device__ __inline__ int rthVertexBufferFormatUsesTexcoords(
  RTHvertexBufferFormat     format)
{
  return rthVertexBufferFormatUsesAttribute(format.texcoords_offset, format.texcoords_stride);
}

#endif /* __optixu_high_level_shared_h__ */
