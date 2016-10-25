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

 /**
 * @file   optix_declarations.h
 * @author NVIDIA Corporation
 * @brief  OptiX public API declarations
 *
 * OptiX public API declarations
 */

/******************************************************************************\
 *
 * Contains declarations used by both optix host and device headers.
 *
\******************************************************************************/

#ifndef __optix_optix_declarations_h__
#define __optix_optix_declarations_h__

/************************************
 **
 **    Preprocessor macros 
 **
 ***********************************/

#if defined(__CUDACC__) || defined(__CUDABE__)
#  include <host_defines.h> /* For __host__ and __device__ */
#  define RT_HOSTDEVICE __host__ __device__
#else
#  define RT_HOSTDEVICE
#endif


/************************************
 **
 **    Enumerated values
 **
 ***********************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! OptiX formats */
typedef enum
{
  RT_FORMAT_UNKNOWN              = 0x100, /*!< Format unknown       */
  RT_FORMAT_FLOAT,                        /*!< Float                */
  RT_FORMAT_FLOAT2,                       /*!< sizeof(float)*2      */
  RT_FORMAT_FLOAT3,                       /*!< sizeof(float)*3      */
  RT_FORMAT_FLOAT4,                       /*!< sizeof(float)*2      */
  RT_FORMAT_BYTE,                         /*!< BYTE                 */
  RT_FORMAT_BYTE2,                        /*!< sizeof(CHAR)*2       */
  RT_FORMAT_BYTE3,                        /*!< sizeof(CHAR)*3       */
  RT_FORMAT_BYTE4,                        /*!< sizeof(CHAR)*4       */
  RT_FORMAT_UNSIGNED_BYTE,                /*!< UCHAR                */
  RT_FORMAT_UNSIGNED_BYTE2,               /*!< sizeof(UCHAR)*2      */
  RT_FORMAT_UNSIGNED_BYTE3,               /*!< sizeof(UCHAR)*3      */
  RT_FORMAT_UNSIGNED_BYTE4,               /*!< sizeof(UCHAR)*4      */
  RT_FORMAT_SHORT,                        /*!< SHORT                */
  RT_FORMAT_SHORT2,                       /*!< sizeof(SHORT)*2      */
  RT_FORMAT_SHORT3,                       /*!< sizeof(SHORT)*3      */
  RT_FORMAT_SHORT4,                       /*!< sizeof(SHORT)*4      */
  RT_FORMAT_UNSIGNED_SHORT,               /*!< USHORT               */
  RT_FORMAT_UNSIGNED_SHORT2,              /*!< sizeof(USHORT)*2     */
  RT_FORMAT_UNSIGNED_SHORT3,              /*!< sizeof(USHORT)*3     */
  RT_FORMAT_UNSIGNED_SHORT4,              /*!< sizeof(USHORT)*4     */
  RT_FORMAT_INT,                          /*!< INT                  */
  RT_FORMAT_INT2,                         /*!< sizeof(INT)*2        */
  RT_FORMAT_INT3,                         /*!< sizeof(INT)*3        */
  RT_FORMAT_INT4,                         /*!< sizeof(INT)*4        */
  RT_FORMAT_UNSIGNED_INT,                 /*!< sizeof(UINT)         */
  RT_FORMAT_UNSIGNED_INT2,                /*!< sizeof(UINT)*2       */
  RT_FORMAT_UNSIGNED_INT3,                /*!< sizeof(UINT)*3       */
  RT_FORMAT_UNSIGNED_INT4,                /*!< sizeof(UINT)*4       */
  RT_FORMAT_USER,                         /*!< User Format          */
  RT_FORMAT_BUFFER_ID,                    /*!< Buffer Id            */
  RT_FORMAT_PROGRAM_ID,                   /*!< Program Id           */
  RT_FORMAT_HALF,                         /*!< half float           */
  RT_FORMAT_HALF2,                        /*!< sizeof(half float)*2 */
  RT_FORMAT_HALF3,                        /*!< sizeof(half float)*3 */
  RT_FORMAT_HALF4                         /*!< sizeof(half float)*4 */
} RTformat;

