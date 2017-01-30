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

// A reader for the VOX file format specified in MagicaVoxel-file-format-vox.txt.
//
// Currently ignores material (MATT) chunks.

#include "read_vox.h"

#include <optixu/optixu_math_namespace.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


// Runtime assertion that does not get disabled in Release builds
#define ASSERT( condition )                                                                                         \
  do                                                                                                                \
  {                                                                                                                 \
    if( !( condition ) )                                                                                            \
      throw std::runtime_error( std::string( __FILE__ ) + ":" + std::to_string( __LINE__) + ": " + #condition );    \
  } while( 0 )


#define DO_DEBUG_PRINT 0
#if DO_DEBUG_PRINT
#define DEBUG_PRINT( x )  \
  do                      \
  {                       \
    std::cerr << x;       \
  } while( 0 )
#else
#define DEBUG_PRINT( x )
#endif


static const unsigned int default_palette[256] = {
	0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
	0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
	0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
	0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
	0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
	0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
	0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
	0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
	0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
	0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
	0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
	0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
	0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
	0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
	0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
	0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
};


struct ChunkHeader
{
    char id[5];
    int  num_bytes;
    int  num_child_bytes;
};

void debugChunkHeader( const ChunkHeader& header )
{
    std::cerr << "chunk id             : " << header.id << std::endl;
    std::cerr << "chunk num_bytes      : " << header.num_bytes << std::endl;
    std::cerr << "chunk num_child_bytes: " << header.num_child_bytes << std::endl;
}

bool readChunkHeader( FILE* f, ChunkHeader& h )
{
    ChunkHeader header;
    if( fread( header.id, sizeof(char), 4, f ) != 4 ) return false;
    header.id[4] = '\0';
    if ( fread( &header.num_bytes, sizeof(int), 1, f ) != 1 ) return false;
    if ( fread( &header.num_child_bytes, sizeof(int), 1, f ) != 1 ) return false;

    h = header;

#if DO_DEBUG_PRINT
    debugChunkHeader( h );
#endif

    return true;
}


// Given a SIZE chunk header, read the SIZE and XYZI data into a voxel model
void readVoxelModel( FILE* f, ChunkHeader child_header, VoxelModel& model )
{
    ASSERT( strcmp( child_header.id, "SIZE" ) == 0 );
    if ( fread( model.dims, sizeof(int), 3, f ) != 3 ) return;

    // Switch from z-up to y-up to match other OptiX samples
    std::swap( model.dims[1], model.dims[2] );

    DEBUG_PRINT( "model dims: " << model.dims[0] << " " << model.dims[1] << " " << model.dims[2] << std::endl );

    ChunkHeader voxel_header;
    readChunkHeader( f, voxel_header );

    int num_voxels = -1;
    if ( fread( &num_voxels, sizeof(int), 1, f ) != 1 ) return;
    ASSERT( num_voxels <= model.dims[0] * model.dims[1] * model.dims[2] );

    DEBUG_PRINT( "num_voxels: " << num_voxels << std::endl );

    model.voxels.reserve( num_voxels );
    for (int i = 0; i < num_voxels; ++i) {
        optix::uchar4 voxel;
        if ( fread( &voxel.x, sizeof(char), 4, f ) != 4 ) return;

        // Switch from z-up to y-up to match other OptiX samples
        std::swap( voxel.y, voxel.z );
        voxel.z = model.dims[2] - voxel.z;
        model.voxels.push_back( voxel );
        
        // Note:
        // Have seen models with voxel index == dim, which should be illegal.  
        // Allow this anyway; the voxel index is not used to directly access an array.
#if 0
        ASSERT( voxel.x >= 0 && voxel.x < model.dims[0] ); 
        ASSERT( voxel.y >= 0 && voxel.y < model.dims[1] ); 
        ASSERT( voxel.z >= 0 && voxel.z < model.dims[2] ); 
#endif
        ASSERT( voxel.w >= 1 );  // Note 1-based indexing for color index
    }
}

void debugPalette( const optix::uchar4* pal )
{
    for ( int i = 1; i < 256; ++i ) {
        std::cerr << (int)pal[i].x << " " << (int)pal[i].y << " " << (int)pal[i].z << " " << (int)pal[i].w << std::endl;
    }
}


void read_vox( const char* filename, std::vector< VoxelModel >& models, optix::uchar4 palette[256] )
{
    FILE* f = fopen( filename, "rb" );
    if ( !f ) {
        std::cerr << "Could not open file: " << filename << std::endl;
        ASSERT( f );
    }
    char magic[5];
    magic[4] = '\0';
    ASSERT ( fread( magic, sizeof(char), 4, f ) == 4 );
    ASSERT( (strcmp( magic, "VOX " ) == 0) && "File is a VOX file" );
    
    int version = 0;
    ASSERT ( fread( &version, sizeof(int), 1, f ) == 1 );

    ChunkHeader main_header;
    readChunkHeader( f, main_header );

    ChunkHeader child_header;
    readChunkHeader( f, child_header );

    int num_models = 1;
    if ( strcmp( child_header.id, "PACK" ) == 0 ) {
        ASSERT ( fread( &num_models, sizeof(int), 1, f ) == 1 );
        DEBUG_PRINT( "found pack, num_models = " << num_models << std::endl );

        // Read first SIZE block to match single-model case
        readChunkHeader( f, child_header );

    } 

    // Read models (pairs of SIZE, XYZI chunks)

    {
        VoxelModel model;
        readVoxelModel( f, child_header, model );
        models.push_back( model );
    }

    // Read extra models in pack
    for (int i = 0; i < num_models-1; ++i) {
        readChunkHeader( f, child_header );
         
        VoxelModel model;
        readVoxelModel( f, child_header, model );
        models.push_back( model );
    }

    // Read optional palette
    bool found_palette = false;
    if ( readChunkHeader( f, child_header ) ) {
        if ( strcmp( child_header.id, "RGBA" ) == 0 ) {
            ASSERT ( fread( palette, sizeof(optix::uchar4), 256, f ) == 256 );
            found_palette = true;
        } else {
            std::cerr << "********************* " << filename << std::endl;
            std::cerr << "********************* Ignoring chunk: " << child_header.id << std::endl;
        }
    }
    
    if ( !found_palette ) {
        const optix::uchar4* src = reinterpret_cast< const optix::uchar4* >( default_palette );
        std::copy( src, src+256, palette );
    }

    //debugPalette( palette );

    if ( readChunkHeader( f, child_header ) ) {
        std::cerr << "********************* " << filename << std::endl;
        std::cerr << "********************* Ignoring chunk " << child_header.id << std::endl;
    }
    

    fclose( f );
}

#if 0
int main( int argc, char ** argv )
{
    
    ASSERT( argc > 1 );
    readVox( argv[1] );
    
    return 0;
}
#endif

