
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

#include "PpmObjLoader.h"

#include <optixu/optixu_math_namespace.h>
#include <sutil.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string.h>

using namespace optix;


//------------------------------------------------------------------------------
// 
//  Helper functions
//
//------------------------------------------------------------------------------

namespace 
{
  std::string getExtension( const std::string& filename )
  {
    // Get the filename extension
    std::string::size_type extension_index = filename.find_last_of( "." );
    return extension_index != std::string::npos ?
           filename.substr( extension_index+1 ) :
           std::string();
  }
}


//------------------------------------------------------------------------------
// 
//  PpmObjLoader class definition 
//
//------------------------------------------------------------------------------

PpmObjLoader::PpmObjLoader( const std::string& filename,
                            Context context,
                            GeometryGroup geometrygroup,
                            const std::string& builder,
                            Material material )
: m_filename( filename ),
  m_context( context ),
  m_geometrygroup( geometrygroup ),
  m_vbuffer( 0 ),
  m_nbuffer( 0 ),
  m_tbuffer( 0 ),
  m_material( material ),
  m_have_default_material( true ),
  m_aabb(),
  m_accel_builder( builder )
{
  m_pathname = m_filename.substr(0,m_filename.find_last_of("/\\")+1);
}


PpmObjLoader::PpmObjLoader( const std::string& filename,
                            Context context,
                            GeometryGroup geometrygroup,
                            const std::string& builder )
: m_filename( filename ),
  m_context( context ),
  m_geometrygroup( geometrygroup ),
  m_vbuffer( 0 ),
  m_nbuffer( 0 ),
  m_tbuffer( 0 ),
  m_material( 0 ),
  m_have_default_material( false ),
  m_aabb(),
  m_accel_builder( builder )
{
  m_pathname = m_filename.substr(0,m_filename.find_last_of("/\\")+1);
}


void PpmObjLoader::load() 
{
  // parse the OBJ file
  GLMmodel* model = glmReadOBJ( m_filename.c_str() );
  if ( !model ) {
    std::stringstream ss;
    ss << "PpmObjLoader::loadImpl - glmReadOBJ( '" << m_filename << "' ) failed" << std::endl;
    throw std::runtime_error( ss.str() );
  }

  // Create a single material to be shared by all GeometryInstances
  createMaterial(); 
  
  // Create vertex data buffers to be shared by all Geometries
  loadVertexData( model );

  // Load triangle_mesh programs
  std::string path = std::string(sutil::samplesPTXDir()) + "/optixProgressivePhotonMap_generated_triangle_mesh.cu.ptx";
  Program mesh_intersect = m_context->createProgramFromPTXFile( path, "mesh_intersect" );
  Program mesh_bbox      = m_context->createProgramFromPTXFile( path, "mesh_bounds" );

  // Create a GeometryInstance and Geometry for each obj group
  createMaterialParams( model );
  createGeometryInstances( model, mesh_intersect, mesh_bbox );

  glmDelete( model );
}


void PpmObjLoader::createMaterial() 
{
  if ( m_have_default_material ) return;

  std::string path1 = std::string(sutil::samplesPTXDir()) + "/optixProgressivePhotonMap_generated_ppm_rtpass.cu.ptx";
  std::string path2 = std::string(sutil::samplesPTXDir()) + "/optixProgressivePhotonMap_generated_ppm_ppass.cu.ptx";
  std::string path3 = std::string(sutil::samplesPTXDir()) + "/optixProgressivePhotonMap_generated_ppm_gather.cu.ptx";

  Program closest_hit1 = m_context->createProgramFromPTXFile( path1, "rtpass_closest_hit" );
  Program closest_hit2 = m_context->createProgramFromPTXFile( path2, "ppass_closest_hit" );
  Program any_hit      = m_context->createProgramFromPTXFile( path3, "gather_any_hit" );
  m_material           = m_context->createMaterial();
  m_material->setClosestHitProgram( 0u, closest_hit1 );
  m_material->setClosestHitProgram( 1u, closest_hit2 );
  m_material->setAnyHitProgram( 2u, any_hit );
}