/*! OptiX Object Types */
typedef enum
{
  RT_OBJECTTYPE_UNKNOWN          = 0x200,   /*!< Object Type Unknown       */
  RT_OBJECTTYPE_GROUP,                      /*!< Group Type                */
  RT_OBJECTTYPE_GEOMETRY_GROUP,             /*!< Geometry Group Type       */
  RT_OBJECTTYPE_TRANSFORM,                  /*!< Transform Type            */
  RT_OBJECTTYPE_SELECTOR,                   /*!< Selector Type             */
  RT_OBJECTTYPE_GEOMETRY_INSTANCE,          /*!< Geometry Instance Type    */
  RT_OBJECTTYPE_BUFFER,                     /*!< Buffer Type               */
  RT_OBJECTTYPE_TEXTURE_SAMPLER,            /*!< Texture Sampler Type      */
  RT_OBJECTTYPE_OBJECT,                     /*!< Object Type               */
  /* RT_OBJECTTYPE_PROGRAM - see below for entry */

  RT_OBJECTTYPE_MATRIX_FLOAT2x2,            /*!< Matrix Float 2x2          */
  RT_OBJECTTYPE_MATRIX_FLOAT2x3,            /*!< Matrix Float 2x3          */
  RT_OBJECTTYPE_MATRIX_FLOAT2x4,            /*!< Matrix Float 2x4          */
  RT_OBJECTTYPE_MATRIX_FLOAT3x2,            /*!< Matrix Float 3x2          */
  RT_OBJECTTYPE_MATRIX_FLOAT3x3,            /*!< Matrix Float 3x3          */
  RT_OBJECTTYPE_MATRIX_FLOAT3x4,            /*!< Matrix Float 3x4          */
  RT_OBJECTTYPE_MATRIX_FLOAT4x2,            /*!< Matrix Float 4x2          */
  RT_OBJECTTYPE_MATRIX_FLOAT4x3,            /*!< Matrix Float 4x3          */
  RT_OBJECTTYPE_MATRIX_FLOAT4x4,            /*!< Matrix Float 4x4          */

  RT_OBJECTTYPE_FLOAT,                      /*!< Float Type                */
  RT_OBJECTTYPE_FLOAT2,                     /*!< Float2 Type               */
  RT_OBJECTTYPE_FLOAT3,                     /*!< Float3 Type               */
  RT_OBJECTTYPE_FLOAT4,                     /*!< Float4 Type               */
  RT_OBJECTTYPE_INT,                        /*!< Integer Type              */
  RT_OBJECTTYPE_INT2,                       /*!< Integer2 Type             */
  RT_OBJECTTYPE_INT3,                       /*!< Integer3 Type             */  
  RT_OBJECTTYPE_INT4,                       /*!< Integer4 Type             */  
  RT_OBJECTTYPE_UNSIGNED_INT,               /*!< Unsigned Integer Type     */  
  RT_OBJECTTYPE_UNSIGNED_INT2,              /*!< Unsigned Integer2 Type    */
  RT_OBJECTTYPE_UNSIGNED_INT3,              /*!< Unsigned Integer3 Type    */
  RT_OBJECTTYPE_UNSIGNED_INT4,              /*!< Unsigned Integer4 Type    */
  RT_OBJECTTYPE_USER,                       /*!< User Object Type          */

  RT_OBJECTTYPE_PROGRAM                     /*!< Object Type Program - Added in OptiX 3.0         */
} RTobjecttype;


/*! Wrap mode */
typedef enum
{
  RT_WRAP_REPEAT,           /*!< Wrap repeat     */
  RT_WRAP_CLAMP_TO_EDGE,    /*!< Clamp to edge   */
  RT_WRAP_MIRROR,           /*!< Mirror          */
  RT_WRAP_CLAMP_TO_BORDER   /*!< Clamp to border */
} RTwrapmode;

/*! Filter mode */
typedef enum
{
  RT_FILTER_NEAREST,      /*!< Nearest     */
  RT_FILTER_LINEAR,       /*!< Linear      */
  RT_FILTER_NONE          /*!< No filter   */
} RTfiltermode;

/*! Texture read mode */
typedef enum
{
  RT_TEXTURE_READ_ELEMENT_TYPE = 0,           /*!< Read element type                                                                                              */
  RT_TEXTURE_READ_NORMALIZED_FLOAT = 1,       /*!< Read normalized float                                                                                          */
  RT_TEXTURE_READ_ELEMENT_TYPE_SRGB = 2,      /*!< Read element type and apply sRGB to linear conversion during texture read for 8-bit integer buffer formats     */
  RT_TEXTURE_READ_NORMALIZED_FLOAT_SRGB = 3   /*!< Read normalized float and apply sRGB to linear conversion during texture read for 8-bit integer buffer formats */
} RTtexturereadmode;

