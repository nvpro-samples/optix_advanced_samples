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
#include <optixu/optixu_math_namespace.h>

#include "vertex_attributes.h"

rtBuffer<VertexAttributes> attributesBuffer;
rtBuffer<uint3>            indicesBuffer;

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, ); 
rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );

// Intersection routine for indexed interleaved triangle data.
RT_PROGRAM void intersection_triangle_indexed(int primitiveIndex)
{
  const uint3 indices = indicesBuffer[primitiveIndex];

  VertexAttributes const& a0 = attributesBuffer[indices.x];
  VertexAttributes const& a1 = attributesBuffer[indices.y];
  VertexAttributes const& a2 = attributesBuffer[indices.z];

  const float3 v0 = a0.vertex;
  const float3 v1 = a1.vertex;
  const float3 v2 = a2.vertex;

  float3 n;
  float  t;
  float  beta;
  float  gamma;

  if (intersect_triangle(theRay, v0, v1, v2, n, t, beta, gamma))
  {
    if (rtPotentialIntersection(t))
    {
      // Barycentric interpolation:
      const float alpha = 1.0f - beta - gamma;

      // Note: No normalization on the TBN attributes here for performance reasons.
      //       It's done after the transformation into world space anyway.
      varGeoNormal      = n;
      varTangent        = a0.tangent  * alpha + a1.tangent  * beta + a2.tangent  * gamma;
      varNormal         = a0.normal   * alpha + a1.normal   * beta + a2.normal   * gamma;
      varTexCoord       = a0.texcoord * alpha + a1.texcoord * beta + a2.texcoord * gamma;
      
      rtReportIntersection(0);
    }
  }
}
