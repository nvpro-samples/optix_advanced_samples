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

/*
 * optixHello.cpp -- Renders a solid green image.
 *
 * A filename can be given on the command line to write the results to file. 
 */

#include <optix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sutil.h>


void printUsageAndExit( const char* argv0 );


int main(int argc, char* argv[])
{
    RTcontext context = 0;

    try { 

        /* Primary RTAPI objects */
        RTprogram ray_gen_program;
        RTbuffer  buffer;

        /* Parameters */
        RTvariable result_buffer;
        RTvariable draw_color;

        char path_to_ptx[512];
        char outfile[512];

        int width  = 512u;
        int height = 384u;
        int i;

        outfile[0] = '\0';

        sutil::initGLFW();

        for( i = 1; i < argc; ++i ) {
            if( strcmp( argv[i], "--help" ) == 0 || strcmp( argv[i], "-h" ) == 0 ) {
                printUsageAndExit( argv[0] );
            } else if( strcmp( argv[i], "--file" ) == 0 || strcmp( argv[i], "-f" ) == 0 ) {
                if( i < argc-1 ) {
                    strcpy( outfile, argv[++i] );
                } else {
                    printUsageAndExit( argv[0] );
                }
            } else if ( strncmp( argv[i], "--dim=", 6 ) == 0 ) {
                const char *dims_arg = &argv[i][6];
                sutil::parseDimensions( dims_arg, width, height );
            } else {
                fprintf( stderr, "Unknown option '%s'\n", argv[i] );
                printUsageAndExit( argv[0] );
            }
        }

        /* Create our objects and set state */
        RT_CHECK_ERROR( rtContextCreate( &context ) );
        RT_CHECK_ERROR( rtContextSetRayTypeCount( context, 1 ) );
        RT_CHECK_ERROR( rtContextSetEntryPointCount( context, 1 ) );

        RT_CHECK_ERROR( rtBufferCreate( context, RT_BUFFER_OUTPUT, &buffer ) );
        RT_CHECK_ERROR( rtBufferSetFormat( buffer, RT_FORMAT_FLOAT4 ) );
        RT_CHECK_ERROR( rtBufferSetSize2D( buffer, width, height ) );
        RT_CHECK_ERROR( rtContextDeclareVariable( context, "result_buffer", &result_buffer ) );
        RT_CHECK_ERROR( rtVariableSetObject( result_buffer, buffer ) );

        sprintf( path_to_ptx, "%s/%s", sutil::samplesPTXDir(), "optixHello_generated_draw_color.cu.ptx" );
        RT_CHECK_ERROR( rtProgramCreateFromPTXFile( context, path_to_ptx, "draw_solid_color", &ray_gen_program ) );
        RT_CHECK_ERROR( rtProgramDeclareVariable( ray_gen_program, "draw_color", &draw_color ) );
        RT_CHECK_ERROR( rtVariableSet3f( draw_color, 0.462f, 0.725f, 0.0f ) );
        RT_CHECK_ERROR( rtContextSetRayGenerationProgram( context, 0, ray_gen_program ) );

        /* Run */
        RT_CHECK_ERROR( rtContextValidate( context ) );
        RT_CHECK_ERROR( rtContextLaunch2D( context, 0 /* entry point */, width, height ) );

        /* Display image */
        if( strlen( outfile ) == 0 ) {
            sutil::displayBufferGLFW( argv[0], buffer );
        } else {
            sutil::writeBufferToFile( outfile, buffer );
        }

        /* Clean up */
        RT_CHECK_ERROR( rtBufferDestroy( buffer ) );
        RT_CHECK_ERROR( rtProgramDestroy( ray_gen_program ) );
        RT_CHECK_ERROR( rtContextDestroy( context ) );

        return( 0 );

    } SUTIL_CATCH( context )
}


void printUsageAndExit( const char* argv0 )
{
  fprintf( stderr, "Usage  : %s [options]\n", argv0 );
  fprintf( stderr, "Options: --file | -f <filename>      Specify file for image output\n" );
  fprintf( stderr, "         --help | -h                 Print this usage message\n" );
  fprintf( stderr, "         --dim=<width>x<height>      Set image dimensions; defaults to 512x384\n" );
  exit(1);
}


