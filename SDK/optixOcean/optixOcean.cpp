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
#include <Camera.h>
#include <SunSky.h>

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

Context      context = 0;



//------------------------------------------------------------------------------
//
// Helpers for checking cuda and cufft return codes
//
//------------------------------------------------------------------------------

#define cutilSafeCall(err) __cudaSafeCall(err, __FILE__, __LINE__)

inline void __cudaSafeCall( cudaError err, const char *file, const int line )
{
  if( cudaSuccess != err) {
    fprintf(stderr, "cudaSafeCall() Runtime error: file <%s>, line %i : %s.\n",
            file, line, cudaGetErrorString( err) );
    exit(-1);
  }
}

#define cufftSafeCall(err) __cufftSafeCall(err, __FILE__, __LINE__)

inline void __cufftSafeCall( cufftResult err, const char *file, const int line )
{
  if( CUFFT_SUCCESS != err) {
    std::string mssg = err == CUFFT_INVALID_PLAN     ? "invalid plan"    :
                       err == CUFFT_ALLOC_FAILED     ? "alloc failed"    :
                       err == CUFFT_INVALID_TYPE     ? "invalid type"    :
                       err == CUFFT_INVALID_VALUE    ? "invalid value"   :
                       err == CUFFT_INTERNAL_ERROR   ? "internal error"  :
                       err == CUFFT_EXEC_FAILED      ? "exec failed"     :
                       err == CUFFT_SETUP_FAILED     ? "setup failed"    :
                       err == CUFFT_INVALID_SIZE     ? "invalid size"    :
                       "bad error code!!!!";

    fprintf(stderr, "cufftSafeCall() CUFFT error '%s' in file <%s>, line %i.\n",
            mssg.c_str(), file, line);
    exit(-1);
  }
}


//------------------------------------------------------------------------------
//
//  Helper functions
//
//------------------------------------------------------------------------------


// Limit work to a single device for simplicity, so we can run all CUDA
// kernels on a single device when updating the height field.
int initSingleDevice()
{
    std::vector<int> devices = context->getEnabledDevices();
    // Limit to single device
    if ( devices.size() > 1 ) {
        context->setDevices( devices.begin(), devices.begin()+1 );
        char name[256];
        context->getDeviceAttribute( devices[0], RT_DEVICE_ATTRIBUTE_NAME, sizeof( name ), name );
        std::cerr << "Limiting to device: " << name << std::endl;
    }
    int ordinal = -1;
    context->getDeviceAttribute( devices[0], RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(ordinal), &ordinal );
   
    cudaSetDevice( ordinal );

    return devices[0]; // Return OptiX device ordinal, NOT the CUDA device ordinal
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
    if( context ) {
        context->destroy();
        context = 0;
    }
}


// State for animating buffers
struct RenderBuffers
{
    Buffer ht;       // Frequency domain heights
    Buffer heights;
    Buffer normals;
    int optix_device_ordinal;
};


void createContext( bool use_pbo, RenderBuffers& buffers )
{
    // Set up context
    context = Context::create();
    
    context->setRayTypeCount( 1 );
    context->setEntryPointCount( 4 );
    context->setStackSize(2000);

    context["scene_epsilon"       ]->setFloat( 1.e-3f );
    context["max_depth"           ]->setInt( 1 );
        
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
    context["frame"]->setUint( 0u ); 

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
    buffers.ht = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT2, FFT_WIDTH, FFT_HEIGHT ); 
    Buffer ik_ht_buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT2, FFT_WIDTH, FFT_HEIGHT ); 
    context["h0"]->set( h0_buffer ); 
    context["ht"]->set( buffers.ht ); 
    context["ik_ht"]->set( ik_ht_buffer ); 
    
    //Ray gen program for normal calculation
    Program normal_program = context->createProgramFromPTXFile( ptx_path, "calculate_normals" );
    context->setRayGenerationProgram( 2, normal_program );
    context["height_scale"]->setFloat( 0.5f );
    // Could pack heights and normals together, but that would preclude using fft_output directly as height_buffer.
    buffers.heights   = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT,
                                             HEIGHTFIELD_WIDTH,
                                             HEIGHTFIELD_HEIGHT );
    buffers.normals = context->createBuffer( RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4,
                                             HEIGHTFIELD_WIDTH,
                                             HEIGHTFIELD_HEIGHT );

    context["heights"]->set(buffers.heights);
    context["normals"]->set(buffers.normals );

    // Ray gen program for tonemap
    ptx_path = ptxPath( "tonemap.cu" );
    Program tonemap_program = context->createProgramFromPTXFile( ptx_path, "tonemap" );
    context->setRayGenerationProgram( 3, tonemap_program );
    context["f_exposure"]->setFloat( 0.0f );
    
}

