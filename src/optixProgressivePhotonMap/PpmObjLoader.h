
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#pragma once

#include <sutil.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "glm.h"
#include <string>


//-----------------------------------------------------------------------------
// 
//  PpmObjLoader class declaration 
//
//-----------------------------------------------------------------------------

class PpmObjLoader
{
public:
  PpmObjLoader( const std::string& filename,          // Model filename
                optix::Context context,               // Context for RT object creation
                optix::GeometryGroup geometrygroup,   // Empty geom group to hold model
                const std::string& builder);

  PpmObjLoader( const std::string& filename,
                optix::Context context,
                optix::GeometryGroup geometrygroup,
                const std::string& builder,
                optix::Material material );           // Material override

  void load();

  optix::Aabb getSceneBBox()const { return m_aabb; }

  static bool isMyFile( const std::string& filename );

private:

  struct MatParams
  {
    std::string name;
    optix::float3 emissive;
    optix::float3 reflectivity;
    float  phong_exp;
    int    illum;
    optix::float3 Kd;
    optix::float3 Ks;
    optix::TextureSampler ambient_map;
    optix::TextureSampler diffuse_map;
    optix::TextureSampler specular_map;
  };

  void createMaterial();
  void createGeometryInstances( GLMmodel* model,
                                optix::Program mesh_intersect,
                                optix::Program mesh_bbox );
  void loadVertexData( GLMmodel* model );
  void createMaterialParams( GLMmodel* model );
  void loadMaterialParams( optix::GeometryInstance gi, unsigned int index );

  std::string            m_pathname;
  std::string            m_filename;
  optix::Context         m_context;
  optix::GeometryGroup   m_geometrygroup;
  optix::Buffer          m_vbuffer;
  optix::Buffer          m_nbuffer;
  optix::Buffer          m_tbuffer;
  optix::Material        m_material;
  bool                   m_have_default_material;
  optix::Aabb            m_aabb;
  std::vector<MatParams> m_material_params;
  std::string            m_accel_builder;
};
