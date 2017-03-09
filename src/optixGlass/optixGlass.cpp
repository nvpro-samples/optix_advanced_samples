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
// optixGlass: a physically-based glass shader example, using path tracing.
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

#include <sutil.h>
#include "commonStructs.h"
#include <Camera.h>
#include <OptiXMesh.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdint.h>

using namespace optix;

const char* const SAMPLE_NAME = "optixGlass";
const unsigned int WIDTH  = 768u;
const unsigned int HEIGHT = 576u;
const float3 DEFAULT_TRANSMITTANCE = make_float3( 0.1f, 0.63f, 0.3f );

//------------------------------------------------------------------------------
//
// Globals
//
//------------------------------------------------------------------------------

Context      context = 0;


//------------------------------------------------------------------------------
//
//  Helper functions
//
//------------------------------------------------------------------------------
    

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


void createContext( bool use_pbo )
{
    // Set up context
    context = Context::create();
    context->setRayTypeCount( 1 );
    context->setEntryPointCount( 1 );

    // Note: this sample does not need a big stack size even with high ray depths, 
    // because rays are not shot recursively.
    context->setStackSize( 800 );

    // Note: high max depth for reflection and refraction through glass
    context["max_depth"]->setInt( 10 );
    context["cutoff_color"]->setFloat( 0.2f, 0.2f, 0.2f );
    context["frame"]->setUint( 0u );
    context["scene_epsilon"]->setFloat( 1.e-3f );

    Buffer buffer = sutil::createOutputBuffer( context, RT_FORMAT_UNSIGNED_BYTE4, WIDTH, HEIGHT, use_pbo );
    context["output_buffer"]->set( buffer );

    // Accumulation buffer
    Buffer accum_buffer = context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL,
            RT_FORMAT_FLOAT4, WIDTH, HEIGHT );
    context["accum_buffer"]->set( accum_buffer );

    // Ray generation program
    std::string ptx_path( ptxPath( "path_trace_camera.cu" ) );
    Program ray_gen_program = context->createProgramFromPTXFile( ptx_path, "pinhole_camera" );
    context->setRayGenerationProgram( 0, ray_gen_program );

    // Exception program
    Program exception_program = context->createProgramFromPTXFile( ptx_path, "exception" );
    context->setExceptionProgram( 0, exception_program );
    context["bad_color"]->setFloat( 1.0f, 0.0f, 1.0f );

    // Miss program
    ptx_path = ptxPath( "gradientbg.cu" );
    context->setMissProgram( 0, context->createProgramFromPTXFile( ptx_path, "miss" ) );
    context["background_light"]->setFloat( 1.0f, 1.0f, 1.0f );
    context["background_dark"]->setFloat( 0.3f, 0.3f, 0.3f );

    // align background's up direction with camera's look direction
    float3 bg_up = normalize( make_float3(0.0f, -1.0f, -1.0f) );

    // tilt the background's up direction in the direction of the camera's up direction
    bg_up.y += 1.0f;
    bg_up = normalize(bg_up);
    context["up"]->setFloat( bg_up.x, bg_up.y, bg_up.z );
}