/*! GL Target */
typedef enum
{
  RT_TARGET_GL_TEXTURE_2D,            /*!< GL texture 2D           */
  RT_TARGET_GL_TEXTURE_RECTANGLE,     /*!< GL texture rectangle    */
  RT_TARGET_GL_TEXTURE_3D,            /*!< GL texture 3D           */
  RT_TARGET_GL_RENDER_BUFFER,         /*!< GL render buffer        */
  RT_TARGET_GL_TEXTURE_1D,            /*!< GL texture 1D           */
  RT_TARGET_GL_TEXTURE_1D_ARRAY,      /*!< GL array of 1D textures */
  RT_TARGET_GL_TEXTURE_2D_ARRAY,      /*!< GL array of 2D textures */
  RT_TARGET_GL_TEXTURE_CUBE_MAP,      /*!< GL cube map texture     */
  RT_TARGET_GL_TEXTURE_CUBE_MAP_ARRAY /*!< GL array of cube maps   */
} RTgltarget;

/*! Texture index mode */
typedef enum
{
  RT_TEXTURE_INDEX_NORMALIZED_COORDINATES,    /*!< Texture Index normalized coordinates */
  RT_TEXTURE_INDEX_ARRAY_INDEX                /*!< Texture Index Array */
} RTtextureindexmode;

/*! Buffer type */
typedef enum
{
  RT_BUFFER_INPUT                = 0x1,                               /*!< Input buffer for the GPU          */
  RT_BUFFER_OUTPUT               = 0x2,                               /*!< Output buffer for the GPU         */
  RT_BUFFER_INPUT_OUTPUT         = RT_BUFFER_INPUT | RT_BUFFER_OUTPUT,/*!< Ouput/Input buffer for the GPU    */
  RT_BUFFER_PROGRESSIVE_STREAM   = 0x10,                              /*!< Progressive stream buffer         */
} RTbuffertype;

/*! Buffer flags */
typedef enum
{
  RT_BUFFER_GPU_LOCAL            = 0x4, /*!< An @ref RT_BUFFER_INPUT_OUTPUT has separate copies on each device that are not synchronized                               */
  RT_BUFFER_COPY_ON_DIRTY        = 0x8, /*!< A CUDA Interop buffer will only be synchronized across devices when dirtied by @ref rtBufferMap or @ref rtBufferMarkDirty */
  RT_BUFFER_LAYERED              = 0x200000, /*!< Depth specifies the number of layers, not the depth of a 3D array */
  RT_BUFFER_CUBEMAP              = 0x400000 /*!< Enables creation of cubemaps. If this flag is set, Width must be equal to Height, and Depth must be six. If the @ref RT_BUFFER_LAYERED flag is also set, then Depth must be a multiple of six */
} RTbufferflag;

/*! Buffer mapping flags */
typedef enum
{
  RT_BUFFER_MAP_READ            = 0x1, /*!< Map buffer memory for reading */  
  RT_BUFFER_MAP_READ_WRITE      = 0x2, /*!< Map buffer memory for both reading and writing */
  RT_BUFFER_MAP_WRITE           = 0x4, /*!< Map buffer memory for writing */
  RT_BUFFER_MAP_WRITE_DISCARD   = 0x8  /*!< Map buffer memory for writing, with the previous contents being undefined*/
} RTbuffermapflag;

/*! Exceptions */
typedef enum
{
  RT_EXCEPTION_PROGRAM_ID_INVALID           = 0x3EE,    /*!< Program ID not valid       */
  RT_EXCEPTION_TEXTURE_ID_INVALID           = 0x3EF,    /*!< Texture ID not valid       */
  RT_EXCEPTION_BUFFER_ID_INVALID            = 0x3FA,    /*!< Buffer ID not valid        */
  RT_EXCEPTION_INDEX_OUT_OF_BOUNDS          = 0x3FB,    /*!< Index out of bounds        */
  RT_EXCEPTION_STACK_OVERFLOW               = 0x3FC,    /*!< Stack overflow             */
  RT_EXCEPTION_BUFFER_INDEX_OUT_OF_BOUNDS   = 0x3FD,    /*!< Buffer index out of bounds */  
  RT_EXCEPTION_INVALID_RAY                  = 0x3FE,    /*!< Invalid ray                */
  RT_EXCEPTION_INTERNAL_ERROR               = 0x3FF,    /*!< Internal error             */
  RT_EXCEPTION_USER                         = 0x400,    /*!< User exception             */

  RT_EXCEPTION_ALL                          = 0x7FFFFFFF  /*!< All exceptions        */
} RTexception;

