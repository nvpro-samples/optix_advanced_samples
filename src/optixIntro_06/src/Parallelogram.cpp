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

// Parallelogram from footpoint position, spanned by unnormalized vectors vecU and vecV, normal is normalized and on the CCW frontface.
optix::Geometry Application::createParallelogram(optix::float3 const& position, optix::float3 const& vecU, optix::float3 const& vecV, optix::float3 const& normal)
{
  std::vector<VertexAttributes> attributes;
  
  VertexAttributes attrib;

  // Same for all four vertices in this parallelogram.
  attrib.tangent   = optix::normalize(vecU);
  attrib.normal    = normal;
  
  attrib.vertex    = position; // left bottom
  attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
  attributes.push_back(attrib);

  attrib.vertex    = position + vecU; // right bottom
  attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
  attributes.push_back(attrib);

  attrib.vertex    = position + vecU + vecV; // right top
  attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
  attributes.push_back(attrib);

  attrib.vertex    = position + vecV; // left top
  attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
  attributes.push_back(attrib);

  std::vector<unsigned int> indices;

  indices.push_back(0);
  indices.push_back(1);
  indices.push_back(2);

  indices.push_back(2);
  indices.push_back(3);
  indices.push_back(0);

  std::cout << "createParallelogram(): Vertices = " << attributes.size() <<  ", Triangles = " << indices.size() / 3 << std::endl;

  return createGeometry(attributes, indices);
}
