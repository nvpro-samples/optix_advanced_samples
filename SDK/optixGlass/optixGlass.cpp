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
// optixGlass: a glass shader example
//
//-----------------------------------------------------------------------------

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glew.h>
#  if defined( _WIN32 )
#  include <GL/wglew.h>
#  include <GL/freeglut.h>
#  else
#  include <GL/glut.h>
#  endif
#endif

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>

#include <sutil.h>
#include "commonStructs.h"
#include <Arcball.h>
#include <OptiXMesh.h>

#include <cstring>
#include <iostream>
#include <stdint.h>

using namespace optix;

const char* const SAMPLE_NAME = "optixGlass";

//------------------------------------------------------------------------------
//
// Globals
//
//------------------------------------------------------------------------------

Context      context;
uint32_t     width  = 768u;
uint32_t     height = 576u;
bool         use_pbo = true;

// Camera state
float3       camera_up;
float3       camera_lookat;
float3       camera_eye;
Matrix4x4    camera_rotate;
bool         camera_dirty = true;  // Do camera params need to be copied to OptiX context
sutil::Arcball arcball;

// Mouse state
int2       mouse_prev_pos;
int        mouse_button;



//------------------------------------------------------------------------------
//
//  Helper functions
//
//------------------------------------------------------------------------------

std::string ptxPath( const std::string& cuda_file )
{
    return
        std::string(sutil::samplesPTXDir()) +
        "/" + std::string(SAMPLE_NAME) + "_generated_" +
        cuda_file +
        ".ptx";
}


Buffer getOutputBuffer()
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


void registerExitHandler()
{
    // register shutdown handler
#ifdef _WIN32
    glutCloseFunc( destroyContext );  // this function is freeglut-only
#else
    atexit( destroyContext );
#endif
}


void createContext()
{
    // Set up context
    context = Context::create();
    context->setRayTypeCount( 2 );
    context->setEntryPointCount( 1 );
    context->setStackSize( 2800 );

    // Note: high max depth for reflection and refraction through glass
    context["max_depth"]->setInt( 10 );
    context["radiance_ray_type"]->setUint( 0 );
    context["shadow_ray_type"]->setUint( 1 );
    context["frame"]->setUint( 0u );
    context["scene_epsilon"]->setFloat( 1.e-3f );

    Buffer buffer = sutil::createOutputBuffer( context, RT_FORMAT_UNSIGNED_BYTE4, width, height, use_pbo );
    context["output_buffer"]->set( buffer );

    // Accumulation buffer
    Buffer accum_buffer = context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL,
            RT_FORMAT_FLOAT4, width, height );
    context["accum_buffer"]->set( accum_buffer );

    // Ray generation program
    std::string ptx_path( ptxPath( "accum_camera.cu" ) );
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
    float3 bg_up = normalize( make_float3(-14.0f, -14.0f, -7.0f) );

    // tilt the background's up direction in the direction of the camera's up direction
    bg_up.y += 1.0f;
    bg_up = normalize(bg_up);
    context["up"]->setFloat( bg_up.x, bg_up.y, bg_up.z );

}