/*! Result */
typedef enum
{
  RT_SUCCESS                           = 0,       /*!< Success                      */

  RT_TIMEOUT_CALLBACK                  = 0x100,   /*!< Timeout callback             */

  RT_ERROR_INVALID_CONTEXT             = 0x500,   /*!< Invalid Context              */
  RT_ERROR_INVALID_VALUE               = 0x501,   /*!< Invalid Value                */
  RT_ERROR_MEMORY_ALLOCATION_FAILED    = 0x502,   /*!< Timeout callback             */
  RT_ERROR_TYPE_MISMATCH               = 0x503,   /*!< Type Mismatch                */
  RT_ERROR_VARIABLE_NOT_FOUND          = 0x504,   /*!< Variable not found           */
  RT_ERROR_VARIABLE_REDECLARED         = 0x505,   /*!< Variable redeclared          */
  RT_ERROR_ILLEGAL_SYMBOL              = 0x506,   /*!< Illegal symbol               */
  RT_ERROR_INVALID_SOURCE              = 0x507,   /*!< Invalid source               */
  RT_ERROR_VERSION_MISMATCH            = 0x508,   /*!< Version mismatch             */

  RT_ERROR_OBJECT_CREATION_FAILED      = 0x600,   /*!< Object creation failed       */
  RT_ERROR_NO_DEVICE                   = 0x601,   /*!< No device                    */
  RT_ERROR_INVALID_DEVICE              = 0x602,   /*!< Invalid device               */
  RT_ERROR_INVALID_IMAGE               = 0x603,   /*!< Invalid image                */
  RT_ERROR_FILE_NOT_FOUND              = 0x604,   /*!< File not found               */
  RT_ERROR_ALREADY_MAPPED              = 0x605,   /*!< Already mapped               */
  RT_ERROR_INVALID_DRIVER_VERSION      = 0x606,   /*!< Invalid driver version       */
  RT_ERROR_CONTEXT_CREATION_FAILED     = 0x607,   /*!< Context creation failed      */

  RT_ERROR_RESOURCE_NOT_REGISTERED     = 0x608,   /*!< Resource not registered      */
  RT_ERROR_RESOURCE_ALREADY_REGISTERED = 0x609,   /*!< Resource already registered  */

  RT_ERROR_LAUNCH_FAILED               = 0x900,   /*!< Launch failed                */

  RT_ERROR_NOT_SUPPORTED               = 0xA00,   /*!< Not supported                */

  RT_ERROR_CONNECTION_FAILED           = 0xB00,   /*!< Connection failed            */
  RT_ERROR_AUTHENTICATION_FAILED       = 0xB01,   /*!< Authentication failed        */
  RT_ERROR_CONNECTION_ALREADY_EXISTS   = 0xB02,   /*!< Connection already exists    */
  RT_ERROR_NETWORK_LOAD_FAILED         = 0xB03,   /*!< Network component failed to load */
  RT_ERROR_NETWORK_INIT_FAILED         = 0xB04,   /*!< Network initialization failed*/
  RT_ERROR_CLUSTER_NOT_RUNNING         = 0xB06,   /*!< No cluster is running        */
  RT_ERROR_CLUSTER_ALREADY_RUNNING     = 0xB07,   /*!< Cluster is already running   */
  RT_ERROR_INSUFFICIENT_FREE_NODES     = 0xB08,   /*!< Not enough free nodes        */

  RT_ERROR_UNKNOWN                     = ~0       /*!< Error unknown                */
} RTresult;

/*! Device attributes */
typedef enum
{
  RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK,      /*!< Max Threads per Block */
  RT_DEVICE_ATTRIBUTE_CLOCK_RATE,                 /*!< Clock rate */
  RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT,       /*!< Multiprocessor count */
  RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED,  /*!< Execution timeout enabled */
  RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT, /*!< Hardware Texture count */
  RT_DEVICE_ATTRIBUTE_NAME,                       /*!< Attribute Name */
  RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY,         /*!< Compute Capabilities */
  RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY,               /*!< Total Memory */
  RT_DEVICE_ATTRIBUTE_TCC_DRIVER,                 /*!< sizeof(int) */
  RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL,        /*!< sizeof(int) */
} RTdeviceattribute;

