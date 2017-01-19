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
//  ocean.cpp: Demonstrate cuda-optix interop via ocean demo.  Based on CUDA
//             SDK sample oceanFFT.
//
//-----------------------------------------------------------------------------

#ifndef __APPLE__
#  include <GL/glew.h>
#  if defined( _WIN32 )
#    include <GL/wglew.h>
#  endif
#endif

#include <GLFW/glfw3.h>

#include <optixu/optixpp.h>

#include <sutil.h>
#include <SunSky.h>

#include "commonStructs.h"
#include "random.h"

#include <cufft.h>
#include <cuda_runtime.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>

#include <fstream>
#include <iostream>
#include <cfloat>
#include <cstdlib>
#include <cstring>


using namespace optixu;

const char* const SAMPLE_NAME = "optixOcean";
const unsigned int WIDTH  = 1024u;
const unsigned int HEIGHT = 768u;
const unsigned int HEIGHTFIELD_WIDTH  = 1024;
const unsigned int HEIGHTFIELD_HEIGHT = 1024;
const unsigned int FFT_WIDTH          = HEIGHTFIELD_WIDTH/2 + 1;
const unsigned int FFT_HEIGHT         = HEIGHTFIELD_HEIGHT;
const float PATCH_SIZE  = 100.0f;

//------------------------------------------------------------------------------
//
// Globals
//
//------------------------------------------------------------------------------

int g_cuda_device = 0;
Context      context = 0;


//------------------------------------------------------------------------------
//
//  Helper functions
//
//------------------------------------------------------------------------------

// Obtain the CUDA device ordinal of a given OptiX device
int OptiXDeviceToCUDADevice( unsigned int optixDeviceIndex )
{
  std::vector<int> devices = context->getEnabledDevices();
  unsigned int numOptixDevices = static_cast<unsigned int>( devices.size() );
  int ordinal;
  if ( optixDeviceIndex < numOptixDevices )
  {
    context->getDeviceAttribute( devices[optixDeviceIndex], RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(ordinal), &ordinal );
    return ordinal;
  }
  return -1;
}


static void errorCallback(int error, const char* description)                   {                                                                                
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;           
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


void createContext( bool use_pbo, Buffer& data_buffer )
{
    // Set up context
    context = Context::create();
    
    context->setRayTypeCount( 1 );
    context->setEntryPointCount( 4 );
    context->setStackSize(2000);

    context["scene_epsilon"       ]->setFloat( 1.e-3f );
    context["max_depth"           ]->setInt( 6 );

        
    // Exception program
    std::string ptx_path = ptxPath( "accum_camera.cu" );
    Program exception_program = context->createProgramFromPTXFile( ptx_path, "exception" );
    context->setExceptionProgram( 0, exception_program );
    context["bad_color"]->setFloat( 1.0f, 0.0f, 1.0f );

    // Ray gen program for raytracing camera
    Program ray_gen_program = context->createProgramFromPTXFile( ptx_path, "pinhole_camera" );
    context->setRayGenerationProgram( 0, ray_gen_program );
    Buffer output_buffer = sutil::createOutputBuffer( context, RT_FORMAT_UNSIGNED_BYTE4, WIDTH, HEIGHT, use_pbo );
    context["output_buffer"]->set( output_buffer ); 
    Buffer accum_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, WIDTH, HEIGHT );
    context["accum_buffer"]->set( accum_buffer ); 
    context["pre_image"]->set( accum_buffer ); 
    context["frame"]->setUint( 1u ); 

    // Preetham sky model
    ptx_path = ptxPath( "ocean_render.cu" );
    context->setMissProgram( 0, context->createProgramFromPTXFile( ptx_path, "miss" ) );
    context["cutoff_color" ]->setFloat( 0.07f, 0.18f, 0.3f );

    // Ray gen program for heightfield update
    ptx_path = ptxPath( "ocean_sim.cu" );
    Program data_gen_program = context->createProgramFromPTXFile( ptx_path, "generate_spectrum" );
    context->setRayGenerationProgram( 1, data_gen_program );
    context["patch_size"]->setFloat( PATCH_SIZE );
    context["t"]->setFloat( 0.0f );
    Buffer h0_buffer = context->createBuffer( RT_BUFFER_INPUT,  RT_FORMAT_FLOAT2, FFT_WIDTH, FFT_HEIGHT );
    Buffer ht_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT2, FFT_WIDTH, FFT_HEIGHT ); 
    Buffer ik_ht_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT2, FFT_WIDTH, FFT_HEIGHT ); 
    context["h0"]->set( h0_buffer ); 
    context["ht"]->set( ht_buffer ); 
    context["ik_ht"]->set( ik_ht_buffer ); 
    
    //Ray gen program for normal calculation
    Program normal_program = context->createProgramFromPTXFile( ptx_path, "calculate_normals" );
    context->setRayGenerationProgram( 2, normal_program );
    context["height_scale"]->setFloat( 0.5f );
    // Could pack data and normals together, but that would preclude using fft_output directly as data_buffer.
    data_buffer   = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT,
                                             HEIGHTFIELD_WIDTH,
                                             HEIGHTFIELD_HEIGHT );
    Buffer normal_buffer = context->createBuffer( RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4,
                                             HEIGHTFIELD_WIDTH,
                                             HEIGHTFIELD_HEIGHT );

    // TODO: generic name should be more specific
    context["data"]->set(data_buffer);
    context["normals"]->set(normal_buffer );

    // Ray gen program for tonemap
    ptx_path = ptxPath( "tonemap.cu" );
    Program tonemap_program = context->createProgramFromPTXFile( ptx_path, "tonemap" );
    context->setRayGenerationProgram( 3, tonemap_program );
    context["f_exposure"]->setFloat( 0.0f );
    
}

