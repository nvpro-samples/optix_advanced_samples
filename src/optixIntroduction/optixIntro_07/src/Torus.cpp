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

#include "inc/Application.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include "inc/MyAssert.h"

optix::Geometry Application::createTorus(const int tessU, const int tessV, const float innerRadius, const float outerRadius)
{
  MY_ASSERT(3 <= tessU && 3 <= tessV); 

  // The torus is a ring with radius outerRadius rotated around the y-axis along the circle with innerRadius.
  /*           +y
      ___       |       ___
    /     \           /     \
   |       |    |    |       |
   |       |         |       |
    \ ___ /     |     \ ___ /
                         <--->
                          outerRadius
                <------->
                innerRadius
  */

  std::vector<VertexAttributes> attributes;
  attributes.reserve((tessU + 1) * (tessV + 1));

  std::vector<unsigned int> indices;
  indices.reserve(8 * tessU * tessV);

  const float u = (float) tessU;
  const float v = (float) tessV;

  float phi_step   = 2.0f * M_PIf / u;
  float theta_step = 2.0f * M_PIf / v;

  // Setup vertices and normals.
  // Generate the torus exactly like the sphere with rings around the origin along the latitudes.
  for (int latitude = 0; latitude <= tessV; ++latitude) // theta angle
  {
    const float theta    = (float) latitude * theta_step;
    const float sinTheta = sinf(theta);
    const float cosTheta = cosf(theta);

    const float radius = innerRadius + outerRadius * cosTheta;

    for (int longitude = 0; longitude <= tessU; ++longitude) // phi angle
    {
      const float phi    = (float) longitude * phi_step;
      const float sinPhi = sinf(phi);
      const float cosPhi = cosf(phi);

      VertexAttributes attrib;

      attrib.vertex   = optix::make_float3(radius * cosPhi, outerRadius * sinTheta, radius * -sinPhi);
      attrib.tangent  = optix::make_float3(-sinPhi, 0.0f, -cosPhi);
      attrib.normal   = optix::make_float3(cosPhi * cosTheta, sinTheta, -sinPhi * cosTheta);
      attrib.texcoord = optix::make_float3((float) longitude / u, (float) latitude / v, 0.0f);

      attributes.push_back(attrib);
    }
  }

  // We have generated tessU + 1 vertices per latitude.
  const unsigned int columns = tessU + 1;

  // Setup indices
  for (int latitude = 0; latitude < tessV; ++latitude ) 
  {
    for (int longitude = 0; longitude < tessU; ++longitude)
    {
      indices.push_back( latitude      * columns + longitude    );  // lower left
      indices.push_back( latitude      * columns + longitude + 1);  // lower right
      indices.push_back((latitude + 1) * columns + longitude + 1);  // upper right

      indices.push_back((latitude + 1) * columns + longitude + 1);  // upper right
      indices.push_back((latitude + 1) * columns + longitude    );  // upper left
      indices.push_back( latitude      * columns + longitude    );  // lower left
    }
  }

  std::cout << "createTorus(): Vertices = " << attributes.size() <<  ", Triangles = " << indices.size() / 3 << std::endl;

  return createGeometry(attributes, indices);
}