/*! RemoteDevice attributes */
typedef enum
{
  RT_REMOTEDEVICE_ATTRIBUTE_CLUSTER_URL,          /*!< URL for the Cluster Manager */
  RT_REMOTEDEVICE_ATTRIBUTE_HEAD_NODE_URL,        /*!< URL for the Head Node */
  RT_REMOTEDEVICE_ATTRIBUTE_NUM_CONFIGURATIONS,   /*!< Number of available configurations */
  RT_REMOTEDEVICE_ATTRIBUTE_STATUS,               /*!< Status */
  RT_REMOTEDEVICE_ATTRIBUTE_NUM_TOTAL_NODES,      /*!< Number of total nodes */
  RT_REMOTEDEVICE_ATTRIBUTE_NUM_FREE_NODES,       /*!< Number of free nodes */
  RT_REMOTEDEVICE_ATTRIBUTE_NUM_RESERVED_NODES,   /*!< Number of reserved nodes */
  RT_REMOTEDEVICE_ATTRIBUTE_NAME,                 /*!< Name */
  RT_REMOTEDEVICE_ATTRIBUTE_NUM_GPUS,             /*!< Number of GPUs */
  RT_REMOTEDEVICE_ATTRIBUTE_GPU_TOTAL_MEMORY,     /*!< Total Memory (per GPU, in bytes) */
  RT_REMOTEDEVICE_ATTRIBUTE_CONFIGURATIONS=0x040000000  /*!< List of descriptions for the available configurations */
} RTremotedeviceattribute;

typedef enum
{
  RT_REMOTEDEVICE_STATUS_READY,                   /*!< RemoteDevice Status Ready */
  RT_REMOTEDEVICE_STATUS_CONNECTED,               /*!< RemoteDevice Status Connected */
  RT_REMOTEDEVICE_STATUS_RESERVED,                /*!< RemoteDevice Status Reserved */

  RT_REMOTEDEVICE_STATUS_DISCONNECTED = ~0        /*!< RemoteDevice Status Disconnected */
} RTremotedevicestatus;

/*! Context attributes */
typedef enum
{
  RT_CONTEXT_ATTRIBUTE_MAX_TEXTURE_COUNT,                    /*!< sizeof(int)    */
  RT_CONTEXT_ATTRIBUTE_CPU_NUM_THREADS,                      /*!< sizeof(int)    */
  RT_CONTEXT_ATTRIBUTE_USED_HOST_MEMORY,                     /*!< sizeof(RTsize) */
  RT_CONTEXT_ATTRIBUTE_GPU_PAGING_ACTIVE,                    /*!< sizeof(int)    */
  RT_CONTEXT_ATTRIBUTE_GPU_PAGING_FORCED_OFF,                /*!< sizeof(int)    */
  RT_CONTEXT_ATTRIBUTE_AVAILABLE_DEVICE_MEMORY = 0x10000000  /*!< sizeof(RTsize) */
} RTcontextattribute;

/*! Buffer attributes */
typedef enum
{
  RT_BUFFER_ATTRIBUTE_STREAM_FORMAT,                          /*!< Format string */
  RT_BUFFER_ATTRIBUTE_STREAM_BITRATE,                         /*!< sizeof(int) */
  RT_BUFFER_ATTRIBUTE_STREAM_FPS,                             /*!< sizeof(int) */
  RT_BUFFER_ATTRIBUTE_STREAM_GAMMA                            /*!< sizeof(float) */
} RTbufferattribute;

/*! Sentinel values */
typedef enum { 
  RT_BUFFER_ID_NULL   = 0 /*!< sentinel for describing a non-existent buffer id  */ 
} RTbufferidnull;
typedef enum {
  RT_PROGRAM_ID_NULL  = 0 /*!< sentinel for describing a non-existent program id */ 
} RTprogramidnull;
typedef enum {
  RT_TEXTURE_ID_NULL  = 0 /*!< sentinel for describing a non-existent texture id */ 
} RTtextureidnull;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __optix_optix_declarations_h__ */