Material createGlassMaterial( )
{
    const std::string ptx_path = ptxPath( "glass.cu" );
    Program ch_program = context->createProgramFromPTXFile( ptx_path, "closest_hit_radiance" );

    Material material = context->createMaterial();
    material->setClosestHitProgram( 0, ch_program );

    material["fresnel_exponent"   ]->setFloat( 4.0f );
    material["fresnel_minimum"    ]->setFloat( 0.1f );
    material["fresnel_maximum"    ]->setFloat( 1.0f );
    material["refraction_index"   ]->setFloat( 1.4f );
    material["refraction_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
    material["reflection_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );

    // Set this on the global context so it's easy to change in the gui
    const float3 transmittance = DEFAULT_TRANSMITTANCE;
    context["transmittance_constant"]->setFloat( transmittance.x, transmittance.y, transmittance.z );

    return material;
}

Material createDiffuseMaterial()
{
    const std::string ptx_path = ptxPath( "diffuse.cu" );
    Program ch_program = context->createProgramFromPTXFile( ptx_path, "closest_hit_radiance" );

    Material material = context->createMaterial();
    material->setClosestHitProgram( 0, ch_program );

    const std::string texture_filename = std::string( sutil::samplesDir() ) + "/data/grid.ppm";
    material["Kd_map"]->setTextureSampler( sutil::loadTexture( context, texture_filename, optix::make_float3( 1.0f ) ) );
    material["Kd_map_scale"]->setFloat( make_float2( 0.05f, 0.05f) );

    return material;
}


optix::Aabb createGeometry(
        const std::vector<std::string>& filenames,
        const std::vector<optix::Matrix4x4>& xforms, 
        const Material glass_material,
        const Material ground_material,
        // output: this is a Group with two GeometryGroup children, for toggling visibility later
        optix::Group& top_group
        )
{

    const std::string ptx_path = ptxPath( "triangle_mesh.cu" );

    top_group = context->createGroup();
    top_group->setAcceleration( context->createAcceleration( "Trbvh" ) );

    int num_triangles = 0;
    optix::Aabb aabb;
    {
        GeometryGroup geometry_group = context->createGeometryGroup();
        geometry_group->setAcceleration( context->createAcceleration( "Trbvh" ) );
        top_group->addChild( geometry_group );
        for (size_t i = 0; i < filenames.size(); ++i) {

            OptiXMesh mesh;
            mesh.context = context;
            
            // override defaults
            mesh.intersection = context->createProgramFromPTXFile( ptx_path, "mesh_intersect_refine" );
            mesh.bounds = context->createProgramFromPTXFile( ptx_path, "mesh_bounds" );
            mesh.material = glass_material;

            loadMesh( filenames[i], mesh, xforms[i] ); 
            geometry_group->addChild( mesh.geom_instance );

            aabb.include( mesh.bbox_min, mesh.bbox_max );

            std::cerr << filenames[i] << ": " << mesh.num_triangles << std::endl;
            num_triangles += mesh.num_triangles;
        }
        std::cerr << "Total triangle count: " << num_triangles << std::endl;
    }

    {
        // Ground plane
        GeometryGroup geometry_group = context->createGeometryGroup();
        geometry_group->setAcceleration( context->createAcceleration( "NoAccel" ) );
        top_group->addChild( geometry_group );
        const std::string floor_ptx = ptxPath( "parallelogram_iterative.cu" );
        GeometryInstance instance = sutil::createOptiXGroundPlane( context, floor_ptx, aabb, ground_material, 3.0f );
        geometry_group->addChild( instance );
    }

    context[ "top_object" ]->set( top_group ); 

    return aabb;
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


void glfwRun( GLFWwindow* window, sutil::Camera& camera, const optix::Group top_group )
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
    float3 glass_transmittance = DEFAULT_TRANSMITTANCE;
    int max_depth = 10;
    bool draw_ground = true;

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
            if (ImGui::SliderFloat3( "transmittance", (float*)(&glass_transmittance.x), 0.01f, 1.0f )) {
                context["transmittance_constant"]->setFloat( glass_transmittance.x, glass_transmittance.y, glass_transmittance.z );
                accumulation_frame = 0;
            }
            if (ImGui::SliderInt( "max depth", &max_depth, 1, 10 )) {
                context["max_depth"]->setInt( max_depth );
                accumulation_frame = 0;
            }
            if (ImGui::Checkbox( "draw ground plane", &draw_ground ) ) {
                if ( draw_ground ) {
                    context["top_object"]->set( top_group );
                } else {
                    // assume group has two children: mesh and ground
                    GeometryGroup geomgroup = top_group->getChild<GeometryGroup>( 0 );
                    context["top_object"]->set( geomgroup );
                }
                accumulation_frame = 0;
            }
            ImGui::End();
        }

        // imgui pops
        ImGui::PopStyleVar( 3 );

        // Render main window
        context["frame"]->setUint( accumulation_frame++ );
        context->launch( 0, camera.width(), camera.height() );
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
    std::cerr << "\nUsage: " << argv0 << " [options] [mesh0 mesh1 ...]\n";
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
        "Mesh files are optional and can be OBJ or PLY.\n"
        << std::endl;

    exit(1);
}


int main( int argc, char** argv )
{
    bool use_pbo  = true;
    std::string out_file;
    std::vector<std::string> mesh_files;
    std::vector<optix::Matrix4x4> mesh_xforms;
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
        else if( arg[0] == '-' )
        {
            std::cerr << "Unknown option '" << arg << "'\n";
            printUsageAndExit( argv[0] );
        }
        else {
            // Interpret argument as a mesh file.
            mesh_files.push_back( argv[i] );
            mesh_xforms.push_back( optix::Matrix4x4::identity() );
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

        createContext( use_pbo );

        if ( mesh_files.empty() ) {

            // Default scene

            const optix::Matrix4x4 xform = optix::Matrix4x4::rotate( -M_PIf/2.0f, make_float3( 0.0f, 1.0f, 0.0f) );
            mesh_files.push_back( std::string( sutil::samplesDir() ) + "/data/teapot_lid.ply" );
            mesh_xforms.push_back( xform );
            mesh_files.push_back( std::string( sutil::samplesDir() ) + "/data/teapot_body.ply" );
            mesh_xforms.push_back( xform );
        }

        Material glass_material = createGlassMaterial();
        Material ground_material = createDiffuseMaterial();
        optix::Group top_group;
        const optix::Aabb aabb = createGeometry( mesh_files, mesh_xforms, glass_material, ground_material, top_group );

        // Note: lighting comes from miss program

        context->validate();

        const optix::float3 camera_eye( optix::make_float3( 0.0f, 1.5f*aabb.extent( 1 ), 1.5f*aabb.extent( 2 ) ) );
        const optix::float3 camera_lookat( aabb.center() );
        const optix::float3 camera_up( optix::make_float3( 0.0f, 1.0f, 0.0f ) );
        sutil::Camera camera( WIDTH, HEIGHT, 
                &camera_eye.x, &camera_lookat.x, &camera_up.x,
                context["eye"], context["U"], context["V"], context["W"] );

        if ( out_file.empty() )
        {
            glfwRun( window, camera, top_group );
        }
        else
        {
            // Accumulate frames for anti-aliasing
            const unsigned int numframes = 256;
            std::cerr << "Accumulating " << numframes << " frames ..." << std::endl;
            for ( unsigned int frame = 0; frame < numframes; ++frame ) {
                context["frame"]->setUint( frame );
                context->launch( 0, WIDTH, HEIGHT );
            }
            sutil::writeBufferToFile( out_file.c_str(), getOutputBuffer() );
            std::cerr << "Wrote " << out_file << std::endl;
            destroyContext();
        }
        return 0;
    }
    SUTIL_CATCH( context->get() )
}

