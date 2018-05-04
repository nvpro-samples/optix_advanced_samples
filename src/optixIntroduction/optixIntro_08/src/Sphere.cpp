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

optix::Geometry Application::createSphere(const int tessU, const int tessV, const float radius, const float maxTheta)
{
  MY_ASSERT(3 <= tessU && 3 <= tessV);

  std::vector<VertexAttributes> attributes;
  attributes.reserve((tessU + 1) * tessV);

  std::vector<unsigned int> indices;
  indices.reserve(6 * tessU * (tessV - 1));

  float phi_step   = 2.0f * M_PIf / (float) tessU;
  float theta_step = maxTheta / (float) (tessV - 1);

  // Latitudinal rings.
  // Starting at the south pole going upwards on the y-axis.
  for (int latitude = 0; latitude < tessV; latitude++) // theta angle
  {
    float theta    = (float) latitude * theta_step;
    float sinTheta = sinf(theta);
    float cosTheta = cosf(theta);
    
    float texv = (float) latitude / (float) (tessV - 1); // Range [0.0f, 1.0f]

    // Generate vertices along the latitudinal rings.
    // On each latitude there are tessU + 1 vertices.
    // The last one and the first one are on identical positions, but have different texture coordinates!
    // DAR FIXME Note that each second triangle connected to the two poles has zero area!
    for (int longitude = 0; longitude <= tessU; longitude++) // phi angle
    {
      float phi    = (float) longitude * phi_step;
      float sinPhi = sinf(phi);
      float cosPhi = cosf(phi);
      
      float texu = (float) longitude / (float) tessU; // Range [0.0f, 1.0f]
        
      // Unit sphere coordinates are the normals.
      optix::float3 normal = optix::make_float3( cosPhi * sinTheta, 
                                                -cosTheta,                 // -y to start at the south pole.
                                                -sinPhi * sinTheta);
      VertexAttributes attrib;

      attrib.vertex   = normal * radius;
      attrib.tangent  = optix::make_float3(-sinPhi, 0.0f, -cosPhi);
      attrib.normal   = normal;
      attrib.texcoord = optix::make_float3(texu, texv, 0.0f);

      attributes.push_back(attrib);
    }
  }
    
  // We have generated tessU + 1 vertices per latitude.
  const unsigned int columns = tessU + 1;

  // Calculate indices.
  for (int latitude = 0 ; latitude < tessV - 1 ; latitude++)
  {                                           
    for (int longitude = 0 ; longitude < tessU ; longitude++)
    {
      indices.push_back( latitude      * columns + longitude    );  // lower left
      indices.push_back( latitude      * columns + longitude + 1);  // lower right
      indices.push_back((latitude + 1) * columns + longitude + 1);  // upper right 

      indices.push_back((latitude + 1) * columns + longitude + 1);  // upper right 
      indices.push_back((latitude + 1) * columns + longitude    );  // upper left
      indices.push_back( latitude      * columns + longitude    );  // lower left
    }
  }
  
  std::cout << "createSphere(): Vertices = " << attributes.size() <<  ", Triangles = " << indices.size() / 3 << std::endl;

  return createGeometry(attributes, indices);
}