void PpmObjLoader::loadVertexData( GLMmodel* model ) 
{
  unsigned int num_vertices  = model->numvertices;
  unsigned int num_texcoords = model->numtexcoords;
  unsigned int num_normals   = model->numnormals;

  // Create vertex buffer
  m_vbuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_vertices );
  float3* vbuffer_data = static_cast<float3*>( m_vbuffer->map() );

  // Create normal buffer
  m_nbuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_normals );
  float3* nbuffer_data = static_cast<float3*>( m_nbuffer->map() );

  // Create texcoord buffer
  m_tbuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, num_texcoords );
  float2* tbuffer_data = static_cast<float2*>( m_tbuffer->map() );

  // Copy vertex, normal and texcoord arrays into buffers 
  memcpy( static_cast<void*>( vbuffer_data ),
          static_cast<void*>( &(model->vertices[3]) ),
          sizeof( float )*num_vertices*3 ); 
  memcpy( static_cast<void*>( nbuffer_data ),
          static_cast<void*>( &(model->normals[3]) ),
          sizeof( float )*num_normals*3 ); 
  memcpy( static_cast<void*>( tbuffer_data ),
          static_cast<void*>( &(model->texcoords[2]) ),
          sizeof( float )*num_texcoords*2 ); 

  m_vbuffer->unmap();
  m_nbuffer->unmap();
  m_tbuffer->unmap();

  // Calculate bbox of model
  for ( unsigned int i = 1u; i <= num_vertices; ++i )
  {
    unsigned int index = i*3u;
    float3 t;
    t.x = model->vertices[ index + 0u ];
    t.y = model->vertices[ index + 1u ];
    t.z = model->vertices[ index + 2u ];

    m_aabb.include( t );
  }
}


void PpmObjLoader::createGeometryInstances( GLMmodel* model,
                                           Program mesh_intersect,
                                           Program mesh_bbox)
{
  std::vector<GeometryInstance> instances;

  // Loop over all groups -- grab the triangles and material props from each group
  unsigned int triangle_count = 0u;
  unsigned int group_count = 0u;
  for ( GLMgroup* obj_group = model->groups;
        obj_group != 0;
        obj_group = obj_group->next, group_count++ ) {

    unsigned int num_triangles = obj_group->numtriangles;
    if ( num_triangles == 0 ) continue; 

    // Create vertex index buffers
    Buffer vindex_buffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, num_triangles );
    int3* vindex_buffer_data = static_cast<int3*>( vindex_buffer->map() );

    Buffer tindex_buffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, num_triangles );
    int3* tindex_buffer_data = static_cast<int3*>( tindex_buffer->map() );

    Buffer nindex_buffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, num_triangles );
    int3* nindex_buffer_data = static_cast<int3*>( nindex_buffer->map() );

    // TODO: Create empty buffer for mat indices, have obj_material check for zero length
    Buffer mbuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT, num_triangles );
    uint* mbuffer_data = static_cast<uint*>( mbuffer->map() );

    // Create the mesh object
    Geometry mesh = m_context->createGeometry();
    mesh->setPrimitiveCount( num_triangles );
    mesh->setIntersectionProgram( mesh_intersect);
    mesh->setBoundingBoxProgram( mesh_bbox );
    mesh[ "vertex_buffer" ]->setBuffer( m_vbuffer );
    mesh[ "normal_buffer" ]->setBuffer( m_nbuffer );
    mesh[ "texcoord_buffer" ]->setBuffer( m_tbuffer );
    mesh[ "vindex_buffer" ]->setBuffer( vindex_buffer );
    mesh[ "tindex_buffer" ]->setBuffer( tindex_buffer );
    mesh[ "nindex_buffer" ]->setBuffer( nindex_buffer );
    mesh[ "material_buffer" ]->setBuffer( mbuffer );

    // Create the geom instance to hold mesh and material params
    GeometryInstance instance = m_context->createGeometryInstance( mesh, &m_material, &m_material+1 );
    loadMaterialParams( instance, obj_group->material );
    instances.push_back( instance );

    for ( unsigned int i = 0; i < obj_group->numtriangles; ++i, ++triangle_count ) {

      unsigned int tindex = obj_group->triangles[i];
      int3 vindices;
      vindices.x = model->triangles[ tindex ].vindices[0] - 1; 
      vindices.y = model->triangles[ tindex ].vindices[1] - 1; 
      vindices.z = model->triangles[ tindex ].vindices[2] - 1; 
      
      int3 nindices;
      nindices.x = model->triangles[ tindex ].nindices[0] - 1; 
      nindices.y = model->triangles[ tindex ].nindices[1] - 1; 
      nindices.z = model->triangles[ tindex ].nindices[2] - 1; 

      int3 tindices;
      tindices.x = model->triangles[ tindex ].tindices[0] - 1; 
      tindices.y = model->triangles[ tindex ].tindices[1] - 1; 
      tindices.z = model->triangles[ tindex ].tindices[2] - 1; 

      vindex_buffer_data[ i ] = vindices;
      nindex_buffer_data[ i ] = nindices;
      tindex_buffer_data[ i ] = tindices;
      mbuffer_data[ i ] = 0; // See above TODO
    }

    vindex_buffer->unmap();
    tindex_buffer->unmap();
    nindex_buffer->unmap();
    mbuffer->unmap();
  }

  assert( triangle_count == model->numtriangles );
  
  // Set up group 
  m_geometrygroup->setChildCount( static_cast<unsigned int>(instances.size()) );
  Acceleration acceleration = m_context->createAcceleration(m_accel_builder.c_str() );
  acceleration->setProperty( "vertex_buffer_name", "vertex_buffer" );
  acceleration->setProperty( "index_buffer_name", "vindex_buffer" );
  m_geometrygroup->setAcceleration( acceleration );
  acceleration->markDirty();


  for ( unsigned int i = 0; i < instances.size(); ++i )
    m_geometrygroup->setChild( i, instances[i] );
}


