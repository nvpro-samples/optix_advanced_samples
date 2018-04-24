/* 
 * Copyright (c) 2013-2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include "vertex_attributes.h"

rtBuffer<VertexAttributes> attributesBuffer;
rtBuffer<uint3>            indicesBuffer;

// Axis Aligned Bounding Box routine for indexed interleaved triangle data.
RT_PROGRAM void boundingbox_triangle_indexed(int primitiveIndex, float result[6])
{
  const uint3 indices = indicesBuffer[primitiveIndex];

  const float3 v0 = attributesBuffer[indices.x].vertex;
  const float3 v1 = attributesBuffer[indices.y].vertex;
  const float3 v2 = attributesBuffer[indices.z].vertex;

  const float area = optix::length(optix::cross(v1 - v0, v2 - v0));

  optix::Aabb *aabb = (optix::Aabb *) result;

  if (0.0f < area && !isinf(area))
  {
    aabb->m_min = fminf(fminf(v0, v1), v2);
    aabb->m_max = fmaxf(fmaxf(v0, v1), v2);
  }
  else
  {
    aabb->invalidate();
  }
}
