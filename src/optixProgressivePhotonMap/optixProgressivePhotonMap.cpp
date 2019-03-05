/* 
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
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

//-----------------------------------------------------------------------------
//
// optixProgressivePhotonMap: progressive photon mapping scene
//
//-----------------------------------------------------------------------------

#ifndef __APPLE__
#  include <GL/glew.h>
#  if defined( _WIN32 )
#    include <GL/wglew.h>
#  endif
#endif

#include <GLFW/glfw3.h>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>

// from sutil
#include <sutil.h>
#include <Camera.h>

#include "Mesh.h"
#include "ppm.h"
#include "random.h"
#include "select.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl2.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdint.h>

using namespace optix;

const char* const SAMPLE_NAME = "optixProgressivePhotonMap";
const unsigned int WIDTH  = 768u;
const unsigned int HEIGHT = 768u;
const unsigned int MAX_PHOTON_COUNT = 2u;
const unsigned int PHOTON_LAUNCH_DIM = 512u;
const float LIGHT_THETA = 1.15f;
const float LIGHT_PHI = 2.19f;

enum SplitChoice {
  RoundRobin,
  HighestVariance,
  LongestDim
};

//------------------------------------------------------------------------------
//
// Globals
//
//------------------------------------------------------------------------------

Context      context = 0;

bool s_display_debug_buffer = false;
bool s_print_timings = false;


//------------------------------------------------------------------------------
//
//  Helper functions
//
//------------------------------------------------------------------------------
    

// Finds the smallest power of 2 greater or equal to x.
static unsigned int pow2roundup(unsigned int x)
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x+1;
}

static int max_component(float3 a)
{
  if(a.x > a.y) {
    if(a.x > a.z) {
      return 0;
    } else {
      return 2;
    }
  } else {
    if(a.y > a.z) {
      return 1;
    } else {
      return 2;
    }
  }
}

static float3 sphericalToCartesian( float theta, float phi )
{
  float cos_theta = cosf( theta );
  float sin_theta = sinf( theta );
  float cos_phi = cosf( phi );
  float sin_phi = sinf( phi );
  float3 v;
  v.x = cos_phi * sin_theta;
  v.z = sin_phi * sin_theta;
  v.y = cos_theta;
  return v;
}

static std::string ptxPath( const std::string& cuda_file )
{
    return
        std::string(sutil::samplesPTXDir()) +
        "/" + std::string(SAMPLE_NAME) + "_generated_" +
        cuda_file +
        ".ptx";
}


static Buffer getOutputBuffer()
{
    return context[ "output_buffer" ]->getBuffer();
}


void destroyContext()
{
    if( context )
    {
        context->destroy();
        context = 0;
    }
}

enum ProgramEnum {
    rtpass,
    ppass,
    gather,
    NUM_PROGRAMS
};

void createContext( bool use_pbo, unsigned int photon_launch_dim, Buffer& photons_buffer, Buffer& photon_map_buffer )
{
    // Set up context
    context = Context::create();

    // There's a performance advantage to using a device that isn't being used as a display.
    // We'll take a guess and pick the second GPU if the second one has the same compute
    // capability as the first.
    int deviceId = 0;
    int computeCaps[2];
    if (RTresult code = rtDeviceGetAttribute(0, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCaps), &computeCaps))
        throw Exception::makeException(code, 0);
    for(unsigned int index = 1; index < Context::getDeviceCount(); ++index) {
        int computeCapsB[2];
        if (RTresult code = rtDeviceGetAttribute(index, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCaps), &computeCapsB))
            throw Exception::makeException(code, 0);
        if (computeCaps[0] == computeCapsB[0] && computeCaps[1] == computeCapsB[1]) {
            deviceId = index;
            break;
        }
    }
    context->setDevices(&deviceId, &deviceId+1);

    context->setRayTypeCount( 3 );
    context->setEntryPointCount( NUM_PROGRAMS );
    context->setStackSize( 800 );

    context["max_depth"]->setUint( 3u );
    context["max_photon_count"]->setUint( MAX_PHOTON_COUNT );

    context["scene_epsilon"]->setFloat( 1.e-1f );
    context["alpha"]->setFloat( 0.7f );
    context["total_emitted"]->setFloat( 0.0f );
    context["frame_number"]->setFloat( 0.0f );
    context["use_debug_buffer"]->setUint( s_display_debug_buffer );

    Buffer buffer = sutil::createOutputBuffer( context, RT_FORMAT_FLOAT4, WIDTH, HEIGHT, use_pbo );
    context["output_buffer"]->set( buffer );

    // Debug output buffer
    Buffer debug_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, WIDTH, HEIGHT );
    context["debug_buffer"]->set( debug_buffer );

    // RTPass output buffer
    Buffer rtpass_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_USER, WIDTH, HEIGHT );
    rtpass_buffer->setElementSize( sizeof( HitRecord ) );
    context["rtpass_output_buffer"]->set( rtpass_buffer );

    // RTPass pixel sample buffers
    Buffer image_rnd_seeds = context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL, RT_FORMAT_UNSIGNED_INT2, WIDTH, HEIGHT );
    context["image_rnd_seeds"]->set( image_rnd_seeds );
    uint2* seeds = reinterpret_cast<uint2*>( image_rnd_seeds->map() );
    for ( unsigned int i = 0; i < WIDTH*HEIGHT; ++i ) {
        seeds[i] = random2u();
    }
    image_rnd_seeds->unmap();

    // RTPass ray gen program
    {
        const std::string ptx_path = ptxPath( "ppm_rtpass.cu" );
        Program ray_gen_program = context->createProgramFromPTXFile( ptx_path, "rtpass_camera" );
        context->setRayGenerationProgram( rtpass, ray_gen_program );

        // RTPass exception/miss programs
        Program exception_program = context->createProgramFromPTXFile( ptx_path, "rtpass_exception" );
        context->setExceptionProgram( rtpass, exception_program );
        context["rtpass_bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
        context->setMissProgram( rtpass, context->createProgramFromPTXFile( ptx_path, "rtpass_miss" ) );
        context["rtpass_bg_color"]->setFloat( make_float3( 0.34f, 0.55f, 0.85f ) );
    }

    // Photon pass
    const unsigned int num_photons = photon_launch_dim * photon_launch_dim * MAX_PHOTON_COUNT;
    photons_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_USER, num_photons );
    photons_buffer->setElementSize( sizeof( PhotonRecord ) );
    context["ppass_output_buffer"]->set( photons_buffer );

    {
        const std::string ptx_path = ptxPath( "ppm_ppass.cu");
        Program ray_gen_program = context->createProgramFromPTXFile( ptx_path, "ppass_camera" );
        context->setRayGenerationProgram( ppass, ray_gen_program );

        Buffer photon_rnd_seeds = context->createBuffer( RT_BUFFER_INPUT,
                RT_FORMAT_UNSIGNED_INT2,
                photon_launch_dim,
                photon_launch_dim );
        uint2* seeds = reinterpret_cast<uint2*>( photon_rnd_seeds->map() );
        for ( unsigned int i = 0; i < photon_launch_dim*photon_launch_dim; ++i ) {
            seeds[i] = random2u();
        }
        photon_rnd_seeds->unmap();
        context["photon_rnd_seeds"]->set( photon_rnd_seeds );
    }

    // Gather phase
    {
        const std::string ptx_path = ptxPath( "ppm_gather.cu" );
        Program gather_program = context->createProgramFromPTXFile( ptx_path, "gather" );
        context->setRayGenerationProgram( gather, gather_program );
        Program exception_program = context->createProgramFromPTXFile( ptx_path, "gather_exception" );
        context->setExceptionProgram( gather, exception_program );

        unsigned int photon_map_size = pow2roundup( num_photons ) - 1;
        photon_map_buffer = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_USER, photon_map_size );
        photon_map_buffer->setElementSize( sizeof( PhotonRecord ) );
        context["photon_map"]->set( photon_map_buffer );
    }

}


// Utilities for translating Mesh data to OptiX buffers.  These are copied and pasted from sutil.
namespace
{

struct MeshBuffers
{
  optix::Buffer tri_indices;
  optix::Buffer mat_indices;
  optix::Buffer positions;
  optix::Buffer normals;
  optix::Buffer texcoords;
};

void setupMeshLoaderInputs(
    optix::Context            context, 
    MeshBuffers&              buffers,
    Mesh&                     mesh
    )
{
  buffers.tri_indices = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3,   mesh.num_triangles );
  buffers.mat_indices = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT,    mesh.num_triangles );
  buffers.positions   = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, mesh.num_vertices );
  buffers.normals     = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3,
                                               mesh.has_normals ? mesh.num_vertices : 0);
  buffers.texcoords   = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2,
                                               mesh.has_texcoords ? mesh.num_vertices : 0);

  mesh.tri_indices = reinterpret_cast<int32_t*>( buffers.tri_indices->map() );
  mesh.mat_indices = reinterpret_cast<int32_t*>( buffers.mat_indices->map() );
  mesh.positions   = reinterpret_cast<float*>  ( buffers.positions->map() );
  mesh.normals     = reinterpret_cast<float*>  ( mesh.has_normals   ? buffers.normals->map()   : 0 );
  mesh.texcoords   = reinterpret_cast<float*>  ( mesh.has_texcoords ? buffers.texcoords->map() : 0 );

  mesh.mat_params = new MaterialParams[ mesh.num_materials ];
}


void unmap( MeshBuffers& buffers, Mesh& mesh )
{
  buffers.tri_indices->unmap();
  buffers.mat_indices->unmap();
  buffers.positions->unmap();
  if( mesh.has_normals )
    buffers.normals->unmap();
  if( mesh.has_texcoords)
    buffers.texcoords->unmap();

  mesh.tri_indices = 0; 
  mesh.mat_indices = 0;
  mesh.positions   = 0;
  mesh.normals     = 0;
  mesh.texcoords   = 0;

  delete [] mesh.mat_params;
  mesh.mat_params = 0;
}

} // namespace


void createGeometry( )
{
    GeometryGroup geometry_group = context->createGeometryGroup();
    std::string full_path = std::string( sutil::samplesDir() ) + "/data/wedding-band.obj";

    // We use the base Mesh class rather than OptiXMesh, so we can customize materials below
    // for different passes.
    Mesh mesh;
    MeshLoader loader( full_path );
    loader.scanMesh( mesh );

    MeshBuffers buffers;
    setupMeshLoaderInputs( context, buffers, mesh );

    loader.loadMesh( mesh );

    // Translate to OptiX geometry
    const std::string path = ptxPath( "triangle_mesh.cu" );
    optix::Program bounds_program = context->createProgramFromPTXFile( path, "mesh_bounds" );
    optix::Program intersection_program = context->createProgramFromPTXFile( path, "mesh_intersect" );

    optix::Geometry geometry = context->createGeometry();  
    geometry[ "vertex_buffer"   ]->setBuffer( buffers.positions ); 
    geometry[ "normal_buffer"   ]->setBuffer( buffers.normals); 
    geometry[ "texcoord_buffer" ]->setBuffer( buffers.texcoords ); 
    geometry[ "material_buffer" ]->setBuffer( buffers.mat_indices); 
    geometry[ "index_buffer"    ]->setBuffer( buffers.tri_indices); 
    geometry->setPrimitiveCount     ( mesh.num_triangles );
    geometry->setBoundingBoxProgram ( bounds_program );
    geometry->setIntersectionProgram( intersection_program );

    // Materials have different hit programs depending on pass.
    Program closest_hit1 = context->createProgramFromPTXFile( ptxPath( "ppm_rtpass.cu" ), "rtpass_closest_hit" );
    Program closest_hit2 = context->createProgramFromPTXFile( ptxPath( "ppm_ppass.cu" ), "ppass_closest_hit" );
    Program any_hit      = context->createProgramFromPTXFile( ptxPath( "ppm_gather.cu" ), "gather_any_hit" );

    std::vector< optix::Material > optix_materials;
    for (int i = 0; i < mesh.num_materials; ++i) {

        optix::Material material = context->createMaterial();
        material->setClosestHitProgram( 0u, closest_hit1 );
        material->setClosestHitProgram( 1u, closest_hit2 );
        material->setAnyHitProgram( 2u, any_hit );

        material["Kd"]->set3fv( mesh.mat_params[i].Kd );
        material["Ks"]->set3fv( mesh.mat_params[i].Ks );
        material[ "grid_color" ]->setFloat( 0.5f, 0.5f, 0.5f );
        material[ "use_grid" ]->setUint( mesh.mat_params[i].name == "01_-_Default" ? 1u : 0 );

        optix_materials.push_back( material );
    }

    optix::GeometryInstance geom_instance = context->createGeometryInstance(
            geometry,
            optix_materials.begin(),
            optix_materials.end()
            );

    unmap( buffers, mesh );

    geometry_group->addChild( geom_instance );
    geometry_group->setAcceleration( context->createAcceleration( "Trbvh" ) );

    context["top_object"]->set( geometry_group );
    context["top_shadower"]->set( geometry_group );
}

void createLight( PPMLight& light )
{
    light.is_area_light = 0; 
    light.position  = 1000.0f * sphericalToCartesian( LIGHT_THETA, LIGHT_PHI );
    light.direction = normalize( make_float3( 0.0f, 0.0f, 0.0f )  - light.position );
    light.radius    = 5.0f *0.01745329252f;
    light.power     = make_float3( 0.5e4f, 0.5e4f, 0.5e4f );
    context["light"]->setUserData( sizeof(PPMLight), &light );
    context["rtpass_default_radius2"]->setFloat( 0.25f);
    context["ambient_light"]->setFloat( 0.1f, 0.1f, 0.1f);
    const std::string full_path = std::string( sutil::samplesDir() ) + "/data/CedarCity.hdr";
    const float3 default_color = make_float3( 0.8f, 0.88f, 0.97f );
    context["envmap"]->setTextureSampler( sutil::loadTexture( context, full_path, default_color) );
}


//------------------------------------------------------------------------------
//
//  Photon map management
//
//------------------------------------------------------------------------------

bool photonCmpX( PhotonRecord* r1, PhotonRecord* r2 ) { return r1->position.x < r2->position.x; }
bool photonCmpY( PhotonRecord* r1, PhotonRecord* r2 ) { return r1->position.y < r2->position.y; }
bool photonCmpZ( PhotonRecord* r1, PhotonRecord* r2 ) { return r1->position.z < r2->position.z; }


void buildKDTree( PhotonRecord** photons, int start, int end, int depth, PhotonRecord* kd_tree, int current_root,
                  SplitChoice split_choice, float3 bbmin, float3 bbmax)
{
  // If we have zero photons, this is a NULL node
  if( end - start == 0 ) {
    kd_tree[current_root].axis = PPM_NULL;
    kd_tree[current_root].energy = make_float3( 0.0f );
    return;
  }

  // If we have a single photon
  if( end - start == 1 ) {
    photons[start]->axis = PPM_LEAF;
    kd_tree[current_root] = *(photons[start]);
    return;
  }

  // Choose axis to split on
  int axis;
  switch(split_choice) {
  case RoundRobin:
    {
      axis = depth%3;
    }
    break;
  case HighestVariance:
    {
      float3 mean  = make_float3( 0.0f ); 
      float3 diff2 = make_float3( 0.0f );
      for(int i = start; i < end; ++i) {
        float3 x     = photons[i]->position;
        float3 delta = x - mean;
        float3 n_inv = make_float3( 1.0f / ( static_cast<float>( i - start ) + 1.0f ) );
        mean = mean + delta * n_inv;
        diff2 += delta*( x - mean );
      }
      float3 n_inv = make_float3( 1.0f / ( static_cast<float>(end-start) - 1.0f ) );
      float3 variance = diff2 * n_inv;
      axis = max_component(variance);
    }
    break;
  case LongestDim:
    {
      float3 diag = bbmax-bbmin;
      axis = max_component(diag);
    }
    break;
  default:
    axis = -1;
    std::cerr << "Unknown SplitChoice " << split_choice << " at "<<__FILE__<<":"<<__LINE__<<"\n";
    exit(2);
    break;
  }

  int median = (start+end) / 2;
  PhotonRecord** start_addr = &(photons[start]);

  switch( axis ) {
  case 0:
    select<PhotonRecord*, 0>( start_addr, 0, end-start-1, median-start );
    photons[median]->axis = PPM_X;
    break;
  case 1:
    select<PhotonRecord*, 1>( start_addr, 0, end-start-1, median-start );
    photons[median]->axis = PPM_Y;
    break;
  case 2:
    select<PhotonRecord*, 2>( start_addr, 0, end-start-1, median-start );
    photons[median]->axis = PPM_Z;
    break;
  }

  float3 rightMin = bbmin;
  float3 leftMax  = bbmax;
  if(split_choice == LongestDim) {
    float3 midPoint = (*photons[median]).position;
    switch( axis ) {
      case 0:
        rightMin.x = midPoint.x;
        leftMax.x  = midPoint.x;
        break;
      case 1:
        rightMin.y = midPoint.y;
        leftMax.y  = midPoint.y;
        break;
      case 2:
        rightMin.z = midPoint.z;
        leftMax.z  = midPoint.z;
        break;
    }
  }

  kd_tree[current_root] = *(photons[median]);
  buildKDTree( photons, start, median, depth+1, kd_tree, 2*current_root+1, split_choice, bbmin,  leftMax );
  buildKDTree( photons, median+1, end, depth+1, kd_tree, 2*current_root+2, split_choice, rightMin, bbmax );
}

void createPhotonMap( Buffer photons_buffer, Buffer photon_map_buffer )
{
  const SplitChoice split_choice = LongestDim;

  PhotonRecord* photons_data    = reinterpret_cast<PhotonRecord*>( photons_buffer->map() );
  PhotonRecord* photon_map_data = reinterpret_cast<PhotonRecord*>( photon_map_buffer->map() );

  RTsize photon_map_size;
  photon_map_buffer->getSize( photon_map_size );
  for( unsigned int i = 0; i < (unsigned int)photon_map_size; ++i ) {
    photon_map_data[i].energy = make_float3( 0.0f );
  }

  // Push all valid photons to front of list
  RTsize num_photons;
  photons_buffer->getSize( num_photons );
  unsigned int valid_photons = 0;
  PhotonRecord** temp_photons = new PhotonRecord*[num_photons];
  for( unsigned int i = 0; i < (unsigned int)num_photons; ++i ) {
    if( fmaxf( photons_data[i].energy ) > 0.0f ) {
      temp_photons[valid_photons++] = &photons_data[i];
    }
  }
  if ( s_display_debug_buffer ) {
    std::cerr << " ** valid_photon/m_num_photons =  " 
              << valid_photons<<"/"<<num_photons
              <<" ("<<valid_photons/static_cast<float>(num_photons)<<")\n";
  }

  // Make sure we aren't at most 1 less than power of 2
  valid_photons = (valid_photons >= (unsigned int) photon_map_size) ? (unsigned int) photon_map_size : valid_photons;

  float3 bbmin = make_float3(0.0f);
  float3 bbmax = make_float3(0.0f);
  if( split_choice == LongestDim ) {
    bbmin = make_float3(  std::numeric_limits<float>::max() );
    bbmax = make_float3( -std::numeric_limits<float>::max() );
    // Compute the bounds of the photons
    for(unsigned int i = 0; i < valid_photons; ++i) {
      float3 position = (*temp_photons[i]).position;
      bbmin = fminf(bbmin, position);
      bbmax = fmaxf(bbmax, position);
    }
  }

  // Now build KD tree
  buildKDTree( temp_photons, 0, valid_photons, 0, photon_map_data, 0, split_choice, bbmin, bbmax );

  delete[] temp_photons;
  photon_map_buffer->unmap();
  photons_buffer->unmap();
}


//------------------------------------------------------------------------------
//
//  GLFW callbacks
//
//------------------------------------------------------------------------------

struct CallbackData
{
    sutil::Camera& camera;
    unsigned int& accumulation_frame;
};

void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    bool handled = false;

    if( action == GLFW_PRESS )
    {
        switch( key )
        {
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                if( context )
                    context->destroy();
                if( window )
                    glfwDestroyWindow( window );
                glfwTerminate();
                exit(EXIT_SUCCESS);

            case( GLFW_KEY_S ):
            {
                const std::string outputImage = std::string(SAMPLE_NAME) + ".png";
                std::cerr << "Saving current frame to '" << outputImage << "'\n";
                sutil::writeBufferToFile( outputImage.c_str(), getOutputBuffer() );
                handled = true;
                break;
            }
            case( GLFW_KEY_F ):
            {
               CallbackData* cb = static_cast<CallbackData*>( glfwGetWindowUserPointer( window ) );
               cb->camera.reset_lookat();
               cb->accumulation_frame = 0;
               handled = true;
               break;
            }
        }
    }

    if (!handled) {
        // forward key event to imgui
        ImGui_ImplGlfw_KeyCallback( window, key, scancode, action, mods );
    }
}

void windowSizeCallback( GLFWwindow* window, int w, int h )
{
    if (w < 0 || h < 0) return;

    const unsigned width = (unsigned)w;
    const unsigned height = (unsigned)h;

    CallbackData* cb = static_cast<CallbackData*>( glfwGetWindowUserPointer( window ) );
    if ( cb->camera.resize( width, height ) ) {
        cb->accumulation_frame = 0;
    }

    sutil::resizeBuffer( getOutputBuffer(), width, height );

    sutil::resizeBuffer( context[ "debug_buffer" ]->getBuffer(), width, height );
    sutil::resizeBuffer( context[ "rtpass_output_buffer" ]->getBuffer(), width, height );
    sutil::resizeBuffer( context[ "image_rnd_seeds" ]->getBuffer(), width, height );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glViewport(0, 0, width, height);
}


//------------------------------------------------------------------------------
//
// GLFW setup and run 
//
//------------------------------------------------------------------------------

GLFWwindow* glfwInitialize( )
{
    GLFWwindow* window = sutil::initGLFW();

    // Note: this overrides imgui key callback with our own.  We'll chain this.
    glfwSetKeyCallback( window, keyCallback );

    glfwSetWindowSize( window, (int)WIDTH, (int)HEIGHT );
    glfwSetWindowSizeCallback( window, windowSizeCallback );

    return window;
}


void launch_all( const sutil::Camera& camera, unsigned int photon_launch_dim, unsigned int accumulation_frame, 
    Buffer photons_buffer, Buffer photon_map_buffer )
{
    if ( accumulation_frame == 1 ) {

        if (s_print_timings) std::cerr << "Starting RT pass ... ";
        double t0 = sutil::currentTime();

        // Trace viewing rays
        context->launch( rtpass, camera.width(), camera.height() );

        double t1 = sutil::currentTime();
        if (s_print_timings) std::cerr << "finished. " << t1 - t0 << std::endl;

        context["total_emitted"]->setFloat(  0.0f );
    }

    // Trace photons
    {
        if (s_print_timings) std::cerr << "Starting photon pass   ... ";

        Buffer photon_rnd_seeds = context["photon_rnd_seeds"]->getBuffer();
        uint2* seeds = reinterpret_cast<uint2*>( photon_rnd_seeds->map() );
        for ( unsigned int i = 0; i < photon_launch_dim*photon_launch_dim; ++i ) {
            seeds[i] = random2u();
        }
        photon_rnd_seeds->unmap();
        double t0 = sutil::currentTime();

        context->launch( ppass, photon_launch_dim, photon_launch_dim );

        double t1 = sutil::currentTime();
        if (s_print_timings) std::cerr << "finished. " << t1 - t0 << std::endl;
    }

    // By computing the total number of photons as an unsigned long long we avoid 32 bit
    // floating point addition errors when the number of photons gets sufficiently large
    // (the error of adding two floating point numbers when the mantissa bits no longer
    // overlap).
    context["total_emitted"]->setFloat( static_cast<float>((unsigned long long)accumulation_frame*photon_launch_dim*photon_launch_dim) );

    // Build KD tree
    {
        if (s_print_timings) std::cerr << "Starting kd_tree build ... ";
        double t0 = sutil::currentTime();

        createPhotonMap( photons_buffer, photon_map_buffer );

        double t1 = sutil::currentTime();
        if (s_print_timings) std::cerr << "finished. " << t1 - t0 << std::endl;
    }


    // Shade view rays by gathering photons
    {
        if (s_print_timings) std::cerr << "Starting gather pass   ... ";
        double t0 = sutil::currentTime();

        context->launch( gather, camera.width(), camera.height() );

        double t1 = sutil::currentTime();
        if (s_print_timings) std::cerr << "finished. " << t1 - t0 << std::endl;
    }

}

void glfwRun( GLFWwindow* window, sutil::Camera& camera, PPMLight& light, unsigned int photon_launch_dim, Buffer photons_buffer, Buffer photon_map_buffer )
{
    // Initialize GL state
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, WIDTH, HEIGHT );

    unsigned int frame_count = 0;
    unsigned int accumulation_frame = 0;
    float light_phi = LIGHT_PHI;
    float light_theta = ( 0.5f*M_PIf - LIGHT_THETA );

    // Expose user data for access in GLFW callback functions when the window is resized, etc.
    // This avoids having to make it global.
    CallbackData cb = { camera, accumulation_frame };
    glfwSetWindowUserPointer( window, &cb );

    while( !glfwWindowShouldClose( window ) )
    {

        glfwPollEvents();                                                        

        ImGui_ImplGlfwGL2_NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        
        // Let imgui process the mouse first
        if (!io.WantCaptureMouse) {

            double x, y;
            glfwGetCursorPos( window, &x, &y );

            if ( camera.process_mouse( (float)x, (float)y, ImGui::IsMouseDown(0), ImGui::IsMouseDown(1), ImGui::IsMouseDown(2) ) ) {
                accumulation_frame = 0; 
            }
        }

        // imgui pushes
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,   ImVec2(0,0) );
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,          0.6f        );
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f        );

        sutil::displayFps( frame_count++ );

        {
            static const ImGuiWindowFlags window_flags = 
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar;

            ImGui::SetNextWindowPos( ImVec2( 2.0f, 40.0f ) );
            ImGui::Begin("controls", 0, window_flags );

            bool light_changed = false;
            if (ImGui::SliderAngle( "light rotation", &light_phi, 0.0f, 360.0f ) ) {
                light_changed = true;
            }
            if (ImGui::SliderAngle( "light elevation", &light_theta, 0.0f, 90.0f ) )  {
                light_changed = true;
            }
            if ( light_changed ) {
                light.position  = 1000.0f * sphericalToCartesian( 0.5f*M_PIf-light_theta, light_phi );
                light.direction = normalize( make_float3( 0.0f, 0.0f, 0.0f )  - light.position );
                context["light"]->setUserData( sizeof(PPMLight), &light );
                accumulation_frame = 0;
            }

            ImGui::End();
        }

        // imgui pops
        ImGui::PopStyleVar( 3 );

        // Render main window

        context["frame_number"]->setFloat( static_cast<float>( accumulation_frame++ ) );
        launch_all( camera, photon_launch_dim, accumulation_frame, photons_buffer, photon_map_buffer );
        sutil::displayBufferGL( getOutputBuffer() );

        const unsigned int buffer_width = camera.width();
        const unsigned int buffer_height = camera.height();

        // Debug output
        if( s_display_debug_buffer ) {
          double t0 = sutil::currentTime( );
          Buffer debug_buffer = context["debug_buffer"]->getBuffer();
          float4* debug_data = reinterpret_cast<float4*>( debug_buffer->map() );
          Buffer hit_records = context["rtpass_output_buffer"]->getBuffer();
          HitRecord* hit_record_data = reinterpret_cast<HitRecord*>( hit_records->map() );
          float4 avg  = make_float4( 0.0f );
          float4 minv = make_float4( std::numeric_limits<float>::max() );
          float4 maxv = make_float4( 0.0f );
          float counter = 0.0f;
          for( unsigned int j = 0; j < buffer_height; ++j ) {
            for( unsigned int i = 0; i < buffer_width; ++i ) {

              if( hit_record_data[j*buffer_width+i].flags & PPM_HIT ) {
                float4 val = debug_data[j*buffer_width+i];
                avg += val;
                minv = fminf(minv, val);
                maxv = fmaxf(maxv, val);
                counter += 1.0f;
              }
            }
          }
          debug_buffer->unmap();
          hit_records->unmap();

          avg = avg / counter; 
          double t1 = sutil::currentTime( );
          if ( s_print_timings ) std::cerr << "Stat collection time ...           " << t1 - t0 << std::endl;
          std::cerr << "(min, max, average):"
            << " loop iterations: ( "
            << minv.x << ", "
            << maxv.x << ", "
            << avg.x << " )"
            << " radius: ( "
            << minv.y << ", "
            << maxv.y << ", "
            << avg.y << " )"
            << " N: ( "
            << minv.z << ", "
            << maxv.z << ", "
            << avg.z << " )"
            << " M: ( "
            << minv.w << ", "
            << maxv.w << ", "
            << avg.w << " )";
          std::cerr << ", total_iterations = "<<accumulation_frame;
          std::cerr << std::endl;
        }

        // Render gui over it
        ImGui::Render();
        ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers( window );
    }
    
    destroyContext();
    glfwDestroyWindow( window );
    glfwTerminate();
}


//------------------------------------------------------------------------------
//
// Main
//
//------------------------------------------------------------------------------

void printUsageAndExit( const std::string& argv0 )
{
    std::cerr << "\nUsage: " << argv0 << " [options]\n";
    std::cerr <<
        "App Options:\n"
        "  -h   | --help                  Print this usage message and exit.\n"
        "  -f   | --file <output_file>    Save image to file and exit.\n"
        "  -n   | --nopbo                 Disable GL interop for display buffer.\n"
        "         --photon-dim <n>        Width and height of photon launch grid. Default = " << PHOTON_LAUNCH_DIM << ".\n"
        "  -ddb | --display-debug-buffer  Display debug buffer information to the shell.\n"
        "  -pt  | --print-timings         Print timing information.\n"
        "App Keystrokes:\n"
        "  q  Quit\n"
        "  s  Save image to '" << SAMPLE_NAME << ".png'\n"
        "  f  Re-center camera\n"
        "\n"
        << std::endl;

    exit(1);
}


int main( int argc, char** argv )
{
    bool use_pbo = true;
    unsigned int photon_launch_dim = PHOTON_LAUNCH_DIM;
    std::string out_file;
    for( int i=1; i<argc; ++i )
    {
        const std::string arg( argv[i] );

        if( arg == "-h" || arg == "--help" )
        {
            printUsageAndExit( argv[0] );
        }
        else if( arg == "-f" || arg == "--file"  )
        {
            if( i == argc-1 )
            {
                std::cerr << "Option '" << arg << "' requires additional argument.\n";
                printUsageAndExit( argv[0] );
            }
            out_file = argv[++i];
        }
        else if( arg == "-n" || arg == "--nopbo"  )
        {
            use_pbo = false;
        }
        else if( arg == "-ddb" || arg == "--display-debug-buffer" )
        {
            s_display_debug_buffer = true;
        }
        else if( arg == "-pt" || arg == "--print-timings" )
        {
            s_print_timings = true;
        }
        else if( arg == "--photon-dim" )
        {
            if( i == argc-1 )
            {
                std::cerr << "Option '" << arg << "' requires additional argument.\n";
                printUsageAndExit( argv[0] );
            }
            int tmp = atoi( argv[++i] );
            if (tmp > 0) photon_launch_dim = static_cast<unsigned int>(tmp);
        }
        else
        {
            std::cerr << "Unknown option '" << arg << "'\n";
            printUsageAndExit( argv[0] );
        }
        
    }

    try
    {
        GLFWwindow* window = glfwInitialize();

#ifndef __APPLE__
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            std::cerr << "GLEW init failed: " << glewGetErrorString( err ) << std::endl;
            exit(EXIT_FAILURE);
        }
#endif

        Buffer photons_buffer;
        Buffer photon_map_buffer;
        createContext( use_pbo, photon_launch_dim, photons_buffer, photon_map_buffer );

        // initial camera data
        const optix::float3 camera_eye( optix::make_float3( -188.0f, 176.0f, 0.0f ) );
        const optix::float3 camera_lookat( optix::make_float3( 0.0f, 0.0f, 0.0f ) );
        const optix::float3 camera_up( optix::make_float3( 0.0f, 1.0f, 0.0f ) );
        sutil::Camera camera( WIDTH, HEIGHT, 
                &camera_eye.x, &camera_lookat.x, &camera_up.x,
                context["rtpass_eye"], context["rtpass_U"], context["rtpass_V"], context["rtpass_W"] );

        createGeometry();
        PPMLight light;
        createLight( light );

        context->validate();
        
        if ( out_file.empty() )
        {
            glfwRun( window, camera, light, photon_launch_dim, photons_buffer, photon_map_buffer );
        }
        else
        {
            const unsigned int numframes = 16;
            std::cerr << "Accumulating " << numframes << " frames ..." << std::endl;
            for ( unsigned int frame = 0; frame < numframes; ++frame ) {
                context["frame_number"]->setFloat( static_cast<float>( frame ) );
                launch_all( camera, photon_launch_dim, frame+1, photons_buffer, photon_map_buffer );
            }
            // Note: the float4 output buffer is written in linear space without gamma correction, 
            // so it won't match the interactive display.  Apply gamma in an image viewer.
            sutil::writeBufferToFile( out_file.c_str(), getOutputBuffer() );
            std::cerr << "Wrote " << out_file << std::endl;
            destroyContext();
        }
        return 0;
    }
    SUTIL_CATCH( context->get() )
}