bool PpmObjLoader::isMyFile( const std::string& filename )
{
  return getExtension( filename ) == "obj";
}


void PpmObjLoader::loadMaterialParams( GeometryInstance gi, unsigned int index )
{
  // We dont need any material params if we have default material
  if ( m_have_default_material ) {
    return;
  }

  // If no materials were given in model use reasonable defaults
  if ( m_material_params.empty() ) {
    std::cerr << " PpmPpmObjLoader not setup to use material override yet! " << std::endl;
    gi[ "emissive" ]->setFloat( 0.0f, 0.0f, 0.0f );
    gi[ "phong_exp" ]->setFloat( 32.0f );
    gi[ "reflectivity" ]->setFloat( 0.3f, 0.3f, 0.3f );
    gi[ "illum" ]->setInt( 2 );

    gi["ambient_map"]->setTextureSampler( sutil::loadTexture( m_context, "", make_float3( 0.2f, 0.2f, 0.2f ) ) );
    gi["diffuse_map"]->setTextureSampler( sutil::loadTexture( m_context, "", make_float3( 0.8f, 0.8f, 0.8f ) ) );
    gi["specular_map"]->setTextureSampler( sutil::loadTexture( m_context, "", make_float3( 0.0f, 0.0f, 0.0f ) ) );
    return;
  }

  // Load params from this material into the GI 
  if ( index < m_material_params.size() ) {
    MatParams& mp = m_material_params[index];
    gi[ "emitted" ]->setFloat( 0.0f, 0.0f, 0.0f );
    gi[ "Kd"  ]->setFloat( mp.Kd );
    gi[ "Ks"  ]->setFloat( mp.Ks );
    gi[ "grid_color"  ]->setFloat( 0.5f, 0.5f, 0.5f );
    gi[ "use_grid"  ]->setUint( mp.name == "01_-_Default" ? 1u : 0 );
    return;
  }

  // Should never reach this point
  std::cerr << "WARNING -- PpmObjLoader::loadMaterialParams given index out of range: "
            << index << std::endl;
}


void PpmObjLoader::createMaterialParams( GLMmodel* model )
{
  m_material_params.resize( model->nummaterials );
  for ( unsigned int i = 0; i < model->nummaterials; ++i ) {

    GLMmaterial& mat = model->materials[i];
    MatParams& params = m_material_params[i];

    /*
    params.emissive     = make_float3( mat.emmissive[0], mat.emmissive[1], mat.emmissive[2] );
    params.reflectivity = make_float3( mat.specular[0], mat.specular[1], mat.specular[2] );
    params.phong_exp    = mat.shininess; 
    params.illum        = ( (mat.shader > 3) ? 2 : mat.shader ); // use 2 as default if out-of-range
    */

    float3 Kd = make_float3( mat.diffuse[0],
                             mat.diffuse[1],
                             mat.diffuse[2] );
    //float3 Ka = make_float3( mat.ambient[0],
    //                         mat.ambient[1],
    //                         mat.ambient[2] );
    float3 Ks = make_float3( mat.specular[0],
                             mat.specular[1],
                             mat.specular[2] );
    params.Kd = Kd;
    params.Ks = Ks;
    params.name = mat.name;

    /*
    // load textures relatively to OBJ main file
    std::string ambient_map  = strlen(mat.ambient_map)  ? m_pathname + mat.ambient_map  : "";
    std::string diffuse_map  = strlen(mat.diffuse_map)  ? m_pathname + mat.diffuse_map  : "";
    std::string specular_map = strlen(mat.specular_map) ? m_pathname + mat.specular_map : "";

    params.ambient_map = loadTexture( m_context, ambient_map, Ka );
    params.diffuse_map = loadTexture( m_context, diffuse_map, Kd );
    params.specular_map = loadTexture( m_context, specular_map, Ks );
    */
  }
}