Material createMaterial( const float3& extinction )
{
    const std::string ptx_path = ptxPath( "glass.cu" );
    Program ch_program = context->createProgramFromPTXFile( ptx_path, "closest_hit_radiance" );
    Program ah_program = context->createProgramFromPTXFile( ptx_path, "any_hit_shadow" );

    Material material = context->createMaterial();
    material->setClosestHitProgram( 0, ch_program );
    material->setAnyHitProgram( 1, ah_program );

    material["importance_cutoff"  ]->setFloat( 0.01f );
    material["cutoff_color"       ]->setFloat( 0.2f, 0.2f, 0.2f );
    material["fresnel_exponent"   ]->setFloat( 4.0f );
    material["fresnel_minimum"    ]->setFloat( 0.1f );
    material["fresnel_maximum"    ]->setFloat( 1.0f );
    material["refraction_index"   ]->setFloat( 1.4f );
    material["refraction_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
    material["reflection_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
    material["refraction_maxdepth"]->setInt( 10 );
    material["reflection_maxdepth"]->setInt( 5 );
    material["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
    material["shadow_attenuation"]->setFloat( 1.0f, 1.0f, 1.0f );

    return material;
}


void createGeometry( const std::vector<std::string>& filenames, const std::vector<optix::Matrix4x4>& xforms, Material material )
{

    const std::string ptx_path = ptxPath( "triangle_mesh_iterative.cu" );

    GeometryGroup geometry_group = context->createGeometryGroup();
    for (size_t i = 0; i < filenames.size(); ++i) {

        OptiXMesh mesh;
        mesh.context = context;
        
        // override defaults
        mesh.intersection = context->createProgramFromPTXFile( ptx_path, "mesh_intersect" );
        mesh.bounds = context->createProgramFromPTXFile( ptx_path, "mesh_bounds" );
        mesh.material = material;

        loadMesh( filenames[i], mesh, xforms[i] ); 
        geometry_group->addChild( mesh.geom_instance );
    }

    geometry_group->setAcceleration( context->createAcceleration( "Trbvh" ) );

    context[ "top_object"   ]->set( geometry_group ); 

}


void setupCamera()
{
    camera_eye    = make_float3( 14.0f, 14.0f, 14.0f );
    camera_lookat = make_float3( 0.0f, 7.0f, 0.0f );
    camera_up   = make_float3( 0.0f, 1.0f,  0.0f );

    camera_rotate  = Matrix4x4::identity();
    camera_dirty = true;
}


void updateCamera()
{
    const float vfov  = 45.0f;
    const float aspect_ratio = static_cast<float>(width) /
        static_cast<float>(height);

    float3 camera_u, camera_v, camera_w;
    sutil::calculateCameraVariables(
            camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
            camera_u, camera_v, camera_w, /*fov_is_vertical*/ true );

    const Matrix4x4 frame = Matrix4x4::fromBasis(
            normalize( camera_u ),
            normalize( camera_v ),
            normalize( -camera_w ),
            camera_lookat);
    const Matrix4x4 frame_inv = frame.inverse();
    // Apply camera rotation twice to match old SDK behavior
    const Matrix4x4 trans   = frame*camera_rotate*camera_rotate*frame_inv;

    camera_eye    = make_float3( trans*make_float4( camera_eye,    1.0f ) );
    camera_lookat = make_float3( trans*make_float4( camera_lookat, 1.0f ) );
    camera_up     = make_float3( trans*make_float4( camera_up,     0.0f ) );

    sutil::calculateCameraVariables(
            camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
            camera_u, camera_v, camera_w, true );

    camera_rotate = Matrix4x4::identity();

    context["eye"]->setFloat( camera_eye );
    context["U"  ]->setFloat( camera_u );
    context["V"  ]->setFloat( camera_v );
    context["W"  ]->setFloat( camera_w );

    camera_dirty = false;
}



//------------------------------------------------------------------------------
//
//  GLUT callbacks
//
//------------------------------------------------------------------------------

void glutDisplay()
{
    static unsigned int accumulation_frame = 0;
    if( camera_dirty ) {
        updateCamera();
        accumulation_frame = 0;
    }

    context["frame"]->setUint( accumulation_frame++ );
    context->launch( 0, width, height );

    sutil::displayBufferGL( getOutputBuffer() );

    {
        static unsigned frame_count = 0;
        sutil::displayFps( frame_count++ );
    }

    glutSwapBuffers();
}


void glutKeyboardPress( unsigned char k, int x, int y )
{
    switch( k )
    {
        case( 'q' ):
        case( 27 ): // ESC
        {
            destroyContext();
            exit(0);
        }
        case( 's' ):
        {
            const std::string outputImage = std::string(SAMPLE_NAME) + ".ppm";
            std::cerr << "Saving current frame to '" << outputImage << "'\n";
            sutil::displayBufferPPM( outputImage.c_str(), getOutputBuffer() );
            break;
        }
    }
}


void glutMousePress( int button, int state, int x, int y )
{
    if( state == GLUT_DOWN )
    {
        mouse_button = button;
        mouse_prev_pos = make_int2( x, y );
    }
    else
    {
        // nothing
    }
}


void glutMouseMotion( int x, int y)
{
    if( mouse_button == GLUT_RIGHT_BUTTON )
    {
        const float dx = static_cast<float>( x - mouse_prev_pos.x ) /
                         static_cast<float>( width );
        const float dy = static_cast<float>( y - mouse_prev_pos.y ) /
                         static_cast<float>( height );
        const float dmax = fabsf( dx ) > fabs( dy ) ? dx : dy;
        const float scale = fminf( dmax, 0.9f );
        camera_eye = camera_eye + (camera_lookat - camera_eye)*scale;
        camera_dirty = true;
    }
    else if( mouse_button == GLUT_LEFT_BUTTON )
    {
        const float2 from = { static_cast<float>(mouse_prev_pos.x),
                              static_cast<float>(mouse_prev_pos.y) };
        const float2 to   = { static_cast<float>(x),
                              static_cast<float>(y) };

        const float2 a = { from.x / width, from.y / height };
        const float2 b = { to.x   / width, to.y   / height };

        camera_rotate = arcball.rotate( b, a );
        camera_dirty = true;
    }

    mouse_prev_pos = make_int2( x, y );
}


void glutResize( int w, int h )
{
    width  = w;
    height = h;
    camera_dirty = true;

    sutil::resizeBuffer( getOutputBuffer(), width, height );
    sutil::resizeBuffer( context[ "accum_buffer" ]->getBuffer(), width, height );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glViewport(0, 0, width, height);
    glutPostRedisplay();
}


void glutInitialize( int* argc, char** argv )
{
    glutInit( argc, argv );
    glutInitDisplayMode( GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE );
    glutInitWindowSize( width, height );
    glutInitWindowPosition( 100, 100 );
    glutCreateWindow( SAMPLE_NAME );
    glutHideWindow();
}


void glutRun()
{
    // Initialize GL state
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width, height);

    glutShowWindow();
    glutReshapeWindow( width, height);

    // register glut callbacks
    glutDisplayFunc( glutDisplay );
    glutIdleFunc( glutDisplay );
    glutReshapeFunc( glutResize );
    glutKeyboardFunc( glutKeyboardPress );
    glutMouseFunc( glutMousePress );
    glutMotionFunc( glutMouseMotion );

    registerExitHandler();

    glutMainLoop();
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
        "  -h | --help              Print this usage message and exit.\n"
        "  -f | --file              Save single frame to file and exit.\n"
        "  -n | --nopbo             Disable GL interop for display buffer.\n"
        "  -m | --mesh <mesh_file>  Specify path to mesh to be loaded.\n"
        "App Keystrokes:\n"
        "  q  Quit\n"
        "  s  Save image to '" << SAMPLE_NAME << ".ppm'\n"
        << std::endl;

    exit(1);
}

int main( int argc, char** argv )
{
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
        else if ( arg == "-m" || arg == "--mesh" )
        {
            if( i == argc-1 )
            {
                std::cerr << "Option '" << arg << "' requires additional argument.\n";
                printUsageAndExit( argv[0] );
            }
            mesh_files.push_back( argv[++i] );
            mesh_xforms.push_back( optix::Matrix4x4::identity() );
        }
        else if( arg == "-n" || arg == "--nopbo"  )
        {
            use_pbo = false;
        }
        else
        {
            std::cerr << "Unknown option '" << arg << "'\n";
            printUsageAndExit( argv[0] );
        }
    }

    try
    {
        glutInitialize( &argc, argv );

#ifndef __APPLE__
        glewInit();
#endif

        createContext();

        const float3 glass_color = make_float3( 1.0f, 1.0f, 1.0f );
        Material material = createMaterial( glass_color );

        if ( mesh_files.empty() ) {

            // Default scene

            mesh_files.push_back( std::string( sutil::samplesDir() ) + "/data/cognacglass.obj" );
            mesh_xforms.push_back( optix::Matrix4x4::translate( make_float3( 0.0f, 0.0f, -5.0f ) ) );

            mesh_files.push_back( std::string( sutil::samplesDir() ) + "/data/wineglass.obj" );
            mesh_xforms.push_back( optix::Matrix4x4::identity() );

            mesh_files.push_back( std::string( sutil::samplesDir() ) + "/data/waterglass.obj" );
            mesh_xforms.push_back( optix::Matrix4x4::translate( make_float3( -5.0f, 0.0f, 0.0f ) ) );
        }

        createGeometry( mesh_files, mesh_xforms, material );

        setupCamera();

        // Note: lighting comes from miss program

        context->validate();

        if ( out_file.empty() )
        {
            glutRun();
        }
        else
        {
            updateCamera();
            // Accumulate a few frames for anti-aliasing
            for ( unsigned int frame = 0; frame < 10; ++frame ) {
                context["frame"]->setUint( frame );
                context->launch( 0, width, height );
            }
            sutil::displayBufferPPM( out_file.c_str(), getOutputBuffer() );
            destroyContext();
        }
        return 0;
    }
    SUTIL_CATCH( context->get() )
}