void createGeometry()
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

  heightfield_matl["fresnel_exponent"   ]->setFloat( 4.0f );
  heightfield_matl["fresnel_minimum"    ]->setFloat( 0.05f );
  heightfield_matl["fresnel_maximum"    ]->setFloat( 0.30f );
  heightfield_matl["refraction_index"   ]->setFloat( 1.4f );
  heightfield_matl["refraction_color"   ]->setFloat( 0.95f, 0.95f, 0.95f );
  heightfield_matl["reflection_color"   ]->setFloat( 0.7f, 0.7f, 0.7f );
  const float3 extinction = make_float3(.75f, .89f, .80f);
  heightfield_matl["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
  heightfield_matl->setClosestHitProgram( 0, water_ch);


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

// Generate initial heightfield in frequency space
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


void updateHeightfield( float anim_time, RenderBuffers buffers )
{

    static const float ANIM_SCALE = 0.25f;
    context["t"]->setFloat( static_cast<float>(anim_time) * (-0.5f) * ANIM_SCALE );

    // Generate_spectrum
    context->launch( 1, FFT_WIDTH, FFT_HEIGHT );

    // Transform results directly into OptiX buffer using CUFFT

    cufftComplex* ht_buffer_device_ptr = static_cast<cufftComplex*>( buffers.ht->getDevicePointer( buffers.optix_device_ordinal ) );
    cufftReal* height_buffer_device_ptr = static_cast<cufftReal*>( buffers.heights->getDevicePointer( buffers.optix_device_ordinal ) );

    cufftHandle fft_plan;
    cufftSafeCall( cufftPlan2d( &fft_plan, HEIGHTFIELD_WIDTH, HEIGHTFIELD_HEIGHT, CUFFT_C2R) );
    cufftSafeCall( cufftExecC2R( fft_plan, ht_buffer_device_ptr, height_buffer_device_ptr ) );

    cufftSafeCall( cufftDestroy( fft_plan ) );

    // Calculate normals for new heights
    context->launch( 2, HEIGHTFIELD_WIDTH, HEIGHTFIELD_HEIGHT );
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
    sutil::resizeBuffer( context[ "accum_buffer" ]->getBuffer(), width, height );

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


void glfwRun( GLFWwindow* window, sutil::Camera& camera, RenderBuffers& buffers )
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
    bool do_animate = true;

    double previous_time = sutil::currentTime();
    double anim_time = 0.0f;

    // Expose user data for access in GLFW callback functions when the window is resized, etc.
    // This avoids having to make it global.
    CallbackData cb = { camera, accumulation_frame };
    glfwSetWindowUserPointer( window, &cb );

    while( !glfwWindowShouldClose( window ) )
    {

        glfwPollEvents();                                                        

        ImGui_ImplGlfw_NewFrame();

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

            if ( ImGui::Checkbox( "animate", &do_animate ) ) {
                previous_time = sutil::currentTime();
            }

            ImGui::End();
        }

        // imgui pops
        ImGui::PopStyleVar( 3 );

        if ( do_animate ) {
            // update animation time
            const double current_time = sutil::currentTime();
            anim_time += previous_time - current_time;
            previous_time = current_time;

            updateHeightfield( static_cast<float>( anim_time ), buffers );
            accumulation_frame = 0;
        }

        // Render main window
        context["frame"]->setUint( accumulation_frame++ );
        context->launch( 0, camera.width(), camera.height() );

        // Tonemap
        context->launch( 3, camera.width(), camera.height() );
        sutil::displayBufferGL( getOutputBuffer() );

        // Render gui over it
        ImGui::Render();

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
        "  -h | --help                  Print this usage message and exit.\n"
        "  -f | --file <output_file>    Save image to file and exit.\n"
        "  -n | --nopbo                 Disable GL interop for display buffer.\n"
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
    bool use_pbo  = true;
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
        else {
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

        RenderBuffers render_buffers;
        createContext( use_pbo, render_buffers );

        render_buffers.optix_device_ordinal = initSingleDevice();

        createGeometry();
        createLights();

        //
        // Initialize frequency-domain heights in OptiX buffer
        //

        Buffer h0_buffer = context["h0"]->getBuffer();
        float2* height0 = static_cast<float2*>( h0_buffer->map() );
        generateH0( height0 );
        h0_buffer->unmap();

        const float3 camera_eye( make_float3( 1.47502f, 0.284192f, 0.8623f ) );
        const float3 camera_lookat( make_float3( 0.0f, 0.0f, 0.0f ) );
        const float3 camera_up( make_float3( 0.0f, 1.0f, 0.0f ) );
        sutil::Camera camera( WIDTH, HEIGHT, 
                &camera_eye.x, &camera_lookat.x, &camera_up.x,
                context["eye"], context["U"], context["V"], context["W"] );

        // Finalize
        context->validate();

        if ( out_file.empty() )
        {
            glfwRun( window, camera, render_buffers );
        }
        else
        {
            // Accumulate frames for anti-aliasing
            updateHeightfield( 0.0f, render_buffers );
            const unsigned int numframes = 64;
            std::cerr << "Accumulating " << numframes << " frames ..." << std::endl;
            for ( unsigned int frame = 0; frame < numframes; ++frame ) {
                context["frame"]->setUint( frame );
                context->launch( 0, WIDTH, HEIGHT );
            }

            // tonemap
            context->launch( 3, WIDTH, HEIGHT );

            sutil::writeBufferToFile( out_file.c_str(), getOutputBuffer() );
            std::cerr << "Wrote " << out_file << std::endl;
            destroyContext();
        }

    }
    SUTIL_CATCH( context->get() )

}