void createGeometry( Buffer data_buffer )
{
  Geometry heightfield = context->createGeometry();
  heightfield->setPrimitiveCount( 1u );

  const std::string ptx_path = ptxPath( "ocean_render.cu" );
  heightfield->setBoundingBoxProgram(  context->createProgramFromPTXFile( ptx_path, "bounds" ) );
  heightfield->setIntersectionProgram( context->createProgramFromPTXFile( ptx_path, "intersect" ) );
  float3 min = make_float3( -2.0f, -0.2f, -2.0f );
  float3 max = make_float3(  2.0f,  0.2f,  2.0f );
  const RTsize nx = HEIGHTFIELD_WIDTH;
  const RTsize nz = HEIGHTFIELD_HEIGHT;
  
  // If buffer is nx by nz, we have nx-1 by nz-1 cells;
  float3 cellsize = (max - min) / (make_float3(static_cast<float>(nx-1), 1.0f, static_cast<float>(nz-1)));
  cellsize.y = 1;
  float3 inv_cellsize = make_float3(1)/cellsize;
  heightfield["boxmin"]->setFloat(min);
  heightfield["boxmax"]->setFloat(max);
  heightfield["cellsize"]->setFloat(cellsize);
  heightfield["inv_cellsize"]->setFloat(inv_cellsize);

  // Create material
  Material heightfield_matl = context->createMaterial();
  Program water_ch = context->createProgramFromPTXFile( ptx_path, "closest_hit_radiance" );

  heightfield_matl["importance_cutoff"  ]->setFloat( 0.01f );
  heightfield_matl["fresnel_exponent"   ]->setFloat( 4.0f );
  heightfield_matl["fresnel_minimum"    ]->setFloat( 0.05f );
  heightfield_matl["fresnel_maximum"    ]->setFloat( 0.30f );
  heightfield_matl["refraction_index"   ]->setFloat( 1.4f );
  heightfield_matl["refraction_color"   ]->setFloat( 0.95f, 0.95f, 0.95f );
  heightfield_matl["reflection_color"   ]->setFloat( 0.7f, 0.7f, 0.7f );
  heightfield_matl["refraction_maxdepth"]->setInt( 6 );
  heightfield_matl["reflection_maxdepth"]->setInt( 6 );
  const float3 extinction = make_float3(.75f, .89f, .80f);
  heightfield_matl["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
  heightfield_matl["shadow_attenuation"]->setFloat( 1.0f, 1.0f, 1.0f );
  heightfield_matl->setClosestHitProgram( 0, water_ch);

  heightfield_matl["Ka"]->setFloat(0.0f, 0.3f, 0.1f);
  heightfield_matl["Kd"]->setFloat(0.3f, 0.7f, 0.5f);
  heightfield_matl["Ks"]->setFloat(0.1f, 0.1f, 0.1f);
  heightfield_matl["phong_exp"]->setFloat(1600);
  heightfield_matl["reflectivity"]->setFloat(0.1f, 0.1f, 0.1f);

  GeometryInstance gi = context->createGeometryInstance( heightfield, &heightfield_matl, &heightfield_matl+1 );
  
  GeometryGroup geometrygroup = context->createGeometryGroup();
  geometrygroup->setChildCount( 1 );
  geometrygroup->setChild( 0, gi );

  geometrygroup->setAcceleration( context->createAcceleration("NoAccel","NoAccel") );
  
  context["top_object"]->set( geometrygroup );
  context["top_shadower"]->set( geometrygroup );
}


void createLights()
{
    // Set up light buffer
    context["ambient_light_color"]->setFloat( 1.0f, 1.0f, 1.0f );
    BasicLight lights[] = { 
        { { 4.0f, 12.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, 1 }
    };

    Buffer light_buffer = context->createBuffer(RT_BUFFER_INPUT);
    light_buffer->setFormat(RT_FORMAT_USER);
    light_buffer->setElementSize(sizeof(BasicLight));
    light_buffer->setSize( sizeof(lights)/sizeof(lights[0]) );
    memcpy(light_buffer->map(), lights, sizeof(lights));
    light_buffer->unmap();

    context["lights"]->set(light_buffer);

    //
    // Sun and sky model
    //
    sutil::PreethamSunSky sun_sky;
    sun_sky.setSunTheta( 1.2f );
    sun_sky.setSunPhi( 0.0f );
    sun_sky.setTurbidity( 2.2f );
    sun_sky.setVariables( context );

}

// Phillips spectrum
// Vdir - wind angle in radians
// V - wind speed
float phillips(float Kx, float Ky, float Vdir, float V, float A)
{
  const float g = 9.81f;            // gravitational constant

  float k_squared = Kx * Kx + Ky * Ky;
  float k_x = Kx / sqrtf(k_squared);
  float k_y = Ky / sqrtf(k_squared);
  float L = V * V / g;
  float w_dot_k = k_x * cosf(Vdir) + k_y * sinf(Vdir);

  if (k_squared == 0.0f ) return 0.0f;
  return A * expf( -1.0f / (k_squared * L * L) ) / (k_squared * k_squared) * w_dot_k * w_dot_k;
}

// Generate base heightfield in frequency space
// This could be a CUDA kernel.
void generateH0( float2* h_h0 )
{
  for (unsigned int y = 0u; y < FFT_HEIGHT; y++) {
    for (unsigned int x = 0u; x < FFT_WIDTH; x++) {
      float kx = M_PIf * x / PATCH_SIZE;
      float ky = 2.0f * M_PIf * y / PATCH_SIZE;

      // note - these random numbers should be from a Gaussian distribution really
      float Er = 2.0f * rand() / static_cast<float>( RAND_MAX ) - 1.0f;
      float Ei = 2.0f * rand() / static_cast<float>( RAND_MAX ) - 1.0f;

      // These can be made user-adjustable
      const float wave_scale = .00000000775f;
      const float wind_speed = 10.0f;     
      const float wind_dir   = M_PIf/3.0f;   

      float P = sqrtf( phillips( kx, ky, wind_dir, wind_speed, wave_scale ) );


      float h0_re = 1.0f / sqrtf(2.0f) * Er * P;
      float h0_im = 1.0f / sqrtf(2.0f) * Ei * P;

      int i = y*FFT_WIDTH+x;
      h_h0[i].x = h0_re;
      h_h0[i].y = h0_im;
      
      if(x == 0) {
        h_h0[i].x = h_h0[i].y = 0.0f;
      }
      
    }
  }
}


int main( int argc, char** argv )
{

    const bool use_pbo = false; //stub
    Buffer data_buffer = 0;
    createContext( use_pbo, data_buffer );
    createGeometry( data_buffer );
    createLights();

    g_cuda_device = OptiXDeviceToCUDADevice( 0 );

    if ( g_cuda_device < 0 ) {
      std::cerr << "OptiX device 0 must be a valid CUDA device number.\n";
      exit(1);
    }

    //
    // Setup initial heights in OptiX buffer
    //

    Buffer h0_buffer = context["h0"]->getBuffer();
    float2* height0 = static_cast<float2*>( h0_buffer->map() );
    generateH0( height0 );
    h0_buffer->unmap();

    // Finalize
    context->validate();
}

