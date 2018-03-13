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

optix::Geometry Application::createPlane(const int tessU, const int tessV, const int upAxis)
{
  MY_ASSERT(1 <= tessU && 1 <= tessV);

  const float uTile = 2.0f / float(tessU);
  const float vTile = 2.0f / float(tessV);
  
  optix::float3 corner;

  std::vector<VertexAttributes> attributes;
  
  VertexAttributes attrib;

  switch (upAxis)
  {
    case 0: // Positive x-axis is the geometry normal, create geometry on the yz-plane.
      corner = optix::make_float3(0.0f, -1.0f, 1.0f); // Lower front corner of the plane. texcoord (0.0f, 0.0f).

      attrib.tangent = optix::make_float3(0.0f, 0.0f, -1.0f);
      attrib.normal  = optix::make_float3(1.0f, 0.0f,  0.0f);

      for (int j = 0; j <= tessV; ++j)
      {
        const float v = float(j) * vTile;

        for (int i = 0; i <= tessU; ++i)
        {
          const float u = float(i) * uTile;

          attrib.vertex   = corner + optix::make_float3(0.0f, v, -u);
          attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

          attributes.push_back(attrib);
        }
      }
      break;

    case 1: // Positive y-axis is the geometry normal, create geometry on the xz-plane.
      corner = optix::make_float3(-1.0f, 0.0f, 1.0f); // left front corner of the plane. texcoord (0.0f, 0.0f).

      attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
      attrib.normal  = optix::make_float3(0.0f, 1.0f, 0.0f);

      for (int j = 0; j <= tessV; ++j)
      {
        const float v = float(j) * vTile;

        for (int i = 0; i <= tessU; ++i)
        {
          const float u = float(i) * uTile;

          attrib.vertex   = corner + optix::make_float3(u, 0.0f, -v);
          attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

          attributes.push_back(attrib);
        }
      }
      break;

    case 2: // Positive z-axis is the geometry normal, create geometry on the xy-plane.
      corner = optix::make_float3(-1.0f, -1.0f, 0.0f); // Lower left corner of the plane. texcoord (0.0f, 0.0f).

      attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
      attrib.normal  = optix::make_float3(0.0f, 0.0f, 1.0f);

      for (int j = 0; j <= tessV; ++j)
      {
        const float v = float(j) * vTile;

        for (int i = 0; i <= tessU; ++i)
        {
          const float u = float(i) * uTile;

          attrib.vertex   = corner + optix::make_float3(u, v, 0.0f);
          attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

          attributes.push_back(attrib);
        }
      }
      break;
  }

  std::vector<unsigned int> indices;
  
  const unsigned int stride = tessU + 1;
  for (int j = 0; j < tessV; ++j)
  {
    for (int i = 0; i < tessU; ++i)
    {
      indices.push_back( j      * stride + i    );
      indices.push_back( j      * stride + i + 1);
      indices.push_back((j + 1) * stride + i + 1);

      indices.push_back((j + 1) * stride + i + 1);
      indices.push_back((j + 1) * stride + i    );
      indices.push_back( j      * stride + i    );
    }
  }

  std::cout << "createPlane(" << upAxis << "): Vertices = " << attributes.size() <<  ", Triangles = " << indices.size() / 3 << std::endl;

  return createGeometry(attributes, indices);
}
