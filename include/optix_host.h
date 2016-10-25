
/*
 * Copyright (c) 2008 - 2015 NVIDIA Corporation.  All rights reserved.
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
 * @file   optix_host.h
 * @author NVIDIA Corporation
 * @brief  OptiX public API
 *
 * OptiX public API Reference - Host side
 */

#ifndef __optix_optix_host_h__
#define __optix_optix_host_h__

#ifndef RTAPI
#if defined(_WIN32)
#define RTAPI __declspec(dllimport)
#else
#define RTAPI
#endif
#endif

#include "internal/optix_declarations.h"


/************************************
 **
 **    Platform-Dependent Types
 **
 ***********************************/

#if defined(_WIN64)
typedef unsigned __int64    RTsize;
#elif defined(_WIN32)
typedef unsigned int        RTsize;
#else
typedef long unsigned int   RTsize;
#endif

/************************************
 **
 **    Opaque Object Types
 **
 ***********************************/

/** Opaque type to handle Acceleration Structures - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTacceleration_api       * RTacceleration;
/** Opaque type to handle Buffers - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTbuffer_api             * RTbuffer;
/** Opaque type to handle Contexts - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTcontext_api            * RTcontext;
/** Opaque type to handle Geometry - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTgeometry_api           * RTgeometry;
/** Opaque type to handle Geometry Instance - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTgeometryinstance_api   * RTgeometryinstance;
/** Opaque type to handle Geometry Group - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTgeometrygroup_api      * RTgeometrygroup;
/** Opaque type to handle Group - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTgroup_api              * RTgroup;
/** Opaque type to handle Material - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTmaterial_api           * RTmaterial;
/** Opaque type to handle Program - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTprogram_api            * RTprogram;
/** Opaque type to handle Selector - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTselector_api           * RTselector;
/** Opaque type to handle Texture Sampler - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTtexturesampler_api     * RTtexturesampler;
/** Opaque type to handle Transform - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTtransform_api          * RTtransform;
/** Opaque type to handle Variable - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTvariable_api           * RTvariable;
/** Opaque type to handle Object - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef void                            * RTobject;
/** Opaque type to handle RemoteDevice - Note that the *_api type should never be used directly.
Only the typedef target name will be guaranteed to remain unchanged */
typedef struct RTremotedevice_api       * RTremotedevice;

/************************************
 **
 **    Callback Function Types
 **
 ***********************************/

/** Callback signature for use with rtContextSetTimeoutCallback.
 * Return 1 to ask for abort, 0 to continue. */
typedef int (*RTtimeoutcallback)(void);


#ifdef __cplusplus
extern "C" {
#endif

/************************************
 **
 **    Context-free functions
 **
 ***********************************/

  /**
  * @brief Returns the current OptiX version
  *
  * @ingroup ContextFreeFunctions
  *
  * <B>Description</B>
  *
  * @ref rtGetVersion returns in \a version a numerically comparable
  * version number of the current OptiX library.
  *
  * The encoding for the version number prior to OptiX 4.0.0 is major*1000 + minor*10 + micro.  
  * For versions 4.0.0 and higher, the encoding is major*10000 + minor*100 + micro.
  * For example, for version 3.5.1 this function would return 3051, and for version 4.5.1 it would return 40501.
  *
  * @param[out]  version   OptiX version number
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGetVersion was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtDeviceGetDeviceCount
  *
  */
  RTresult RTAPI rtGetVersion(unsigned int* version);

  /**
  * @brief Returns the number of OptiX capable devices
  *
  * @ingroup ContextFreeFunctions
  *
  * <B>Description</B>
  *
  * @ref rtDeviceGetDeviceCount returns in \a count the number of compute
  * devices that are available in the host system and will be used by
  * OptiX.
  *
  * @param[out]  count   Number devices available for OptiX
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtDeviceGetDeviceCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGetVersion
  *
  */
  RTresult RTAPI rtDeviceGetDeviceCount(unsigned int* count);

  /**
  * @brief Returns an attribute specific to an OptiX device
  *
  * @ingroup ContextFreeFunctions
  *
  * <B>Description</B>
  *
  * @ref rtDeviceGetAttribute returns in \a p the value of the per device attribute
  * specified by \a attrib for device \a ordinal.
  *
  * Each attribute can have a different size.  The sizes are given in the following list:
  *
  *   - @ref RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK        sizeof(int)
  *   - @ref RT_DEVICE_ATTRIBUTE_CLOCK_RATE                   sizeof(int)
  *   - @ref RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT         sizeof(int)
  *   - @ref RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED    sizeof(int)
  *   - @ref RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT   sizeof(int)
  *   - @ref RT_DEVICE_ATTRIBUTE_NAME                         up to size-1
  *   - @ref RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY           sizeof(int2)
  *   - @ref RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY                 sizeof(RTsize)
  *   - @ref RT_DEVICE_ATTRIBUTE_TCC_DRIVER                   sizeof(int)
  *   - @ref RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL          sizeof(int)
  *
  * @param[in]   ordinal   OptiX device ordinal
  * @param[in]   attrib    Attribute to query
  * @param[in]   size      Size of the attribute being queried.  Parameter \a p must have at least this much memory allocated
  * @param[out]  p         Return pointer where the value of the attribute will be copied into.  This must point to at least \a size bytes of memory
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE - Can be returned if size does not match the proper size of the attribute, if \a p is
  * \a NULL, or if \a ordinal does not correspond to an OptiX device
  *
  * <B>History</B>
  *
  * @ref rtDeviceGetAttribute was introduced in OptiX 2.0.
  * @ref RT_DEVICE_ATTRIBUTE_TCC_DRIVER was introduced in OptiX 3.0.
  * @ref RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL was introduced in OptiX 3.0.
  *
  * <B>See also</B>
  * @ref rtDeviceGetDeviceCount,
  * @ref rtContextGetAttribute
  *
  */
  RTresult RTAPI rtDeviceGetAttribute(int ordinal, RTdeviceattribute attrib, RTsize size, void* p);


/************************************
 **
 **    Object Variable Accessors
 **
 ***********************************/

  /* Sets */
  /**
  * @ingroup rtVariableSet Variable setters
  *
  * @brief Functions designed to modify the value of a program variable
  *
  * <B>Description</B>
  *
  * @ref rtVariableSet functions modify the value of a program variable or variable array. The
  * target variable is specificed by \a v, which should be a value returned by
  * @ref rtContextGetVariable.
  *
  * The commands \a rtVariableSet{1-2-3-4}{f-i-ui}v are used to modify the value of a
  * program variable specified by \a v using the values passed as arguments.
  * The number specified in the command should match the number of components in
  * the data type of the specified program variable (e.g., 1 for float, int,
  * unsigned int; 2 for float2, int2, uint2, etc.). The suffix \a f indicates
  * that \a v has floating point type, the suffix \a i indicates that
  * \a v has integral type, and the suffix \a ui indicates that that
  * \a v has unsigned integral type. The \a v variants of this function
  * should be used to load the program variable's value from the array specified by
  * parameter \a v. In this case, the array \a v should contain as many elements as
  * there are program variable components.
  *
  * The commands \a rtVariableSetMatrix{2-3-4}x{2-3-4}fv are used to modify the value
  * of a program variable whose data type is a matrix. The numbers in the command
  * names are the number of rows and columns, respectively.
  * For example, \a 2x4 indicates a matrix with 2 rows and 4 columns (i.e., 8 values).
  * If \a transpose is \a 0, the matrix is specified in row-major order, otherwise
  * in column-major order or, equivalently, as a matrix with the number of rows and
  * columns swapped in row-major order.
  *
  * If \a v is not a valid variable, these calls have no effect and return
  * @ref RT_ERROR_INVALID_VALUE
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtVariableSet were introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtVariableGet,
  * @ref rtVariableSet,
  * @ref rtDeclareVariable
  *
  * @{
  */
  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f1         Specifies the new float value of the program variable
  */
  RTresult RTAPI rtVariableSet1f(RTvariable v, float f1);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f1         Specifies the new float value of the program variable
  * @param[in]   f2         Specifies the new float value of the program variable
  */
  RTresult RTAPI rtVariableSet2f(RTvariable v, float f1, float f2);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f1         Specifies the new float value of the program variable
  * @param[in]   f2         Specifies the new float value of the program variable
  * @param[in]   f3         Specifies the new float value of the program variable
  */
  RTresult RTAPI rtVariableSet3f(RTvariable v, float f1, float f2, float f3);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f1         Specifies the new float value of the program variable
  * @param[in]   f2         Specifies the new float value of the program variable
  * @param[in]   f3         Specifies the new float value of the program variable
  * @param[in]   f4         Specifies the new float value of the program variable
  */
  RTresult RTAPI rtVariableSet4f(RTvariable v, float f1, float f2, float f3, float f4);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f          Array of float values to set the variable to
  */
  RTresult RTAPI rtVariableSet1fv(RTvariable v, const float* f);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f          Array of float values to set the variable to
  */
  RTresult RTAPI rtVariableSet2fv(RTvariable v, const float* f);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f          Array of float values to set the variable to
  */
  RTresult RTAPI rtVariableSet3fv(RTvariable v, const float* f);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   f          Array of float values to set the variable to
  */
  RTresult RTAPI rtVariableSet4fv(RTvariable v, const float* f);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i1         Specifies the new integer value of the program variable
  */
  RTresult RTAPI rtVariableSet1i(RTvariable v, int i1);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i1         Specifies the new integer value of the program variable
  * @param[in]   i2         Specifies the new integer value of the program variable
  */
  RTresult RTAPI rtVariableSet2i(RTvariable v, int i1, int i2);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i1         Specifies the new integer value of the program variable
  * @param[in]   i2         Specifies the new integer value of the program variable
  * @param[in]   i3         Specifies the new integer value of the program variable
  */
  RTresult RTAPI rtVariableSet3i(RTvariable v, int i1, int i2, int i3);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i1         Specifies the new integer value of the program variable
  * @param[in]   i2         Specifies the new integer value of the program variable
  * @param[in]   i3         Specifies the new integer value of the program variable
  * @param[in]   i4         Specifies the new integer value of the program variable
  */
  RTresult RTAPI rtVariableSet4i(RTvariable v, int i1, int i2, int i3, int i4);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i          Array of integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet1iv(RTvariable v, const int* i);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i          Array of integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet2iv(RTvariable v, const int* i);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i          Array of integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet3iv(RTvariable v, const int* i);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   i          Array of integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet4iv(RTvariable v, const int* i);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u1         Specifies the new unsigned integer value of the program variable
  */
  RTresult RTAPI rtVariableSet1ui(RTvariable v, unsigned int u1);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u1         Specifies the new unsigned integer value of the program variable
  * @param[in]   u2         Specifies the new unsigned integer value of the program variable
  */
  RTresult RTAPI rtVariableSet2ui(RTvariable v, unsigned int u1, unsigned int u2);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u1         Specifies the new unsigned integer value of the program variable
  * @param[in]   u2         Specifies the new unsigned integer value of the program variable
  * @param[in]   u3         Specifies the new unsigned integer value of the program variable
  */
  RTresult RTAPI rtVariableSet3ui(RTvariable v, unsigned int u1, unsigned int u2, unsigned int u3);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u1         Specifies the new unsigned integer value of the program variable
  * @param[in]   u2         Specifies the new unsigned integer value of the program variable
  * @param[in]   u3         Specifies the new unsigned integer value of the program variable
  * @param[in]   u4         Specifies the new unsigned integer value of the program variable
  */
  RTresult RTAPI rtVariableSet4ui(RTvariable v, unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u          Array of unsigned integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet1uiv(RTvariable v, const unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u          Array of unsigned integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet2uiv(RTvariable v, const unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u          Array of unsigned integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet3uiv(RTvariable v, const unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   u          Array of unsigned integer values to set the variable to
  */
  RTresult RTAPI rtVariableSet4uiv(RTvariable v, const unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix2x2fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix2x3fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix2x4fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix3x2fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix3x3fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix3x4fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix4x2fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix4x3fv(RTvariable v, int transpose, const float* m);

  /**
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   transpose  Specifies row-major or column-major order
  * @param[in]   m          Array of float values to set the matrix to
  */
  RTresult RTAPI rtVariableSetMatrix4x4fv(RTvariable v, int transpose, const float* m);
  /**
  * @}
  */

  /**
  * @brief Sets a program variable value to a OptiX object
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableSetObject sets a program variable to an OptiX object value.  The target
  * variable is specified by \a v. The new value of the program variable is
  * specified by \a object. The concrete type of \a object can be one of @ref RTbuffer,
  * @ref RTtexturesampler, @ref RTgroup, @ref RTprogram, @ref RTselector, @ref
  * RTgeometrygroup, or @ref RTtransform.  If \a v is not a valid variable or \a
  * object is not a valid OptiX object, this call has no effect and returns @ref
  * RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   v          Specifies the program variable to be set
  * @param[in]   object     Specifies the new value of the program variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtVariableSetObject was introduced in OptiX 1.0.  The ability to bind an @ref
  * RTprogram to a variable was intrduced in OptiX 3.0.
  *
  * <B>See also</B>
  * @ref rtVariableGetObject,
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableSetObject(RTvariable v, RTobject object);

  /**
  * @brief Defined
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableSetUserData modifies the value of a program variable whose data type is
  * user-defined. The value copied into the variable is defined by an arbitrary region of
  * memory, pointed to by \a ptr. The size of the memory region is given by \a size. The
  * target variable is specified by \a v.  If \a v is not a valid variable,
  * this call has no effect and returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   v          Specifies the program variable to be modified
  * @param[in]   size       Specifies the size of the new value, in bytes
  * @param[in]   ptr        Specifies a pointer to the new value of the program variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtVariableSetUserData was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtVariableGetUserData,
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableSetUserData(RTvariable v, RTsize size, const void* ptr);


  /* Gets */
  /**
  * @ingroup rtVariableGet
  *
  * @brief Functions designed to modify the value of a program variable
  *
  * <B>Description</B>
  *
  * @ref rtVariableGet functions return the value of a program variable or variable
  * array. The target variable is specificed by \a v.
  *
  * The commands \a rtVariableGet{1-2-3-4}{f-i-ui}v are used to query the value
  * of a program variable specified by \a v using the pointers passed as arguments
  * as return locations for each component of the vector-typed variable. The number
  * specified in the command should match the number of components in the data type
  * of the specified program variable (e.g., 1 for float, int, unsigned int; 2 for
  * float2, int2, uint2, etc.). The suffix \a f indicates that floating-point
  * values are expected to be returned, the suffix \a i indicates that integer
  * values are expected, and the suffix \a ui indicates that unsigned integer
  * values are expected, and this type should also match the data type of the
  * specified program variable. The \a f variants of this function should be used
  * to query values for program variables defined as float, float2, float3, float4,
  * or arrays of these. The \a i variants of this function should be used to
  * query values for program variables defined as int, int2, int3, int4, or
  * arrays of these. The \a ui variants of this function should be used to query
  * values for program variables defined as unsigned int, uint2, uint3, uint4,
  * or arrays of these. The \a v variants of this function should be used to
  * return the program variable's value to the array specified by parameter
  * \a v. In this case, the array \a v should be large enough to accommodate all
  * of the program variable's components.
  *
  * The commands \a rtVariableGetMatrix{2-3-4}x{2-3-4}fv are used to query the
  * value of a program variable whose data type is a matrix. The numbers in the
  * command names are interpreted as the dimensionality of the matrix. For example,
  * \a 2x4 indicates a 2 x 4 matrix with 2 columns and 4 rows (i.e., 8
  * values). If \a transpose is \a 0, the matrix is returned in row major order,
  * otherwise in column major order.
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtVariableGet were introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtVariableSet,
  * @ref rtVariableGetType,
  * @ref rtContextDeclareVariable
  *
  * @{
  */
  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f1         Float value to be returned
  */
  RTresult RTAPI rtVariableGet1f(RTvariable v, float* f1);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f1         Float value to be returned
  * @param[in]   f2         Float value to be returned
  */
  RTresult RTAPI rtVariableGet2f(RTvariable v, float* f1, float* f2);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f1         Float value to be returned
  * @param[in]   f2         Float value to be returned
  * @param[in]   f3         Float value to be returned
  */
  RTresult RTAPI rtVariableGet3f(RTvariable v, float* f1, float* f2, float* f3);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f1         Float value to be returned
  * @param[in]   f2         Float value to be returned
  * @param[in]   f3         Float value to be returned
  * @param[in]   f4         Float value to be returned
  */
  RTresult RTAPI rtVariableGet4f(RTvariable v, float* f1, float* f2, float* f3, float* f4);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f          Array of float value(s) to be returned
  */
  RTresult RTAPI rtVariableGet1fv(RTvariable v, float* f);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f          Array of float value(s) to be returned
  */
  RTresult RTAPI rtVariableGet2fv(RTvariable v, float* f);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f          Array of float value(s) to be returned
  */
  RTresult RTAPI rtVariableGet3fv(RTvariable v, float* f);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   f          Array of float value(s) to be returned
  */
  RTresult RTAPI rtVariableGet4fv(RTvariable v, float* f);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i1         Integer value to be returned
  */
  RTresult RTAPI rtVariableGet1i(RTvariable v, int* i1);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i1         Integer value to be returned
  * @param[in]   i2         Integer value to be returned
  */
  RTresult RTAPI rtVariableGet2i(RTvariable v, int* i1, int* i2);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i1         Integer value to be returned
  * @param[in]   i2         Integer value to be returned
  * @param[in]   i3         Integer value to be returned
  */
  RTresult RTAPI rtVariableGet3i(RTvariable v, int* i1, int* i2, int* i3);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i1         Integer value to be returned
  * @param[in]   i2         Integer value to be returned
  * @param[in]   i3         Integer value to be returned
  * @param[in]   i4         Integer value to be returned
  */
  RTresult RTAPI rtVariableGet4i(RTvariable v, int* i1, int* i2, int* i3, int* i4);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i          Array of integer values to be returned
  */
  RTresult RTAPI rtVariableGet1iv(RTvariable v, int* i);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i          Array of integer values to be returned
  */
  RTresult RTAPI rtVariableGet2iv(RTvariable v, int* i);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i          Array of integer values to be returned
  */
  RTresult RTAPI rtVariableGet3iv(RTvariable v, int* i);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   i          Array of integer values to be returned
  */
  RTresult RTAPI rtVariableGet4iv(RTvariable v, int* i);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u1         Unsigned integer value to be returned
  */
  RTresult RTAPI rtVariableGet1ui(RTvariable v, unsigned int* u1);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u1         Unsigned integer value to be returned
  * @param[in]   u2         Unsigned integer value to be returned
  */
  RTresult RTAPI rtVariableGet2ui(RTvariable v, unsigned int* u1, unsigned int* u2);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u1         Unsigned integer value to be returned
  * @param[in]   u2         Unsigned integer value to be returned
  * @param[in]   u3         Unsigned integer value to be returned
  */
  RTresult RTAPI rtVariableGet3ui(RTvariable v, unsigned int* u1, unsigned int* u2, unsigned int* u3);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u1         Unsigned integer value to be returned
  * @param[in]   u2         Unsigned integer value to be returned
  * @param[in]   u3         Unsigned integer value to be returned
  * @param[in]   u4         Unsigned integer value to be returned
  */
  RTresult RTAPI rtVariableGet4ui(RTvariable v, unsigned int* u1, unsigned int* u2, unsigned int* u3, unsigned int* u4);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u          Array of unsigned integer values to be returned
  */
  RTresult RTAPI rtVariableGet1uiv(RTvariable v, unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u          Array of unsigned integer values to be returned
  */
  RTresult RTAPI rtVariableGet2uiv(RTvariable v, unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u          Array of unsigned integer values to be returned
  */
  RTresult RTAPI rtVariableGet3uiv(RTvariable v, unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   u          Array of unsigned integer values to be returned
  */
  RTresult RTAPI rtVariableGet4uiv(RTvariable v, unsigned int* u);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix2x2fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix2x3fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix2x4fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix3x2fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix3x3fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix3x4fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix4x2fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix4x3fv(RTvariable v, int transpose, float* m);

  /**
  * @param[in]   v          Specifies the program variable whose value is to be returned
  * @param[in]   transpose  Specify(ies) row-major or column-major order
  * @param[in]   m          Array of float values to be returned
  */
  RTresult RTAPI rtVariableGetMatrix4x4fv(RTvariable v, int transpose, float* m);
  /**
  * @}
  */

  /**
  * @brief Returns the value of a OptiX object program variable
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableGetObject queries the value of a program variable whose data type is a
  * OptiX object.  The target variable is specified by \a v. The value of the
  * program variable is returned in \a *object. The concrete
  * type of the program variable can be queried using @ref rtVariableGetType, and the @ref
  * RTobject handle returned by @ref rtVariableGetObject may safely be cast to an OptiX
  * handle of corresponding type. If \a v is not a valid variable, this call sets
  * \a *object to \a NULL and returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   v          Specifies the program variable to be queried
  * @param[out]  object     Returns the value of the program variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtVariableGetObject was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtVariableSetObject,
  * @ref rtVariableGetType,
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableGetObject(RTvariable v, RTobject* object);

  /**
  * @brief Defined
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableGetUserData queries the value of a program variable whose data type is
  * user-defined. The variable of interest is specified by \a v.  The size of the
  * variable's value must match the value given by the parameter \a size.  The value of
  * the program variable is copied to the memory region pointed to by \a ptr. The storage
  * at location \a ptr must be large enough to accommodate all of the program variable's
  * value data. If \a v is not a valid variable, this call has no effect and
  * returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   v          Specifies the program variable to be queried
  * @param[in]   size       Specifies the size of the program variable, in bytes
  * @param[out]  ptr        Location in which to store the value of the variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtVariableGetUserData was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtVariableSetUserData,
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableGetUserData(RTvariable v, RTsize size, void* ptr);


  /* Other */
  /**
  * @brief Queries the name of a program variable
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * Queries a program variable's name. The variable of interest is specified by \a
  * variable, which should be a value returned by @ref rtContextDeclareVariable. A pointer
  * to the string containing the name of the variable is returned in \a *name_return.
  * If \a v is not a valid variable, this
  * call sets \a *name_return to \a NULL and returns @ref RT_ERROR_INVALID_VALUE.  \a
  * *name_return will point to valid memory until another API function that returns a
  * string is called.
  *
  * @param[in]   v             Specifies the program variable to be queried
  * @param[out]  name_return   Returns the program variable's name
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtVariableGetName was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableGetName(RTvariable v, const char** name_return);

  /**
  * @brief Queries the annotation string of a program variable
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableGetAnnotation queries a program variable's annotation string. A pointer
  * to the string containing the annotation is returned in \a *annotation_return.
  * If \a v is not a valid variable, this call sets
  * \a *annotation_return to \a NULL and returns @ref RT_ERROR_INVALID_VALUE.  \a
  * *annotation_return will point to valid memory until another API function that returns
  * a string is called.
  *
  * @param[in]   v                   Specifies the program variable to be queried
  * @param[out]  annotation_return   Returns the program variable's annotation string
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtVariableGetAnnotation was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtDeclareVariable,
  * @ref rtDeclareAnnotation
  *
  */
  RTresult RTAPI rtVariableGetAnnotation(RTvariable v, const char** annotation_return);

  /**
  * @brief Returns type information about a program variable
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableGetType queries a program variable's type. The variable of interest is
  * specified by \a v. The program variable's type enumeration is returned in \a *type_return,
  * if it is not \a NULL. It is one of the following:
  *
  *   - @ref RT_OBJECTTYPE_UNKNOWN
  *   - @ref RT_OBJECTTYPE_GROUP
  *   - @ref RT_OBJECTTYPE_GEOMETRY_GROUP
  *   - @ref RT_OBJECTTYPE_TRANSFORM
  *   - @ref RT_OBJECTTYPE_SELECTOR
  *   - @ref RT_OBJECTTYPE_GEOMETRY_INSTANCE
  *   - @ref RT_OBJECTTYPE_BUFFER
  *   - @ref RT_OBJECTTYPE_TEXTURE_SAMPLER
  *   - @ref RT_OBJECTTYPE_OBJECT
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT2x2
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT2x3
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT2x4
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT3x2
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT3x3
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT3x4
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT4x2
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT4x3
  *   - @ref RT_OBJECTTYPE_MATRIX_FLOAT4x4
  *   - @ref RT_OBJECTTYPE_FLOAT
  *   - @ref RT_OBJECTTYPE_FLOAT2
  *   - @ref RT_OBJECTTYPE_FLOAT3
  *   - @ref RT_OBJECTTYPE_FLOAT4
  *   - @ref RT_OBJECTTYPE_INT
  *   - @ref RT_OBJECTTYPE_INT2
  *   - @ref RT_OBJECTTYPE_INT3
  *   - @ref RT_OBJECTTYPE_INT4
  *   - @ref RT_OBJECTTYPE_UNSIGNED_INT
  *   - @ref RT_OBJECTTYPE_UNSIGNED_INT2
  *   - @ref RT_OBJECTTYPE_UNSIGNED_INT3
  *   - @ref RT_OBJECTTYPE_UNSIGNED_INT4
  *   - @ref RT_OBJECTTYPE_USER
  *
  * Sets \a *type_return to @ref RT_OBJECTTYPE_UNKNOWN if \a v is not a valid variable.
  * Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  *
  * @param[in]   v             Specifies the program variable to be queried
  * @param[out]  type_return   Returns the type of the program variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtVariableGetType was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableGetType(RTvariable v, RTobjecttype* type_return);

  /**
  * @brief Returns the context associated with a program variable
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableGetContext queries the context associated with a program variable.  The
  * target variable is specified by \a v. The context of the program variable is
  * returned to \a *context if the pointer \a context is not \a NULL. If \a v is
  * not a valid variable, \a *context is set to \a NULL and @ref RT_ERROR_INVALID_VALUE is
  * returned.
  *
  * @param[in]   v          Specifies the program variable to be queried
  * @param[out]  context    Returns the context associated with the program variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtVariableGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableGetContext(RTvariable v, RTcontext* context);

  /**
  * @brief Queries the size, in bytes, of a variable
  *
  * @ingroup Variables
  *
  * <B>Description</B>
  *
  * @ref rtVariableGetSize queries a declared program variable for its size in bytes.
  * This is most often used to query the size of a variable that has a user-defined type.
  * Builtin types (int, float, unsigned int, etc.) may be queried, but object typed
  * variables, such as buffers, texture samplers and graph nodes, cannot be queried and
  * will return @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   v          Specifies the program variable to be queried
  * @param[out]  size       Specifies a pointer where the size of the variable, in bytes, will be returned
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtVariableGetSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtVariableGetUserData,
  * @ref rtContextDeclareVariable
  *
  */
  RTresult RTAPI rtVariableGetSize(RTvariable v, RTsize* size);


/************************************
 **
 **    Context object
 **
 ***********************************/

  /**
  * @brief Creates a new context object
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextCreate allocates and returns a handle to a new context object.
  * Returns @ref RT_ERROR_INVALID_VALUE if passed a \a NULL pointer.
  *
  * @param[out]  context   Handle to context for return value
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_NO_DEVICE
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  *
  *
  */
  RTresult RTAPI rtContextCreate(RTcontext* context);

  /**
  * @brief Destroys a context and frees all associated resources
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextDestroy frees all resources, including OptiX objects, associated with
  * this object.  Returns @ref RT_ERROR_INVALID_VALUE if passed a \a NULL context.  @ref
  * RT_ERROR_LAUNCH_FAILED may be returned if a previous call to @ref rtContextLaunch "rtContextLaunch"
  * failed.
  *
  * @param[in]   context   Handle of the context to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_LAUNCH_FAILED
  *
  * <B>History</B>
  *
  * @ref rtContextDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtContextDestroy(RTcontext context);

  /**
  * @brief Checks the given context for valid internal state
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextValidate checks the the given context and all of its associated OptiX
  * objects for a valid state.  These checks include tests for presence of necessary
  * programs (e.g. an intersection program for a geometry node), invalid internal state
  * such as \a NULL children in graph nodes, and presence of variables required by all
  * specified programs. @ref rtContextGetErrorString can be used to retrieve a description
  * of a validation failure.
  *
  * @param[in]   context   The context to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_INVALID_SOURCE
  *
  * <B>History</B>
  *
  * @ref rtContextValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetErrorString
  *
  */
  RTresult RTAPI rtContextValidate(RTcontext context);

  /**
  * @brief Returns the error string associated with a given
  * error
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetErrorString return a descriptive string given an error code.  If \a
  * context is valid and additional information is available from the last OptiX failure,
  * it will be appended to the generic error code description.  \a return_string will be
  * set to point to this string.  The memory \a return_string points to will be valid
  * until the next API call that returns a string.
  *
  * @param[in]   context         The context object to be queried, or \a NULL
  * @param[in]   code            The error code to be converted to string
  * @param[out]  return_string   The return parameter for the error string
  *
  * <B>Return values</B>
  *
  * @ref rtContextGetErrorString does not return a value
  *
  * <B>History</B>
  *
  * @ref rtContextGetErrorString was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  *
  *
  */
  void RTAPI rtContextGetErrorString(RTcontext context, RTresult code, const char** return_string);

  /**
  * @brief Set an attribute specific to an OptiX context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetAttribute sets \a p as the value of the per context attribute
  * specified by \a attrib.
  *
  * Each attribute can have a different size.  The sizes are given in the following list:
  *
  *   - @ref RT_CONTEXT_ATTRIBUTE_CPU_NUM_THREADS          sizeof(int)
  *
  * @ref RT_CONTEXT_ATTRIBUTE_CPU_NUM_THREADS sets the number of host CPU threads OptiX
  * can use for various tasks.
  *
  * @param[in]   context   The context object to be modified
  * @param[in]   attrib    Attribute to set
  * @param[in]   size      Size of the attribute being set
  * @param[in]   p         Pointer to where the value of the attribute will be copied from.  This must point to at least \a size bytes of memory
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE - Can be returned if \a size does not match the proper size of the attribute, or if \a p
  * is \a NULL
  *
  * <B>History</B>
  *
  * @ref rtContextSetAttribute was introduced in OptiX 2.5.
  *
  * <B>See also</B>
  * @ref rtContextGetAttribute
  *
  */
  RTresult RTAPI rtContextSetAttribute(RTcontext context, RTcontextattribute attrib, RTsize size, void* p);

  /**
  * @brief Returns an attribute specific to an OptiX context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetAttribute returns in \a p the value of the per context attribute
  * specified by \a attrib.
  *
  * Each attribute can have a different size.  The sizes are given in the following list:
  *
  *   - @ref RT_CONTEXT_ATTRIBUTE_MAX_TEXTURE_COUNT        sizeof(int)
  *   - @ref RT_CONTEXT_ATTRIBUTE_CPU_NUM_THREADS          sizeof(int)
  *   - @ref RT_CONTEXT_ATTRIBUTE_USED_HOST_MEMORY         sizeof(RTsize)
  *   - @ref RT_CONTEXT_ATTRIBUTE_AVAILABLE_DEVICE_MEMORY  sizeof(RTsize)
  *
  * @ref RT_CONTEXT_ATTRIBUTE_MAX_TEXTURE_COUNT queries the maximum number of textures
  * handled by OptiX. For OptiX versions below 2.5 this value depends on the number of
  * textures supported by CUDA.
  *
  * @ref RT_CONTEXT_ATTRIBUTE_CPU_NUM_THREADS queries the number of host CPU threads OptiX
  * can use for various tasks.
  *
  * @ref RT_CONTEXT_ATTRIBUTE_USED_HOST_MEMORY queries the amount of host memory allocated
  * by OptiX.
  *
  * @ref RT_CONTEXT_ATTRIBUTE_AVAILABLE_DEVICE_MEMORY queries the amount of free device
  * memory.
  *
  * Some attributes are used to get per device information.  In contrast to @ref
  * rtDeviceGetAttribute, these attributes are determined by the context and are therefore
  * queried through the context.  This is done by adding the attribute with the OptiX
  * device ordinal number when querying the attribute.  The following are per device attributes.
  *
  *   @ref RT_CONTEXT_ATTRIBUTE_AVAILABLE_DEVICE_MEMORY
  *
  * @param[in]   context   The context object to be queried
  * @param[in]   attrib    Attribute to query
  * @param[in]   size      Size of the attribute being queried.  Parameter \a p must have at least this much memory allocated
  * @param[out]  p         Return pointer where the value of the attribute will be copied into.  This must point to at least \a size bytes of memory
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE - Can be returned if \a size does not match the proper size of the attribute, if \a p is
  * \a NULL, or if \a attribute+ordinal does not correspond to an OptiX device
  *
  * <B>History</B>
  *
  * @ref rtContextGetAttribute was introduced in OptiX 2.0.
  *
  * <B>See also</B>
  * @ref rtContextGetDeviceCount,
  * @ref rtContextSetAttribute,
  * @ref rtDeviceGetAttribute
  *
  */
  RTresult RTAPI rtContextGetAttribute(RTcontext context, RTcontextattribute attrib, RTsize size, void* p);

  /**
  * @brief Specify a list of hardware devices to be used by the
  * kernel
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetDevices specifies a list of hardware devices to be used during
  * execution of the subsequent trace kernels. Note that the device numbers are 
  * OptiX device ordinals, which may not be the same as CUDA device ordinals. 
  * Use @ref rtDeviceGetAttribute with @ref RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL to query the CUDA device
  * corresponding to a particular OptiX device.
  *
  * @param[in]   context   The context to which the hardware list is applied
  * @param[in]   count     The number of devices in the list
  * @param[in]   devices   The list of devices
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_NO_DEVICE
  * - @ref RT_ERROR_INVALID_DEVICE
  *
  * <B>History</B>
  *
  * @ref rtContextSetDevices was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetDevices,
  * @ref rtContextGetDeviceCount
  *
  */
  RTresult RTAPI rtContextSetDevices(RTcontext context, unsigned int count, const int* devices);

  /**
  * @brief Retrieve a list of hardware devices being used by the
  * kernel
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetDevices retrieves a list of hardware devices used by the context. 
  * Note that the device numbers are  OptiX device ordinals, which may not be the same as CUDA device ordinals. 
  * Use @ref rtDeviceGetAttribute with @ref RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL to query the CUDA device
  * corresponding to a particular OptiX device.
  *
  * @param[in]   context   The context to which the hardware list is applied
  * @param[out]  devices   Return parameter for the list of devices.  The memory must be able to hold entries
  * numbering least the number of devices as returned by @ref rtContextGetDeviceCount
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetDevices was introduced in OptiX 2.0.
  *
  * <B>See also</B>
  * @ref rtContextSetDevices,
  * @ref rtContextGetDeviceCount
  *
  */
  RTresult RTAPI rtContextGetDevices(RTcontext context, int* devices);

  /**
  * @brief Query the number of devices currently being used
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetDeviceCount - Query the number of devices currently being used.
  *
  * @param[in]   context   The context containing the devices
  * @param[out]  count     Return parameter for the device count
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetDeviceCount was introduced in OptiX 2.0.
  *
  * <B>See also</B>
  * @ref rtContextSetDevices,
  * @ref rtContextGetDevices
  *
  */
  RTresult RTAPI rtContextGetDeviceCount(RTcontext context, unsigned int* count);

  /**
  * @brief Enable rendering on a remote device
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * Associates a context with a remote device. If successful, any further OptiX calls will be
  * directed to the remote device and executed there. The context must be an empty, newly created
  * context. In other words, in order to use a context remotely, the call to @ref rtContextSetRemoteDevice
  * should immediately follow the call to @ref rtContextCreate.
  *
  * Note that a context that was used for remote rendering cannot be re-used for local rendering by
  * changing devices. However, the Progressive API (that is, @ref rtContextLaunchProgressive2D,
  * stream buffers, etc.) can be used locally by simply not creating a remote device and not calling
  * @ref rtContextSetRemoteDevice.
  *
  * Only a single remote device can be associated with a context. Switching between
  * different remote devices is not supported.
  *
  * @param[in]   context        Newly created context to use on the remote device
  * @param[in]   remote_dev     Remote device on which rendering is to be executed
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetRemoteDevice was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtRemoteDeviceCreate
  * @ref rtRemoteDeviceGetAttribute
  * @ref rtRemoteDeviceReserve
  * @ref rtContextLaunchProgressive2D
  *
  */
  RTresult RTAPI rtContextSetRemoteDevice(RTcontext context, RTremotedevice remote_dev);

  /**
  * @brief Set the stack size for a given context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetStackSize sets the stack size for the given context to
  * \a stack_size_bytes bytes. Returns @ref RT_ERROR_INVALID_VALUE if context is not valid.
  *
  * @param[in]   context            The context node to be modified
  * @param[in]   stack_size_bytes   The desired stack size in bytes
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetStackSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetStackSize
  *
  */
  RTresult RTAPI rtContextSetStackSize(RTcontext context, RTsize stack_size_bytes);

  /**
  * @brief Query the stack size for this context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetStackSize passes back the stack size associated with this context in
  * \a stack_size_bytes.  Returns @ref RT_ERROR_INVALID_VALUE if passed a \a NULL pointer.
  *
  * @param[in]   context            The context node to be queried
  * @param[out]  stack_size_bytes   Return parameter to store the size of the stack
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetStackSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextSetStackSize
  *
  */
  RTresult RTAPI rtContextGetStackSize(RTcontext context, RTsize* stack_size_bytes);


  /**
  * @brief Side timeout callback function
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetTimeoutCallback sets an application-side callback function
  * \a callback and a time interval \a min_polling_seconds in
  * seconds. Potentially long-running OptiX API calls such as 
  * @ref rtContextLaunch call the callback function about every
  * \a min_polling_seconds seconds. The core purpose of a timeout callback
  * function is to give the application a chance to do whatever it might need
  * to do frequently, such as handling GUI events.
  *
  * If the callback function returns true,
  * the API call tries to abort, leaving the context in a clean but
  * unfinished state. Output buffers are left in an unpredictable state.
  * In case an OptiX API call is terminated by a callback function, it
  * returns @ref RT_TIMEOUT_CALLBACK.
  *
  * As a side effect, timeout functions also help control the OptiX
  * kernel run-time. This can in some cases prevent OptiX kernel
  * launches from running so long that they cause driver timeouts. For
  * example, if \a min_polling_seconds is 0.5 seconds then once the
  * kernel has been running for 0.5 seconds it won't start any new
  * launch indices (calls to a ray generation program). Thus, if the
  * driver's timeout is 2 seconds (the default on Windows), then a
  * launch index may take up to 1.5 seconds without triggering a
  * driver timeout.
  *
  * @ref RTtimeoutcallback is defined as \a int (*RTtimeoutcallback)(void).
  *
  * To unregister a callback function, \a callback needs to be set to
  * \a NULL and \a min_polling_seconds to 0.
  *
  * Only one timeout callback function can be specified at any time.
  *
  * Returns @ref RT_ERROR_INVALID_VALUE if \a context is not valid, if
  * \a min_polling_seconds is negative, if \a callback is \a NULL but
  * \a min_polling_seconds is not 0, or if \a callback is not \a NULL but
  * \a min_polling_seconds is 0.
  *
  * @param[in]   context               The context node to be modified
  * @param[in]   callback              The function to be called
  * @param[in]   min_polling_seconds   The timeout interval after which the function is called
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetTimeoutCallback was introduced in OptiX 2.5.
  *
  * <B>See also</B>
  * @ref rtContextLaunch
  *
  */
  RTresult RTAPI rtContextSetTimeoutCallback(RTcontext context, RTtimeoutcallback callback, double min_polling_seconds);

  /**
  * @brief Set the number of entry points for a given context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetEntryPointCount sets the number of entry points associated with
  * the given context to \a num_entry_points.
  *
  * @param[in]   context            The context to be modified
  * @param[in]   num_entry_points   The number of entry points to use
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetEntryPointCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetEntryPointCount
  *
  */
  RTresult RTAPI rtContextSetEntryPointCount(RTcontext context, unsigned int num_entry_points);

  /**
  * @brief Query the number of entry points for this
  * context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetEntryPointCount passes back the number of entry points associated
  * with this context in \a num_entry_points.  Returns @ref RT_ERROR_INVALID_VALUE if
  * passed a \a NULL pointer.
  *
  * @param[in]   context            The context node to be queried
  * @param[out]  num_entry_points   Return parameter for passing back the entry point count
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetEntryPointCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextSetEntryPointCount
  *
  */
  RTresult RTAPI rtContextGetEntryPointCount(RTcontext context, unsigned int* num_entry_points);

  /**
  * @brief Specifies the ray generation program for
  * a given context entry point
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetRayGenerationProgram sets \a context's ray generation program at
  * entry point \a entry_point_index. @ref RT_ERROR_INVALID_VALUE is returned if \a
  * entry_point_index is outside of the range [\a 0, @ref rtContextGetEntryPointCount
  * \a -1].
  *
  * @param[in]   context             The context node to which the exception program will be added
  * @param[in]   entry_point_index   The entry point the program will be associated with
  * @param[in]   program             The ray generation program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtContextSetRayGenerationProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetEntryPointCount,
  * @ref rtContextGetRayGenerationProgram
  *
  */
  RTresult RTAPI rtContextSetRayGenerationProgram(RTcontext context, unsigned int entry_point_index, RTprogram program);

  /**
  * @brief Queries the ray generation program
  * associated with the given context and entry point
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetRayGenerationProgram passes back the ray generation program
  * associated with the given context and entry point.  This program is set via @ref
  * rtContextSetRayGenerationProgram.  Returns @ref RT_ERROR_INVALID_VALUE if given an
  * invalid entry point index or \a NULL pointer.
  *
  * @param[in]   context             The context node associated with the ray generation program
  * @param[in]   entry_point_index   The entry point index for the desired ray generation program
  * @param[out]  program             Return parameter to store the ray generation program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetRayGenerationProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextSetRayGenerationProgram
  *
  */
  RTresult RTAPI rtContextGetRayGenerationProgram(RTcontext context, unsigned int entry_point_index, RTprogram* program);

  /**
  * @brief Specifies the exception program for a given context entry point
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetExceptionProgram sets \a context's exception program at entry point
  * \a entry_point_index. @ref RT_ERROR_INVALID_VALUE is returned if \a entry_point_index
  * is outside of the range [\a 0, @ref rtContextGetEntryPointCount \a -1].
  *
  * @param[in]   context             The context node to which the exception program will be added
  * @param[in]   entry_point_index   The entry point the program will be associated with
  * @param[in]   program             The exception program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtContextSetExceptionProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetEntryPointCount,
  * @ref rtContextGetExceptionProgram
  * @ref rtContextSetExceptionEnabled,
  * @ref rtContextGetExceptionEnabled,
  * @ref rtGetExceptionCode,
  * @ref rtThrow,
  * @ref rtPrintExceptionDetails
  *
  */
  RTresult RTAPI rtContextSetExceptionProgram(RTcontext context, unsigned int entry_point_index, RTprogram program);

  /**
  * @brief Queries the exception program associated with
  * the given context and entry point
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetExceptionProgram passes back the exception program associated with
  * the given context and entry point.  This program is set via @ref
  * rtContextSetExceptionProgram.  Returns @ref RT_ERROR_INVALID_VALUE if given an invalid
  * entry point index or \a NULL pointer.
  *
  * @param[in]   context             The context node associated with the exception program
  * @param[in]   entry_point_index   The entry point index for the desired exception program
  * @param[out]  program             Return parameter to store the exception program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetExceptionProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextSetExceptionProgram,
  * @ref rtContextSetEntryPointCount,
  * @ref rtContextSetExceptionEnabled,
  * @ref rtContextGetExceptionEnabled,
  * @ref rtGetExceptionCode,
  * @ref rtThrow,
  * @ref rtPrintExceptionDetails
  *
  */
  RTresult RTAPI rtContextGetExceptionProgram(RTcontext context, unsigned int entry_point_index, RTprogram* program);

  /**
  * @brief Enable or disable an exception
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetExceptionEnabled is used to enable or disable specific exceptions.
  * If an exception is enabled, the exception condition is checked for at runtime, and the
  * exception program is invoked if the condition is met. The exception program can query
  * the type of the caught exception by calling @ref rtGetExceptionCode.
  * \a exception may take one of the following values:
  *
  *   - @ref RT_EXCEPTION_TEXTURE_ID_INVALID
  *   - @ref RT_EXCEPTION_BUFFER_ID_INVALID
  *   - @ref RT_EXCEPTION_INDEX_OUT_OF_BOUNDS
  *   - @ref RT_EXCEPTION_STACK_OVERFLOW
  *   - @ref RT_EXCEPTION_BUFFER_INDEX_OUT_OF_BOUNDS
  *   - @ref RT_EXCEPTION_INVALID_RAY
  *   - @ref RT_EXCEPTION_INTERNAL_ERROR
  *   - @ref RT_EXCEPTION_USER
  *   - @ref RT_EXCEPTION_ALL
  *
  *
  * @ref RT_EXCEPTION_TEXTURE_ID_INVALID verifies that every access of a texture id is
  * valid, including use of RT_TEXTURE_ID_NULL and IDs out of bounds.
  *
  * @ref RT_EXCEPTION_BUFFER_ID_INVALID verifies that every access of a buffer id is
  * valid, including use of RT_BUFFER_ID_NULL and IDs out of bounds.
  *
  * @ref RT_EXCEPTION_INDEX_OUT_OF_BOUNDS checks that @ref rtIntersectChild and @ref
  * rtReportIntersection are called with a valid index.
  *
  * @ref RT_EXCEPTION_STACK_OVERFLOW checks the runtime stack against overflow. The most
  * common cause for an overflow is a too deep @ref rtTrace recursion tree.
  *
  * @ref RT_EXCEPTION_BUFFER_INDEX_OUT_OF_BOUNDS checks every read and write access to
  * @ref rtBuffer objects to be within valid bounds.
  *
  * @ref RT_EXCEPTION_INVALID_RAY checks the each ray's origin and direction values
  * against \a NaNs and \a infinity values.
  *
  * @ref RT_EXCEPTION_INTERNAL_ERROR indicates an unexpected internal error in the
  * runtime.
  *
  * @ref RT_EXCEPTION_USER is used to enable or disable all user-defined exceptions. The
  * reserved range of exception codes for user-defined exceptions starts at @ref
  * RT_EXCEPTION_USER (\a 0x400) and ends at \a 0xFFFF. See @ref rtThrow for more
  * information.
  *
  * @ref RT_EXCEPTION_ALL is a placeholder value which can be used to enable or disable
  * all possible exceptions with a single call to @ref rtContextSetExceptionEnabled.
  *
  * By default, @ref RT_EXCEPTION_STACK_OVERFLOW is enabled and all other exceptions are
  * disabled.
  *
  * @param[in]   context     The context for which the exception is to be enabled or disabled
  * @param[in]   exception   The exception which is to be enabled or disabled
  * @param[in]   enabled     Nonzero to enable the exception, \a 0 to disable the exception
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetExceptionEnabled was introduced in OptiX 1.1.
  *
  * <B>See also</B>
  * @ref rtContextGetExceptionEnabled,
  * @ref rtContextSetExceptionProgram,
  * @ref rtContextGetExceptionProgram,
  * @ref rtGetExceptionCode,
  * @ref rtThrow,
  * @ref rtPrintExceptionDetails
  *
  */
  RTresult RTAPI rtContextSetExceptionEnabled(RTcontext context, RTexception exception, int enabled);

  /**
  * @brief Query whether a specified exception is enabled
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetExceptionEnabled passes back \a 1 in \a *enabled if the given exception is
  * enabled, \a 0 otherwise. \a exception specifies the type of exception to be queried. For a list
  * of available types, see @ref rtContextSetExceptionEnabled. If \a exception
  * is @ref RT_EXCEPTION_ALL, \a enabled is set to \a 1 only if all possible
  * exceptions are enabled.
  *
  * @param[in]   context     The context to be queried
  * @param[in]   exception   The exception of which to query the state
  * @param[out]  enabled     Return parameter to store whether the exception is enabled
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetExceptionEnabled was introduced in OptiX 1.1.
  *
  * <B>See also</B>
  * @ref rtContextSetExceptionEnabled,
  * @ref rtContextSetExceptionProgram,
  * @ref rtContextGetExceptionProgram,
  * @ref rtGetExceptionCode,
  * @ref rtThrow,
  * @ref rtPrintExceptionDetails
  *
  */
  RTresult RTAPI rtContextGetExceptionEnabled(RTcontext context, RTexception exception, int* enabled);

  /**
  * @brief Sets the number of ray types for a given context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetRayTypeCount Sets the number of ray types associated with the given
  * context.
  *
  * @param[in]   context         The context node
  * @param[in]   num_ray_types   The number of ray types to be used
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetRayTypeCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetRayTypeCount
  *
  */
  RTresult RTAPI rtContextSetRayTypeCount(RTcontext context, unsigned int num_ray_types);

  /**
  * @brief Query the number of ray types associated with this
  * context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetRayTypeCount passes back the number of entry points associated with
  * this context in \a num_ray_types.  Returns @ref RT_ERROR_INVALID_VALUE if passed a \a
  * NULL pointer.
  *
  * @param[in]   context         The context node to be queried
  * @param[out]  num_ray_types   Return parameter to store the number of ray types
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetRayTypeCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextSetRayTypeCount
  *
  */
  RTresult RTAPI rtContextGetRayTypeCount(RTcontext context, unsigned int* num_ray_types);

  /**
  * @brief Specifies the miss program for a given context ray type
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetMissProgram sets \a context's miss program associated with ray type
  * \a ray_type_index. @ref RT_ERROR_INVALID_VALUE is returned if \a ray_type_index
  * is outside of the range [\a 0, @ref rtContextGetRayTypeCount \a -1].
  *
  * @param[in]   context          The context node to which the miss program will be added
  * @param[in]   ray_type_index   The ray type the program will be associated with
  * @param[in]   program          The miss program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtContextSetMissProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetRayTypeCount,
  * @ref rtContextGetMissProgram
  *
  */
  RTresult RTAPI rtContextSetMissProgram(RTcontext context, unsigned int ray_type_index, RTprogram program);

  /**
  * @brief Queries the miss program associated with the given
  * context and ray type
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetMissProgram passes back the miss program associated with the
  * given context and ray type.  This program is set via @ref rtContextSetMissProgram.
  * Returns @ref RT_ERROR_INVALID_VALUE if given an invalid ray type index or a \a NULL pointer.
  *
  * @param[in]   context          The context node associated with the miss program
  * @param[in]   ray_type_index   The ray type index for the desired miss program
  * @param[out]  program          Return parameter to store the miss program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetMissProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextSetMissProgram,
  * @ref rtContextGetRayTypeCount
  *
  */
  RTresult RTAPI rtContextGetMissProgram(RTcontext context, unsigned int ray_type_index, RTprogram* program);

  /**
  * @brief Gets an RTtexturesampler corresponding to the texture id
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetId returns a handle to the texture sampler in \a *sampler
  * corresponding to the \a sampler_id supplied.  If \a sampler_id does not map to a valid
  * texture handle, \a *sampler is \a NULL or if \a context is invalid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   context     The context the sampler should be originated from
  * @param[in]   sampler_id  The ID of the sampler to query
  * @param[out]  sampler     The return handle for the sampler object corresponding to the sampler_id
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetTextureSamplerFromId was introduced in OptiX 3.5.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetId
  *
  */
  RTresult RTAPI rtContextGetTextureSamplerFromId(RTcontext context, int sampler_id, RTtexturesampler* sampler);

  /**
  * Deprecated in OptiX 4.0. Calling this function has no effect. The kernel is automatically compiled at launch if needed.
  *
  */
  RTresult RTAPI rtContextCompile(RTcontext context);

  /**
  * @brief Executes the computation kernel for a given context
  *
  * @ingroup rtContextLaunch
  *
  * <B>Description</B>
  *
  * @ref rtContextLaunch "rtContextLaunch" functions execute the computation kernel associated with the
  * given context.  If the context has not yet been compiled, or if the context has been
  * modified since the last compile, @ref rtContextLaunch "rtContextLaunch" will recompile the kernel
  * internally.  Acceleration structures of the context which are marked dirty will be
  * updated and their dirty flags will be cleared.  Similarly, validation will occur if
  * necessary.  The ray generation program specified by \a entry_point_index will be
  * invoked once for every element (pixel or voxel) of the computation grid specified by
  * \a image_width, \a image_height, and \a image_depth.
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_INVALID_SOURCE
  * - @ref RT_ERROR_LAUNCH_FAILED
  *
  * <B>History</B>
  *
  * @ref rtContextLaunch "rtContextLaunch" was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextGetRunningState,
  * @ref rtContextValidate
  *
  */
  /**
  * @ingroup rtContextLaunch
  * @param[in]   context                                    The context to be executed
  * @param[in]   entry_point_index                          The initial entry point into kernel
  * @param[in]   image_width                                Width of the computation grid
  */
  RTresult RTAPI rtContextLaunch1D(RTcontext context, unsigned int entry_point_index, RTsize image_width);
  /**
  * @ingroup rtContextLaunch
  * @param[in]   context                                    The context to be executed
  * @param[in]   entry_point_index                          The initial entry point into kernel
  * @param[in]   image_width                                Width of the computation grid
  * @param[in]   image_height                               Height of the computation grid
  */
  RTresult RTAPI rtContextLaunch2D(RTcontext context, unsigned int entry_point_index, RTsize image_width, RTsize image_height);
  /**
  * @ingroup rtContextLaunch
  * @param[in]   context                                    The context to be executed
  * @param[in]   entry_point_index                          The initial entry point into kernel
  * @param[in]   image_width                                Width of the computation grid
  * @param[in]   image_height                               Height of the computation grid
  * @param[in]   image_depth                                Depth of the computation grid
  */
  RTresult RTAPI rtContextLaunch3D(RTcontext context, unsigned int entry_point_index, RTsize image_width, RTsize image_height, RTsize image_depth);

  /**
  * @brief Query whether the given context is currently
  * running
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * This function is currently unimplemented and it is provided as a placeholder for a future implementation.
  *
  * @param[in]   context   The context node to be queried
  * @param[out]  running   Return parameter to store the running state
  *
  * <B>Return values</B>
  *
  * Since unimplemented, this function will always throw an assertion failure.
  *
  * <B>History</B>
  *
  * @ref rtContextGetRunningState was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextLaunch1D,
  * @ref rtContextLaunch2D,
  * @ref rtContextLaunch3D
  *
  */
  RTresult RTAPI rtContextGetRunningState(RTcontext context, int* running);

  /**
  * @brief Executes a Progressive Launch for a given context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * Starts the (potentially parallel) generation of subframes for progressive rendering. If
  * \a max_subframes is zero, there is no limit on the number of subframes generated. The
  * generated subframes are automatically composited into a single result and streamed to
  * the client at regular intervals, where they can be read by mapping an associated stream
  * buffer. An application can therefore initiate a progressive launch, and then repeatedly
  * map and display the contents of the stream buffer in order to visualize the progressive
  * refinement of the image.
  *
  * The call is nonblocking. A polling approach should be used to decide when to map and
  * display the stream buffer contents (see @ref rtBufferGetProgressiveUpdateReady). If a
  * progressive launch is already in progress at the time of the call and its parameters
  * match the initial launch, the call has no effect. Otherwise, the accumulated result will be
  * reset and a new progressive launch will be started.
  *
  * If any other OptiX function is called while a progressive launch is in progress, it will
  * cause the launch to stop generating new subframes (however, subframes that have
  * already been generated and are currently in flight may still arrive at the client). The only
  * exceptions to this rule are the operations to map a stream buffer, issuing another
  * progressive launch with unchanged parameters, and polling for an update. Those
  * exceptions do not cause the progressive launch to stop generating subframes.
  *
  * There is no guarantee that the call actually produces any subframes, especially if
  * @ref rtContextLaunchProgressive2D and other OptiX commands are called in short
  * succession. For example, during an animation, @ref rtVariableSet calls may be tightly
  * interleaved with progressive launches, and when rendering remotely the server may decide to skip some of the
  * launches in order to avoid a large backlog in the command pipeline.
  *
  * @param[in]   context                The context in which the launch is to be executed
  * @param[in]   entry_index            The initial entry point into kernel
  * @param[in]   width                  Width of the computation grid
  * @param[in]   height                 Height of the computation grid
  * @param[in]   max_subframes          The maximum number of subframes to be generated. Set to zero to generate an unlimited number of subframes
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_LAUNCH_FAILED
  *
  * <B>History</B>
  *
  * @ref rtContextLaunchProgressive2D was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtContextStopProgressive
  * @ref rtBufferGetProgressiveUpdateReady
  *
  */
  RTresult RTAPI rtContextLaunchProgressive2D(RTcontext context, unsigned int entry_index, RTsize width, RTsize height, unsigned int max_subframes);

  /**
  * @brief Stops a Progressive Launch
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * If a progressive launch is currently in progress, calling @ref rtContextStopProgressive
  * terminates it. Otherwise, the call has no effect. If a launch is stopped using this function,
  * no further subframes will arrive at the client, even if they have already been generated
  * by the server and are currently in flight.
  *
  * This call should only be used if the application must guarantee that frames generated by
  * previous progressive launches won't be accessed. Do not call @ref rtContextStopProgressive in
  * the main rendering loop if the goal is only to change OptiX state (e.g. rtVariable values).
  * The call is unnecessary in that case and will degrade performance.
  *
  * @param[in]   context                The context associated with the progressive launch
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_INVALID_CONTEXT
  *
  * <B>History</B>
  *
  * @ref rtContextStopProgressive was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtContextLaunchProgressive2D
  *
  */
  RTresult RTAPI rtContextStopProgressive(RTcontext context);

  /**
  * @brief Enable or disable text printing from programs
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetPrintEnabled is used to control whether text printing in programs
  * through @ref rtPrintf is currently enabled for this context.
  *
  * @param[in]   context   The context for which printing is to be enabled or disabled
  * @param[in]   enabled   Setting this parameter to a nonzero value enables printing, \a 0 disables printing
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetPrintEnabled was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtPrintf,
  * @ref rtContextGetPrintEnabled,
  * @ref rtContextSetPrintBufferSize,
  * @ref rtContextGetPrintBufferSize,
  * @ref rtContextSetPrintLaunchIndex,
  * @ref rtContextGetPrintLaunchIndex
  *
  */
  RTresult RTAPI rtContextSetPrintEnabled(RTcontext context, int enabled);

  /**
  * @brief Query whether text printing from programs
  * is enabled
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetPrintEnabled passes back \a 1 if text printing from programs through
  * @ref rtPrintf is currently enabled for this context; \a 0 otherwise.  Returns @ref
  * RT_ERROR_INVALID_VALUE if passed a \a NULL pointer.
  *
  * @param[in]   context   The context to be queried
  * @param[out]  enabled   Return parameter to store whether printing is enabled
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetPrintEnabled was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtPrintf,
  * @ref rtContextSetPrintEnabled,
  * @ref rtContextSetPrintBufferSize,
  * @ref rtContextGetPrintBufferSize,
  * @ref rtContextSetPrintLaunchIndex,
  * @ref rtContextGetPrintLaunchIndex
  *
  */
  RTresult RTAPI rtContextGetPrintEnabled(RTcontext context, int* enabled);

  /**
  * @brief Set the size of the print buffer
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetPrintBufferSize is used to set the buffer size available to hold
  * data generated by @ref rtPrintf.
  * Returns @ref RT_ERROR_INVALID_VALUE if it is called after the first invocation of rtContextLaunch.
  *
  *
  * @param[in]   context             The context for which to set the print buffer size
  * @param[in]   buffer_size_bytes   The print buffer size in bytes
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetPrintBufferSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtPrintf,
  * @ref rtContextSetPrintEnabled,
  * @ref rtContextGetPrintEnabled,
  * @ref rtContextGetPrintBufferSize,
  * @ref rtContextSetPrintLaunchIndex,
  * @ref rtContextGetPrintLaunchIndex
  *
  */
  RTresult RTAPI rtContextSetPrintBufferSize(RTcontext context, RTsize buffer_size_bytes);

  /**
  * @brief Get the current size of the print buffer
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetPrintBufferSize is used to query the buffer size available to hold
  * data generated by @ref rtPrintf. Returns @ref RT_ERROR_INVALID_VALUE if passed a \a
  * NULL pointer.
  *
  * @param[in]   context             The context from which to query the print buffer size
  * @param[out]  buffer_size_bytes   The returned print buffer size in bytes
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetPrintBufferSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtPrintf,
  * @ref rtContextSetPrintEnabled,
  * @ref rtContextGetPrintEnabled,
  * @ref rtContextSetPrintBufferSize,
  * @ref rtContextSetPrintLaunchIndex,
  * @ref rtContextGetPrintLaunchIndex
  *
  */
  RTresult RTAPI rtContextGetPrintBufferSize(RTcontext context, RTsize* buffer_size_bytes);

  /**
  * @brief Sets the active launch index to limit text output
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextSetPrintLaunchIndex is used to control for which launch indices @ref
  * rtPrintf generates output. The initial value of (x,y,z) is (\a -1,\a -1,\a -1), which
  * generates output for all indices.
  *
  * @param[in]   context   The context for which to set the print launch index
  * @param[in]   x         The launch index in the x dimension to which to limit the output of @ref rtPrintf invocations.
  * If set to \a -1, output is generated for all launch indices in the x dimension
  * @param[in]   y         The launch index in the y dimension to which to limit the output of @ref rtPrintf invocations.
  * If set to \a -1, output is generated for all launch indices in the y dimension
  * @param[in]   z         The launch index in the z dimension to which to limit the output of @ref rtPrintf invocations.
  * If set to \a -1, output is generated for all launch indices in the z dimension
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextSetPrintLaunchIndex was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtPrintf,
  * @ref rtContextGetPrintEnabled,
  * @ref rtContextSetPrintEnabled,
  * @ref rtContextSetPrintBufferSize,
  * @ref rtContextGetPrintBufferSize,
  * @ref rtContextGetPrintLaunchIndex
  *
  */
  RTresult RTAPI rtContextSetPrintLaunchIndex(RTcontext context, int x, int y, int z);

  /**
  * @brief Gets the active print launch index
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetPrintLaunchIndex is used to query for which launch indices @ref
  * rtPrintf generates output. The initial value of (x,y,z) is (\a -1,\a -1,\a -1), which
  * generates output for all indices.
  *
  * @param[in]   context   The context from which to query the print launch index
  * @param[out]  x         Returns the launch index in the x dimension to which the output of @ref rtPrintf invocations
  * is limited. Will not be written to if a \a NULL pointer is passed
  * @param[out]  y         Returns the launch index in the y dimension to which the output of @ref rtPrintf invocations
  * is limited. Will not be written to if a \a NULL pointer is passed
  * @param[out]  z         Returns the launch index in the z dimension to which the output of @ref rtPrintf invocations
  * is limited. Will not be written to if a \a NULL pointer is passed
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetPrintLaunchIndex was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtPrintf,
  * @ref rtContextGetPrintEnabled,
  * @ref rtContextSetPrintEnabled,
  * @ref rtContextSetPrintBufferSize,
  * @ref rtContextGetPrintBufferSize,
  * @ref rtContextSetPrintLaunchIndex
  *
  */
  RTresult RTAPI rtContextGetPrintLaunchIndex(RTcontext context, int* x, int* y, int* z);

  /**
  * @brief Declares a new named variable associated with this
  * context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextDeclareVariable - Declares a new variable named \a name and associated
  * with this context.  Only a single variable of a given name can exist for a given
  * context and any attempt to create multiple variables with the same name will cause a
  * failure with a return value of @ref RT_ERROR_VARIABLE_REDECLARED.  Returns @ref
  * RT_ERROR_INVALID_VALUE if passed a \a NULL pointer.  Return @ref
  * RT_ERROR_ILLEGAL_SYMBOL if \a name is not syntactically valid.
  *
  * @param[in]   context   The context node to which the variable will be attached
  * @param[in]   name      The name that identifies the variable to be queried
  * @param[out]  v         Pointer to variable handle used to return the new object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_VARIABLE_REDECLARED
  *
  * <B>History</B>
  *
  * @ref rtContextDeclareVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryDeclareVariable,
  * @ref rtGeometryInstanceDeclareVariable,
  * @ref rtMaterialDeclareVariable,
  * @ref rtProgramDeclareVariable,
  * @ref rtSelectorDeclareVariable,
  * @ref rtContextGetVariable,
  * @ref rtContextGetVariableCount,
  * @ref rtContextQueryVariable,
  * @ref rtContextRemoveVariable
  *
  */
  RTresult RTAPI rtContextDeclareVariable(RTcontext context, const char* name, RTvariable* v);

  /**
  * @brief Returns a named variable associated with this context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextQueryVariable queries a variable identified by the string \a name
  * from \a context and stores the result in \a *v. A variable must
  * be declared with @ref rtContextDeclareVariable before it can be queried, otherwise \a *v will be set to \a NULL.
  * @ref RT_ERROR_INVALID_VALUE will be returned if \a name or \a v is \a NULL.
  *
  * @param[in]   context   The context node to query a variable from
  * @param[in]   name      The name that identifies the variable to be queried
  * @param[out]  v         Return value to store the queried variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextQueryVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryQueryVariable,
  * @ref rtGeometryInstanceQueryVariable,
  * @ref rtMaterialQueryVariable,
  * @ref rtProgramQueryVariable,
  * @ref rtSelectorQueryVariable,
  * @ref rtContextDeclareVariable,
  * @ref rtContextGetVariableCount,
  * @ref rtContextGetVariable,
  * @ref rtContextRemoveVariable
  *
  */
  RTresult RTAPI rtContextQueryVariable(RTcontext context, const char* name, RTvariable* v);

  /**
  * @brief Removes a variable from the given context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextRemoveVariable removes variable \a v from \a context if present.
  * Returns @ref RT_ERROR_VARIABLE_NOT_FOUND if the variable is not attached to this
  * context. Returns @ref RT_ERROR_INVALID_VALUE if passed an invalid variable.
  *
  * @param[in]   context   The context node from which to remove a variable
  * @param[in]   v         The variable to be removed
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtContextRemoveVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryRemoveVariable,
  * @ref rtGeometryInstanceRemoveVariable,
  * @ref rtMaterialRemoveVariable,
  * @ref rtProgramRemoveVariable,
  * @ref rtSelectorRemoveVariable,
  * @ref rtContextDeclareVariable,
  * @ref rtContextGetVariable,
  * @ref rtContextGetVariableCount,
  * @ref rtContextQueryVariable,
  *
  */
  RTresult RTAPI rtContextRemoveVariable(RTcontext context, RTvariable v);

  /**
  * @brief Returns the number of variables associated
  * with this context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetVariableCount returns the number of variables that are currently
  * attached to \a context.  Returns @ref RT_ERROR_INVALID_VALUE if passed a \a NULL pointer.
  *
  * @param[in]   context   The context to be queried for number of attached variables
  * @param[out]  count     Return parameter to store the number of variables
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetVariableCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGetVariableCount,
  * @ref rtGeometryInstanceGetVariableCount,
  * @ref rtMaterialGetVariableCount,
  * @ref rtProgramGetVariableCount,
  * @ref rtSelectorGetVariable,
  * @ref rtContextDeclareVariable,
  * @ref rtContextGetVariable,
  * @ref rtContextQueryVariable,
  * @ref rtContextRemoveVariable
  *
  */
  RTresult RTAPI rtContextGetVariableCount(RTcontext context, unsigned int* count);

  /**
  * @brief Queries an indexed variable associated with this context
  *
  * @ingroup Context
  *
  * <B>Description</B>
  *
  * @ref rtContextGetVariable queries the variable at position \a index in the
  * variable array from \a context and stores the result in the parameter \a v.
  * A variable must be declared first with @ref rtContextDeclareVariable and
  * \a index must be in the range [\a 0, @ref rtContextGetVariableCount \a -1].
  *
  * @param[in]   context   The context node to be queried for an indexed variable
  * @param[in]   index     The index that identifies the variable to be queried
  * @param[out]  v         Return value to store the queried variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGetVariable,
  * @ref rtGeometryInstanceGetVariable,
  * @ref rtMaterialGetVariable,
  * @ref rtProgramGetVariable,
  * @ref rtSelectorGetVariable,
  * @ref rtContextDeclareVariable,
  * @ref rtContextGetVariableCount,
  * @ref rtContextQueryVariable,
  * @ref rtContextRemoveVariable
  *
  */
  RTresult RTAPI rtContextGetVariable(RTcontext context, unsigned int index, RTvariable* v);

/************************************
 **
 **    Program object
 **
 ***********************************/

  /**
  * @brief Creates a new program object
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramCreateFromPTXString allocates and returns a handle to a new program
  * object.  The program is created from PTX code held in the \a NULL-terminated string \a
  * ptx from function \a program_name.
  *
  * @param[in]   context        The context to create the program in
  * @param[in]   ptx            The string containing the PTX code
  * @param[in]   program_name   The name of the PTX function to create the program from
  * @param[in]   program        Handle to the program to be created
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_INVALID_SOURCE
  *
  * <B>History</B>
  *
  * @ref rtProgramCreateFromPTXString was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref RT_PROGRAM,
  * @ref rtProgramCreateFromPTXFile,
  * @ref rtProgramDestroy
  *
  */
  RTresult RTAPI rtProgramCreateFromPTXString(RTcontext context, const char* ptx, const char* program_name, RTprogram* program);

  /**
  * @brief Creates a new program object
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramCreateFromPTXFile allocates and returns a handle to a new program object.
  * The program is created from PTX code held in \a filename from function \a program_name.
  *
  * @param[in]   context        The context to create the program in
  * @param[in]   filename       Path to the file containing the PTX code
  * @param[in]   program_name   The name of the PTX function to create the program from
  * @param[in]   program        Handle to the program to be created
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_INVALID_SOURCE
  * - @ref RT_ERROR_FILE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtProgramCreateFromPTXFile was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref RT_PROGRAM,
  * @ref rtProgramCreateFromPTXString,
  * @ref rtProgramDestroy
  *
  */
  RTresult RTAPI rtProgramCreateFromPTXFile(RTcontext context, const char* filename, const char* program_name, RTprogram* program);

  /**
  * @brief Destroys a program object
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramDestroy removes \a program from its context and deletes it.
  * \a program should be a value returned by \a rtProgramCreate*.
  * Associated variables declared via @ref rtProgramDeclareVariable are destroyed.
  * After the call, \a program is no longer a valid handle.
  *
  * @param[in]   program   Handle of the program to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtProgramDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramCreateFromPTXFile,
  * @ref rtProgramCreateFromPTXString
  *
  */
  RTresult RTAPI rtProgramDestroy(RTprogram program);

  /**
  * @brief Validates the state of a program
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramValidate checks \a program for completeness.  If \a program or any of
  * the objects attached to \a program are not valid, returns @ref
  * RT_ERROR_INVALID_CONTEXT.
  *
  * @param[in]   program   The program to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtProgramValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramCreateFromPTXFile,
  * @ref rtProgramCreateFromPTXString
  *
  */
  RTresult RTAPI rtProgramValidate(RTprogram program);

  /**
  * @brief Gets the context object that created a program
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramGetContext returns a handle to the context object that was used to
  * create \a program. Returns @ref RT_ERROR_INVALID_VALUE if \a context is \a NULL.
  *
  * @param[in]   program   The program to be queried for its context object
  * @param[out]  context   The return handle for the requested context object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtProgramGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtProgramGetContext(RTprogram program, RTcontext* context);

  /**
  * @brief Declares a new named variable associated with a program
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramDeclareVariable declares a new variable, \a name, and associates it with
  * the program.  A variable can only be declared with the same name once on the program.
  * Any attempt to declare multiple variables with the same name will cause the call to
  * fail and return @ref RT_ERROR_VARIABLE_REDECLARED.  If \a name or\a v is \a NULL
  * returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   program   The program the declared variable will be attached to
  * @param[in]   name      The name of the variable to be created
  * @param[out]  v         Return handle to the variable to be created
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_REDECLARED
  * - @ref RT_ERROR_ILLEGAL_SYMBOL
  *
  * <B>History</B>
  *
  * @ref rtProgramDeclareVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramRemoveVariable,
  * @ref rtProgramGetVariable,
  * @ref rtProgramGetVariableCount,
  * @ref rtProgramQueryVariable
  *
  */
  RTresult RTAPI rtProgramDeclareVariable(RTprogram program, const char* name, RTvariable* v);

  /**
  * @brief Returns a handle to the named variable attached to a program
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramQueryVariable returns a handle to a variable object, in \a *v, attached
  * to \a program referenced by the \a NULL-terminated string \a name.  If \a name is not
  * the name of a variable attached to \a program, \a *v will be \a NULL after the call.
  *
  * @param[in]   program   The program to be queried for the named variable
  * @param[in]   name      The name of the program to be queried for
  * @param[out]  v         The return handle to the variable object
  * @param  program   Handle to the program to be created
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtProgramQueryVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramDeclareVariable,
  * @ref rtProgramRemoveVariable,
  * @ref rtProgramGetVariable,
  * @ref rtProgramGetVariableCount
  *
  */
  RTresult RTAPI rtProgramQueryVariable(RTprogram program, const char* name, RTvariable* v);

  /**
  * @brief Removes the named variable from a program
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramRemoveVariable removes variable \a v from the \a program object.  Once a
  * variable has been removed from this program, another variable with the same name as
  * the removed variable may be declared.
  *
  * @param[in]   program   The program to remove the variable from
  * @param[in]   v         The variable to remove
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtProgramRemoveVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramDeclareVariable,
  * @ref rtProgramGetVariable,
  * @ref rtProgramGetVariableCount,
  * @ref rtProgramQueryVariable
  *
  */
  RTresult RTAPI rtProgramRemoveVariable(RTprogram program, RTvariable v);

  /**
  * @brief Returns the number of variables attached to a program
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramGetVariableCount returns, in \a *count, the number of variable objects that
  * have been attached to \a program.
  *
  * @param[in]   program   The program to be queried for its variable count
  * @param[out]  count     The return handle for the number of variables attached to this program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtProgramGetVariableCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramDeclareVariable,
  * @ref rtProgramRemoveVariable,
  * @ref rtProgramGetVariable,
  * @ref rtProgramQueryVariable
  *
  */
  RTresult RTAPI rtProgramGetVariableCount(RTprogram program, unsigned int* count);

  /**
  * @brief Returns a handle to a variable attached to a program by index
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramGetVariable returns a handle to a variable in \a *v attached to \a
  * program with @ref rtProgramDeclareVariable by \a index.  \a index must be between
  * 0 and one less than the value returned by @ref rtProgramGetVariableCount.  The order
  * in which variables are enumerated is not constant and may change as variables are
  * attached and removed from the program object.
  *
  * @param[in]   program   The program to be queried for the indexed variable object
  * @param[in]   index     The index of the variable to return
  * @param[out]  v         Return handle to the variable object specified by the index
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtProgramGetVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtProgramDeclareVariable,
  * @ref rtProgramRemoveVariable,
  * @ref rtProgramGetVariableCount,
  * @ref rtProgramQueryVariable
  *
  */
  RTresult RTAPI rtProgramGetVariable(RTprogram program, unsigned int index, RTvariable* v);


  /**
  * @brief Returns the ID for the Program object
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtProgramGetId returns an ID for the provided program.  The returned ID is used
  * to reference \a program from device code.  If \a program_id is \a NULL or the \a
  * program is not a valid \a RTprogram, returns @ref RT_ERROR_INVALID_VALUE.
  * @ref RT_PROGRAM_ID_NULL can be used as a sentinel for a non-existent program, since
  * this value will never be returned as a valid program id.
  *
  * @param[in]   program      The program to be queried for its id
  * @param[out]  program_id   The returned ID of the program.
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtProgramGetId was introduced in OptiX 3.6.
  *
  * <B>See also</B>
  * @ref rtContextGetProgramFromId
  *
  */
  RTresult RTAPI rtProgramGetId(RTprogram program, int* program_id);

  /**
  * @brief Gets an RTprogram corresponding to the program id
  *
  * @ingroup Program
  *
  * <B>Description</B>
  *
  * @ref rtContextGetProgramFromId returns a handle to the program in \a *program
  * corresponding to the \a program_id supplied.  If \a program_id is not a valid
  * program handle, \a *program is set to \a NULL. Returns @ref RT_ERROR_INVALID_VALUE
  * if \a context is invalid or \a program_id is not a valid program handle.
  *
  * @param[in]   context     The context the program should be originated from
  * @param[in]   program_id  The ID of the program to query
  * @param[out]  program     The return handle for the program object corresponding to the program_id
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetProgramFromId was introduced in OptiX 3.6.
  *
  * <B>See also</B>
  * @ref rtProgramGetId
  *
  */
  RTresult RTAPI rtContextGetProgramFromId(RTcontext context, int program_id, RTprogram* program);

/************************************
 **
 **    Group object
 **
 ***********************************/

  /**
  * @brief Creates a new group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupCreate creates a new group within a context. \a context
  * specifies the target context, and should be a value returned by
  * @ref rtContextCreate.  Sets \a *group to the handle of a newly created group
  * within \a context. Returns @ref RT_ERROR_INVALID_VALUE if \a group is \a NULL.
  *
  * @param[in]   context   Specifies a context within which to create a new group
  * @param[out]  group     Returns a newly created group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupDestroy,
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtGroupCreate(RTcontext context, RTgroup* group);

  /**
  * @brief Destroys a group node
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupDestroy removes \a group from its context and deletes it.
  * \a group should be a value returned by @ref rtGroupCreate.
  * No child graph nodes are destroyed.
  * After the call, \a group is no longer a valid handle.
  *
  * @param[in]   group   Handle of the group node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupCreate
  *
  */
  RTresult RTAPI rtGroupDestroy(RTgroup group);

  /**
  * @brief Verifies the state of the group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupValidate checks \a group for completeness. If \a group or
  * any of the objects attached to \a group are not valid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   group   Specifies the group to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupCreate
  *
  */
  RTresult RTAPI rtGroupValidate(RTgroup group);

  /**
  * @brief Returns the context associated with a group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupGetContext queries a group for its associated context.
  * \a group specifies the group to query, and must be a value returned by
  * @ref rtGroupCreate. Sets \a *context to the context
  * associated with \a group.
  *
  * @param[in]   group     Specifies the group to query
  * @param[out]  context   Returns the context associated with the group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextCreate,
  * @ref rtGroupCreate
  *
  */
  RTresult RTAPI rtGroupGetContext(RTgroup group, RTcontext* context);

  /**
  * @brief Set the acceleration structure for a group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupSetAcceleration attaches an acceleration structure to a group. The acceleration
  * structure must have been previously created using @ref rtAccelerationCreate. Every group is
  * required to have an acceleration structure assigned in order to pass validation. The acceleration
  * structure will be built over the children of the group. For example, if an acceleration structure
  * is attached to a group that has a selector, a geometry group, and a transform child,
  * the acceleration structure will be built over the bounding volumes of these three objects.
  *
  * Note that it is legal to attach a single RTacceleration object to multiple groups, as long as
  * the underlying bounds of the children are the same. For example, if another group has three
  * children which are known to have the same bounding volumes as the ones in the example
  * above, the two groups can share an acceleration structure, thus saving build time. This is
  * true even if the details of the children, such as the actual type of a node or its geometry
  * content, differ from the first set of group children. All that is required is for a child
  * node at a given index to have the same bounds as the other group's child node at the same index.
  *
  * Sharing an acceleration structure this way corresponds to attaching an acceleration structure
  * to multiple geometry groups at lower graph levels using @ref rtGeometryGroupSetAcceleration.
  *
  * @param[in]   group          The group handle
  * @param[in]   acceleration   The acceleration structure to attach to the group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupSetAcceleration was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupGetAcceleration,
  * @ref rtAccelerationCreate,
  * @ref rtGeometryGroupSetAcceleration
  *
  */
  RTresult RTAPI rtGroupSetAcceleration(RTgroup group, RTacceleration acceleration);

  /**
  * @brief Returns the acceleration structure attached to a group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupGetAcceleration returns the acceleration structure attached to a group using @ref rtGroupSetAcceleration.
  * If no acceleration structure has previously been set, \a *acceleration is set to \a NULL.
  *
  * @param[in]   group          The group handle
  * @param[out]  acceleration   The returned acceleration structure object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupGetAcceleration was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupSetAcceleration,
  * @ref rtAccelerationCreate
  *
  */
  RTresult RTAPI rtGroupGetAcceleration(RTgroup group, RTacceleration* acceleration);

  /**
  * @brief Sets the number of child nodes to be attached to the group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupSetChildCount specifies the number of child slots in this group. Potentially existing links to children
  * at indices greater than \a count-1 are removed. If the call increases the number of slots, the newly
  * created slots are empty and need to be filled using @ref rtGroupSetChild before validation.
  *
  * @param[in]   group   The parent group handle
  * @param[in]   count   Number of child slots to allocate for the group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupSetChildCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupGetChild,
  * @ref rtGroupGetChildCount,
  * @ref rtGroupGetChildType,
  * @ref rtGroupSetChild
  *
  */
  RTresult RTAPI rtGroupSetChildCount(RTgroup group, unsigned int count);

  /**
  * @brief Returns the number of child slots for a group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupGetChildCount returns the number of child slots allocated using @ref
  * rtGroupSetChildCount.  This includes empty slots which may not yet have actual children assigned
  * by @ref rtGroupSetChild.  Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  *
  * @param[in]   group   The parent group handle
  * @param[out]  count   Returned number of child slots
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupGetChildCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupSetChild,
  * @ref rtGroupGetChild,
  * @ref rtGroupSetChildCount,
  * @ref rtGroupGetChildType
  *
  */
  RTresult RTAPI rtGroupGetChildCount(RTgroup group, unsigned int* count);

  /**
  * @brief Attaches a child node to a group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * Attaches a new child node \a child to the parent node
  * \a group. \a index specifies the number of the slot where the child
  * node gets attached. A sufficient number of slots must be allocated
  * using @ref rtGroupSetChildCount.
  * Legal child node types are @ref RTgroup, @ref RTselector, @ref RTgeometrygroup, and
  * @ref RTtransform.
  *
  * @param[in]   group   The parent group handle
  * @param[in]   index   The index in the parent's child slot array
  * @param[in]   child   The child node to be attached. Can be of type {@ref RTgroup, @ref RTselector,
  * @ref RTgeometrygroup, @ref RTtransform}
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupSetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupSetChildCount,
  * @ref rtGroupGetChildCount,
  * @ref rtGroupGetChild,
  * @ref rtGroupGetChildType
  *
  */
  RTresult RTAPI rtGroupSetChild(RTgroup group, unsigned int index, RTobject child);

  /**
  * @brief Returns a child node of a group
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupGetChild returns the child object at slot \a index of the parent \a group.
  * If no child has been assigned to the given slot, \a *child is set to \a NULL.
  * Returns @ref RT_ERROR_INVALID_VALUE if given an invalid child index or \a NULL pointer.
  *
  * @param[in]   group   The parent group handle
  * @param[in]   index   The index of the child slot to query
  * @param[out]  child   The returned child object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupGetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupSetChild,
  * @ref rtGroupSetChildCount,
  * @ref rtGroupGetChildCount,
  * @ref rtGroupGetChildType
  *
  */
  RTresult RTAPI rtGroupGetChild(RTgroup group, unsigned int index, RTobject* child);

  /**
  * @brief Get the type of a group child
  *
  * @ingroup GroupNode
  *
  * <B>Description</B>
  *
  * @ref rtGroupGetChildType returns the type of the group child at slot \a index.
  * If no child is associated with the given index, \a *type is set to
  * @ref RT_OBJECTTYPE_UNKNOWN and @ref RT_ERROR_INVALID_VALUE is returned.
  * Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  *
  * @param[in]   group   The parent group handle
  * @param[in]   index   The index of the child slot to query
  * @param[out]  type    The returned child type
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGroupGetChildType was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGroupSetChild,
  * @ref rtGroupGetChild,
  * @ref rtGroupSetChildCount,
  * @ref rtGroupGetChildCount
  *
  */
  RTresult RTAPI rtGroupGetChildType(RTgroup group, unsigned int index, RTobjecttype* type);

/************************************
 **
 **    Selector object
 **
 ***********************************/

  /**
  * @brief Creates a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * Creates a new Selector node within \a context. After calling
  * @ref rtSelectorCreate the new node is in a "raw" state.  For the node
  * to be functional, a visit program must be assigned using
  * @ref rtSelectorSetVisitProgram. Furthermore, a number of (zero or
  * more) children can be attached by using @ref rtSelectorSetChildCount and
  * @ref rtSelectorSetChild. Sets \a *selector to the handle of a newly
  * created selector within \a context. Returns @ref RT_ERROR_INVALID_VALUE if \a selector is \a NULL.
  *
  * @param[in]   context    Specifies the rendering context of the Selector node
  * @param[out]  selector   New Selector node handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorDestroy,
  * @ref rtSelectorValidate,
  * @ref rtSelectorGetContext,
  * @ref rtSelectorSetVisitProgram,
  * @ref rtSelectorSetChildCount,
  * @ref rtSelectorSetChild
  *
  */
  RTresult RTAPI rtSelectorCreate(RTcontext context, RTselector* selector);

  /**
  * @brief Destroys a selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorDestroy removes \a selector from its context and deletes it.  \a selector should
  * be a value returned by @ref rtSelectorCreate.  Associated variables declared via @ref
  * rtSelectorDeclareVariable are destroyed, but no child graph nodes are destroyed.  After the
  * call, \a selector is no longer a valid handle.
  *
  * @param[in]   selector   Handle of the selector node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorCreate,
  * @ref rtSelectorValidate,
  * @ref rtSelectorGetContext
  *
  */
  RTresult RTAPI rtSelectorDestroy(RTselector selector);

  /**
  * @brief Checks a Selector node for internal consistency
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorValidate recursively checks consistency of the Selector
  * node \a selector and its children, i.e., it tries to validate the
  * whole model sub-tree with \a selector as root. For a Selector node to
  * be valid, it must be assigned a visit program, and the number of its
  * children must match the number specified by
  * @ref rtSelectorSetChildCount.
  *
  * @param[in]   selector   Selector root node of a model sub-tree to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorCreate,
  * @ref rtSelectorDestroy,
  * @ref rtSelectorGetContext,
  * @ref rtSelectorSetVisitProgram,
  * @ref rtSelectorSetChildCount,
  * @ref rtSelectorSetChild
  *
  */
  RTresult RTAPI rtSelectorValidate(RTselector selector);

  /**
  * @brief Returns the context of a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorGetContext returns in \a context the rendering context
  * in which the Selector node \a selector has been created.
  *
  * @param[in]   selector   Selector node handle
  * @param[out]  context    The context, \a selector belongs to
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorCreate,
  * @ref rtSelectorDestroy,
  * @ref rtSelectorValidate
  *
  */
  RTresult RTAPI rtSelectorGetContext(RTselector selector, RTcontext* context);

  /**
  * @brief Assigns a visit program to a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorSetVisitProgram specifies a visit program that is
  * executed when the Selector node \a selector gets visited by a ray
  * during traversal of the model graph. A visit program steers how
  * traversal of the Selectors's children is performed.  It usually
  * chooses only a single child to continue traversal, but is also allowed
  * to process zero or multiple children. Programs can be created from PTX
  * files using @ref rtProgramCreateFromPTXFile.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   program    Program handle associated with a visit program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtSelectorSetVisitProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorGetVisitProgram,
  * @ref rtProgramCreateFromPTXFile
  *
  */
  RTresult RTAPI rtSelectorSetVisitProgram(RTselector selector, RTprogram program);

  /**
  * @brief Returns the currently assigned visit program
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorGetVisitProgram returns in \a program a handle of the
  * visit program curently bound to \a selector.
  *
  * @param[in]   selector   Selector node handle
  * @param[out]  program    Current visit progam assigned to \a selector
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetVisitProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorSetVisitProgram
  *
  */
  RTresult RTAPI rtSelectorGetVisitProgram(RTselector selector, RTprogram* program);

  /**
  * @brief Specifies the number of child nodes to be
  * attached to a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorSetChildCount allocates a number of children slots,
  * i.e., it pre-defines the exact number of child nodes the parent
  * Selector node \a selector will have.  Child nodes have to be attached
  * to the Selector node using @ref rtSelectorSetChild. Empty slots will
  * cause a validation error.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   count      Number of child nodes to be attached to \a selector
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorSetChildCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorValidate,
  * @ref rtSelectorGetChildCount,
  * @ref rtSelectorSetChild,
  * @ref rtSelectorGetChild,
  * @ref rtSelectorGetChildType
  *
  */
  RTresult RTAPI rtSelectorSetChildCount(RTselector selector, unsigned int count);

  /**
  * @brief Returns the number of child node slots of
  * a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorGetChildCount returns in \a count the number of child
  * node slots that have been previously reserved for the Selector node
  * \a selector by @ref rtSelectorSetChildCount. The value of \a count
  * does not reflect the actual number of child nodes that have so far
  * been attached to the Selector node using @ref rtSelectorSetChild.
  *
  * @param[in]   selector   Selector node handle
  * @param[out]  count      Number of child node slots reserved for \a selector
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetChildCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorSetChildCount,
  * @ref rtSelectorSetChild,
  * @ref rtSelectorGetChild,
  * @ref rtSelectorGetChildType
  *
  */
  RTresult RTAPI rtSelectorGetChildCount(RTselector selector, unsigned int* count);

  /**
  * @brief Attaches a child node to a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * Attaches a new child node \a child to the parent node
  * \a selector. \a index specifies the number of the slot where the child
  * node gets attached.  The index value must be lower than the number
  * previously set by @ref rtSelectorSetChildCount, thus it must be in
  * the range from \a 0 to @ref rtSelectorGetChildCount \a -1.  Legal child
  * node types are @ref RTgroup, @ref RTselector, @ref RTgeometrygroup, and
  * @ref RTtransform.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   index      Index of the parent slot the node \a child gets attached to
  * @param[in]   child      Child node to be attached. Can be {@ref RTgroup, @ref RTselector,
  * @ref RTgeometrygroup, @ref RTtransform}
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorSetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorSetChildCount,
  * @ref rtSelectorGetChildCount,
  * @ref rtSelectorGetChild,
  * @ref rtSelectorGetChildType
  *
  */
  RTresult RTAPI rtSelectorSetChild(RTselector selector, unsigned int index, RTobject child);

  /**
  * @brief Returns a child node that is attached to a
  * Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorGetChild returns in \a child a handle of the child node
  * currently attached to \a selector at slot \a index. The index value
  * must be lower than the number previously set by
  * @ref rtSelectorSetChildCount, thus it must be in the range from \a 0
  * to @ref rtSelectorGetChildCount \a - 1. The returned pointer is of generic
  * type @ref RTobject and needs to be cast to the actual child type, which
  * can be @ref RTgroup, @ref RTselector, @ref RTgeometrygroup, or
  * @ref RTtransform. The actual type of \a child can be queried using
  * @ref rtSelectorGetChildType;
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   index      Child node index
  * @param[out]  child      Child node handle. Can be {@ref RTgroup, @ref RTselector,
  * @ref RTgeometrygroup, @ref RTtransform}
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorSetChildCount,
  * @ref rtSelectorGetChildCount,
  * @ref rtSelectorSetChild,
  * @ref rtSelectorGetChildType
  *
  */
  RTresult RTAPI rtSelectorGetChild(RTselector selector, unsigned int index, RTobject* child);

  /**
  * @brief Returns type information about a Selector
  * child node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorGetChildType queries the type of the child node
  * attached to \a selector at slot \a index.
  * If no child is associated with the given index, \a *type is set to
  * @ref RT_OBJECTTYPE_UNKNOWN and @ref RT_ERROR_INVALID_VALUE is returned.
  * Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  * The returned type is one of:
  *
  *   @ref RT_OBJECTTYPE_GROUP
  *   @ref RT_OBJECTTYPE_GEOMETRY_GROUP
  *   @ref RT_OBJECTTYPE_TRANSFORM
  *   @ref RT_OBJECTTYPE_SELECTOR
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   index      Child node index
  * @param[out]  type       Type of the child node
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetChildType was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorSetChildCount,
  * @ref rtSelectorGetChildCount,
  * @ref rtSelectorSetChild,
  * @ref rtSelectorGetChild
  *
  */
  RTresult RTAPI rtSelectorGetChildType(RTselector selector, unsigned int index, RTobjecttype* type);

  /**
  * @brief Declares a variable associated with a
  * Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * Declares a new variable identified by \a name, and associates it with
  * the Selector node \a selector. The new variable handle is returned in
  * \a v. After declaration, a variable does not have a type until its
  * value is set by an \a rtVariableSet{...} function. Once a variable
  * type has been set, it cannot be changed, i.e., only
  * \a rtVariableSet{...} functions of the same type can be used to
  * change the value of the variable.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   name       Variable identifier
  * @param[out]  v          New variable handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_REDECLARED
  * - @ref RT_ERROR_ILLEGAL_SYMBOL
  *
  * <B>History</B>
  *
  * @ref rtSelectorDeclareVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorQueryVariable,
  * @ref rtSelectorRemoveVariable,
  * @ref rtSelectorGetVariableCount,
  * @ref rtSelectorGetVariable,
  * @ref rtVariableSet{...}
  *
  */
  RTresult RTAPI rtSelectorDeclareVariable(RTselector selector, const char* name, RTvariable* v);

  /**
  * @brief Returns a variable associated with a
  * Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * Returns in \a v a handle to the variable identified by \a name, which
  * is associated with the Selector node \a selector. The current value of
  * a variable can be retrieved from its handle by using an appropriate
  * \a rtVariableGet{...} function matching the variable's type.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   name       Variable identifier
  * @param[out]  v          Variable handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorQueryVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorDeclareVariable,
  * @ref rtSelectorRemoveVariable,
  * @ref rtSelectorGetVariableCount,
  * @ref rtSelectorGetVariable,
  * \a rtVariableGet{...}
  *
  */
  RTresult RTAPI rtSelectorQueryVariable(RTselector selector, const char* name, RTvariable* v);

  /**
  * @brief Removes a variable from a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorRemoveVariable removes the variable \a v from the
  * Selector node \a selector and deletes it. The handle \a v must be
  * considered invalid afterwards.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   v          Variable handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtSelectorRemoveVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorDeclareVariable,
  * @ref rtSelectorQueryVariable,
  * @ref rtSelectorGetVariableCount,
  * @ref rtSelectorGetVariable
  *
  */
  RTresult RTAPI rtSelectorRemoveVariable(RTselector selector, RTvariable v);

  /**
  * @brief Returns the number of variables
  * attached to a Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * @ref rtSelectorGetVariableCount returns in \a count the number of
  * variables that are currently attached to the Selector node
  * \a selector.
  *
  * @param[in]   selector   Selector node handle
  * @param[out]  count      Number of variables associated with \a selector
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetVariableCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorDeclareVariable,
  * @ref rtSelectorQueryVariable,
  * @ref rtSelectorRemoveVariable,
  * @ref rtSelectorGetVariable
  *
  */
  RTresult RTAPI rtSelectorGetVariableCount(RTselector selector, unsigned int* count);

  /**
  * @brief Returns a variable associated with a
  * Selector node
  *
  * @ingroup SelectorNode
  *
  * <B>Description</B>
  *
  * Returns in \a v a handle to the variable located at position \a index
  * in the Selectors's variable array. \a index is a sequential number
  * depending on the order of variable declarations. The index must be
  * in the range from \a 0 to @ref rtSelectorGetVariableCount \a - 1.  The
  * current value of a variable can be retrieved from its handle by using
  * an appropriate \a rtVariableGet{...} function matching the
  * variable's type.
  *
  * @param[in]   selector   Selector node handle
  * @param[in]   index      Variable index
  * @param[out]  v          Variable handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtSelectorGetVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtSelectorDeclareVariable,
  * @ref rtSelectorQueryVariable,
  * @ref rtSelectorRemoveVariable,
  * @ref rtSelectorGetVariableCount,
  * \a rtVariableGet{...}
  *
  */
  RTresult RTAPI rtSelectorGetVariable(RTselector selector, unsigned int index, RTvariable* v);

/************************************
 **
 **    Transform object
 **
 ***********************************/

  /**
  * @brief Creates a new Transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * Creates a new Transform node within the given context. For the node to be functional, a child
  * node must be attached using @ref rtTransformSetChild.  A transformation matrix can be associated
  * with the transform node with @ref rtTransformSetMatrix. Sets \a *transform to the handle of a
  * newly created transform within \a context. Returns @ref RT_ERROR_INVALID_VALUE if \a transform
  * is \a NULL.
  *
  * @param[in]   context    Specifies the rendering context of the Transform node
  * @param[out]  transform  New Transform node handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformDestroy,
  * @ref rtTransformValidate,
  * @ref rtTransformGetContext,
  * @ref rtTransformSetMatrix,
  * @ref rtTransformGetMatrix,
  * @ref rtTransformSetChild,
  * @ref rtTransformGetChild,
  * @ref rtTransformGetChildType
  *
  */
  RTresult RTAPI rtTransformCreate(RTcontext context, RTtransform* transform);

  /**
  * @brief Destroys a transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformDestroy removes \a transform from its context and deletes it.
  * \a transform should be a value returned by @ref rtTransformCreate.
  * No child graph nodes are destroyed.
  * After the call, \a transform is no longer a valid handle.
  *
  * @param[in]   transform   Handle of the transform node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformCreate,
  * @ref rtTransformValidate,
  * @ref rtTransformGetContext
  *
  */
  RTresult RTAPI rtTransformDestroy(RTtransform transform);

  /**
  * @brief Checks a Transform node for internal
  * consistency
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformValidate recursively checks consistency of the
  * Transform node \a transform and its child, i.e., it tries to validate
  * the whole model sub-tree with \a transform as root. For a Transform
  * node to be valid, it must have a child node attached. It is, however,
  * not required to explicitly set a transformation matrix. Without a specified
  * transformation matrix, the identity matrix is applied.
  *
  * @param[in]   transform   Transform root node of a model sub-tree to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformCreate,
  * @ref rtTransformDestroy,
  * @ref rtTransformGetContext,
  * @ref rtTransformSetMatrix,
  * @ref rtTransformSetChild
  *
  */
  RTresult RTAPI rtTransformValidate(RTtransform transform);

  /**
  * @brief Returns the context of a Transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformGetContext queries a transform node for its associated context.  \a transform
  * specifies the transform node to query, and should be a value returned by @ref
  * rtTransformCreate. Sets \a *context to the context associated with \a transform.
  *
  * @param[in]   transform   Transform node handle
  * @param[out]  context     The context associated with \a transform
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformCreate,
  * @ref rtTransformDestroy,
  * @ref rtTransformValidate
  *
  */
  RTresult RTAPI rtTransformGetContext(RTtransform transform, RTcontext* context);

  /**
  * @brief Associates an affine transformation matrix
  * with a Transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformSetMatrix associates a 4x4 matrix with the Transform
  * node \a transform. The provided transformation matrix results in a
  * corresponding affine transformation of all geometry contained in the
  * sub-tree with \a transform as root. At least one of the pointers
  * \a matrix and \a inverse_matrix must be non-\a NULL. If exactly one
  * pointer is valid, the other matrix will be computed. If both are
  * valid, the matrices will be used as-is. If \a transpose is \a 0,
  * source matrices are expected to be in row-major format, i.e., matrix
  * rows are contiguously laid out in memory:
  *
  *   float matrix[4*4] = { a11,  a12,  a13,  a14,
  *                         a21,  a22,  a23,  a24,
  *                         a31,  a32,  a33,  a34,
  *                         a41,  a42,  a43,  a44 };
  *
  * Here, the translational elements \a a14, \a a24, and \a a34 are at the
  * 4th, 8th, and 12th position the matrix array.  If the supplied
  * matrices are in column-major format, a non-0 \a transpose flag
  * can be used to trigger an automatic transpose of the input matrices.
  *
  * @param[in]   transform        Transform node handle
  * @param[in]   transpose        Flag indicating whether \a matrix and \a inverse_matrix should be
  * transposed
  * @param[in]   matrix           Affine matrix (4x4 float array)
  * @param[in]   inverse_matrix   Inverted form of \a matrix
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformSetMatrix was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformGetMatrix
  *
  */
  RTresult RTAPI rtTransformSetMatrix(RTtransform transform, int transpose, const float* matrix, const float* inverse_matrix);

  /**
  * @brief Returns the affine matrix and its inverse associated with a Transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformGetMatrix returns in \a matrix the affine matrix that
  * is currently used to perform a transformation of the geometry
  * contained in the sub-tree with \a transform as root. The corresponding
  * inverse matrix will be retured in \a inverse_matrix. One or both
  * pointers are allowed to be \a NULL. If \a transpose is \a 0, matrices
  * are returned in row-major format, i.e., matrix rows are contiguously
  * laid out in memory. If \a transpose is non-zero, matrices are returned
  * in column-major format. If non-\a NULL, matrix pointers must point to a
  * float array of at least 16 elements.
  *
  * @param[in]   transform        Transform node handle
  * @param[in]   transpose        Flag indicating whether \a matrix and \a inverse_matrix should be
  * transposed
  * @param[out]  matrix           Affine matrix (4x4 float array)
  * @param[out]  inverse_matrix   Inverted form of \a matrix
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformGetMatrix was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformSetMatrix
  *
  */
  RTresult RTAPI rtTransformGetMatrix(RTtransform transform, int transpose, float* matrix, float* inverse_matrix);

  /**
  * @brief Attaches a child node to a Transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * Attaches a child node \a child to the parent node \a transform. Legal
  * child node types are @ref RTgroup, @ref RTselector, @ref RTgeometrygroup,
  * and @ref RTtransform. A transform node must have exactly one child.  If
  * a tranformation matrix has been attached to \a transform with
  * @ref rtTransformSetMatrix, it is effective on the model sub-tree with
  * \a child as root node.
  *
  * @param[in]   transform   Transform node handle
  * @param[in]   child       Child node to be attached. Can be {@ref RTgroup, @ref RTselector,
  * @ref RTgeometrygroup, @ref RTtransform}
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformSetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformSetMatrix,
  * @ref rtTransformGetChild,
  * @ref rtTransformGetChildType
  *
  */
  RTresult RTAPI rtTransformSetChild(RTtransform transform, RTobject child);

  /**
  * @brief Returns the child node that is attached to a
  * Transform node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformGetChild returns in \a child a handle of the child
  * node currently attached to \a transform. The returned pointer is of
  * generic type @ref RTobject and needs to be cast to the actual child
  * type, which can be @ref RTgroup, @ref RTselector, @ref RTgeometrygroup, or
  * @ref RTtransform. The actual type of \a child can be queried using
  * @ref rtTransformGetChildType.
  * Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  *
  * @param[in]   transform   Transform node handle
  * @param[out]  child       Child node handle. Can be {@ref RTgroup, @ref RTselector,
  * @ref RTgeometrygroup, @ref RTtransform}
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformGetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformSetChild,
  * @ref rtTransformGetChildType
  *
  */
  RTresult RTAPI rtTransformGetChild(RTtransform transform, RTobject* child);

  /**
  * @brief Returns type information about a
  * Transform child node
  *
  * @ingroup TransformNode
  *
  * <B>Description</B>
  *
  * @ref rtTransformGetChildType queries the type of the child node
  * attached to \a transform. If no child is attached, \a *type is set to
  * @ref RT_OBJECTTYPE_UNKNOWN and @ref RT_ERROR_INVALID_VALUE is returned.
  * Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  * The returned type is one of:
  *
  *  - @ref RT_OBJECTTYPE_GROUP
  *  - @ref RT_OBJECTTYPE_GEOMETRY_GROUP
  *  - @ref RT_OBJECTTYPE_TRANSFORM
  *  - @ref RT_OBJECTTYPE_SELECTOR
  *
  * @param[in]   transform   Transform node handle
  * @param[out]  type        Type of the child node
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTransformGetChildType was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTransformSetChild,
  * @ref rtTransformGetChild
  *
  */
  RTresult RTAPI rtTransformGetChildType(RTtransform transform, RTobjecttype* type);

/************************************
 **
 **    GeometryGroup object
 **
 ***********************************/

  /**
  * @brief Creates a new geometry group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupCreate creates a new geometry group within a context. \a context
  * specifies the target context, and should be a value returned by @ref rtContextCreate.
  * Sets \a *geometrygroup to the handle of a newly created geometry group within \a context.
  * Returns @ref RT_ERROR_INVALID_VALUE if \a geometrygroup is \a NULL.
  *
  * @param[in]   context         Specifies a context within which to create a new geometry group
  * @param[out]  geometrygroup   Returns a newly created geometry group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupDestroy,
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtGeometryGroupCreate(RTcontext context, RTgeometrygroup* geometrygroup);

  /**
  * @brief Destroys a geometry group node
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupDestroy removes \a geometrygroup from its context and deletes it.
  * \a geometrygroup should be a value returned by @ref rtGeometryGroupCreate.
  * No child graph nodes are destroyed.
  * After the call, \a geometrygroup is no longer a valid handle.
  *
  * @param[in]   geometrygroup   Handle of the geometry group node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupCreate
  *
  */
  RTresult RTAPI rtGeometryGroupDestroy(RTgeometrygroup geometrygroup);

  /**
  * @brief Validates the state of the geometry group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupValidate checks \a geometrygroup for completeness. If \a geometrygroup or
  * any of the objects attached to \a geometrygroup are not valid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   geometrygroup   Specifies the geometry group to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupCreate
  *
  */
  RTresult RTAPI rtGeometryGroupValidate(RTgeometrygroup geometrygroup);

  /**
  * @brief Returns the context associated with a geometry group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupGetContext queries a geometry group for its associated context.
  * \a geometrygroup specifies the geometry group to query, and must be a value returned by
  * @ref rtGeometryGroupCreate. Sets \a *context to the context
  * associated with \a geometrygroup.
  *
  * @param[in]   geometrygroup   Specifies the geometry group to query
  * @param[out]  context         Returns the context associated with the geometry group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextCreate,
  * @ref rtGeometryGroupCreate
  *
  */
  RTresult RTAPI rtGeometryGroupGetContext(RTgeometrygroup geometrygroup, RTcontext* context);

  /**
  * @brief Set the acceleration structure for a group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupSetAcceleration attaches an acceleration structure to a geometry group. The
  * acceleration structure must have been previously created using @ref rtAccelerationCreate. Every
  * geometry group is required to have an acceleration structure assigned in order to pass
  * validation. The acceleration structure will be built over the primitives contained in all
  * children of the geometry group. This enables a single acceleration structure to be built over
  * primitives of multiple geometry instances.  Note that it is legal to attach a single
  * RTacceleration object to multiple geometry groups, as long as the underlying geometry of all
  * children is the same. This corresponds to attaching an acceleration structure to multiple groups
  * at higher graph levels using @ref rtGroupSetAcceleration.
  *
  * @param[in]   geometrygroup   The geometry group handle
  * @param[in]   acceleration    The acceleration structure to attach to the geometry group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupSetAcceleration was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupGetAcceleration,
  * @ref rtAccelerationCreate,
  * @ref rtGroupSetAcceleration
  *
  */
  RTresult RTAPI rtGeometryGroupSetAcceleration(RTgeometrygroup geometrygroup, RTacceleration acceleration);

  /**
  * @brief Returns the acceleration structure attached to a geometry group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupGetAcceleration returns the acceleration structure attached to a geometry
  * group using @ref rtGeometryGroupSetAcceleration.  If no acceleration structure has previously
  * been set, \a *acceleration is set to \a NULL.
  *
  * @param[in]   geometrygroup   The geometry group handle
  * @param[out]  acceleration    The returned acceleration structure object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupGetAcceleration was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupSetAcceleration,
  * @ref rtAccelerationCreate
  *
  */
  RTresult RTAPI rtGeometryGroupGetAcceleration(RTgeometrygroup geometrygroup, RTacceleration* acceleration);

  /**
  * @brief Sets the number of child nodes to be attached to the group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupSetChildCount specifies the number of child slots in this geometry
  * group. Potentially existing links to children at indices greater than \a count-1 are removed. If
  * the call increases the number of slots, the newly created slots are empty and need to be filled
  * using @ref rtGeometryGroupSetChild before validation.
  *
  * @param[in]   geometrygroup   The parent geometry group handle
  * @param[in]   count           Number of child slots to allocate for the geometry group
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupSetChildCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupGetChild,
  * @ref rtGeometryGroupGetChildCount
  * @ref rtGeometryGroupSetChild
  *
  */
  RTresult RTAPI rtGeometryGroupSetChildCount(RTgeometrygroup geometrygroup, unsigned int count);

  /**
  * @brief Returns the number of child slots for a group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupGetChildCount returns the number of child slots allocated using @ref
  * rtGeometryGroupSetChildCount.  This includes empty slots which may not yet have actual children
  * assigned by @ref rtGeometryGroupSetChild.
  *
  * @param[in]   geometrygroup   The parent geometry group handle
  * @param[out]  count           Returned number of child slots
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupGetChildCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupSetChild,
  * @ref rtGeometryGroupGetChild,
  * @ref rtGeometryGroupSetChildCount
  *
  */
  RTresult RTAPI rtGeometryGroupGetChildCount(RTgeometrygroup geometrygroup, unsigned int* count);

  /**
  * @brief Attaches a child node to a geometry group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupSetChild attaches a new child node \a geometryinstance to the parent node
  * \a geometrygroup. \a index specifies the number of the slot where the child
  * node gets attached.  The index value must be lower than the number
  * previously set by @ref rtGeometryGroupSetChildCount.
  *
  * @param[in]   geometrygroup      The parent geometry group handle
  * @param[in]   index              The index in the parent's child slot array
  * @param[in]   geometryinstance   The child node to be attached
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupSetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupSetChildCount,
  * @ref rtGeometryGroupGetChildCount,
  * @ref rtGeometryGroupGetChild
  *
  */
  RTresult RTAPI rtGeometryGroupSetChild(RTgeometrygroup geometrygroup, unsigned int index, RTgeometryinstance geometryinstance);

  /**
  * @brief Returns a child node of a geometry group
  *
  * @ingroup GeometryGroup
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGroupGetChild returns the child geometry instance at slot \a index of the parent
  * \a geometrygroup.  If no child has been assigned to the given slot, \a *geometryinstance is set
  * to \a NULL.  Returns @ref RT_ERROR_INVALID_VALUE if given an invalid child index or \a NULL
  * pointer.
  *
  * @param[in]   geometrygroup      The parent geometry group handle
  * @param[in]   index              The index of the child slot to query
  * @param[out]  geometryinstance   The returned child geometry instance
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGroupGetChild was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGroupSetChild,
  * @ref rtGeometryGroupSetChildCount,
  * @ref rtGeometryGroupGetChildCount,
  *
  */
  RTresult RTAPI rtGeometryGroupGetChild(RTgeometrygroup geometrygroup, unsigned int index, RTgeometryinstance* geometryinstance);

/************************************
 **
 **    Acceleration object
 **
 ***********************************/

  /**
  * @brief Creates a new acceleration structure
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationCreate creates a new ray tracing acceleration structure within a context.  An
  * acceleration structure is used by attaching it to a group or geometry group by calling @ref
  * rtGroupSetAcceleration or @ref rtGeometryGroupSetAcceleration. Note that an acceleration
  * structure can be shared by attaching it to multiple groups or geometry groups if the underlying
  * geometric structures are the same, see @ref rtGroupSetAcceleration and @ref
  * rtGeometryGroupSetAcceleration for more details. A newly created acceleration structure is
  * initially in dirty state.  Sets \a *acceleration to the handle of a newly created acceleration
  * structure within \a context.  Returns @ref RT_ERROR_INVALID_VALUE if \a acceleration is \a NULL.
  *
  * @param[in]   context        Specifies a context within which to create a new acceleration structure
  * @param[out]  acceleration   Returns the newly created acceleration structure
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationDestroy,
  * @ref rtContextCreate,
  * @ref rtAccelerationMarkDirty,
  * @ref rtAccelerationIsDirty,
  * @ref rtGroupSetAcceleration,
  * @ref rtGeometryGroupSetAcceleration
  *
  */
  RTresult RTAPI rtAccelerationCreate(RTcontext context, RTacceleration* acceleration);

  /**
  * @brief Destroys an acceleration structure object
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationDestroy removes \a acceleration from its context and deletes it.
  * \a acceleration should be a value returned by @ref rtAccelerationCreate.
  * After the call, \a acceleration is no longer a valid handle.
  *
  * @param[in]   acceleration   Handle of the acceleration structure to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationCreate
  *
  */
  RTresult RTAPI rtAccelerationDestroy(RTacceleration acceleration);

  /**
  * @brief Validates the state of an acceleration structure
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationValidate checks \a acceleration for completeness. If \a acceleration is
  * not valid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   acceleration   The acceleration structure handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationCreate
  *
  */
  RTresult RTAPI rtAccelerationValidate(RTacceleration acceleration);

  /**
  * @brief Returns the context associated with an acceleration structure
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationGetContext queries an acceleration structure for its associated context.
  * The context handle is returned in \a *context.
  *
  * @param[in]   acceleration   The acceleration structure handle
  * @param[out]  context        Returns the context associated with the acceleration structure
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationCreate
  *
  */
  RTresult RTAPI rtAccelerationGetContext(RTacceleration acceleration, RTcontext* context);


  /**
  * @brief Specifies the builder to be used for an acceleration structure
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationSetBuilder specifies the method used to construct the ray tracing
  * acceleration structure represented by \a acceleration. A builder must be set for the
  * acceleration structure to pass validation.  The current builder can be changed at any time,
  * including after a call to @ref rtContextLaunch "rtContextLaunch".  In this case, data previously
  * computed for the acceleration structure is invalidated and the acceleration will be marked
  * dirty.
  *
  * \a builder can take one of the following values:
  *
  * - "NoAccel": Specifies that no acceleration structure is explicitly built. Traversal linearly loops through the
  * list of primitives to intersect. This can be useful e.g. for higher level groups with only few children, where managing a more complex structure introduces unnecessary overhead.
  *
  * - "Bvh": A standard bounding volume hierarchy, useful for most types of graph levels and geometry. Medium build speed, good ray tracing performance.
  *
  * - "Sbvh": A high quality BVH variant for maximum ray tracing performance. Slower build speed and slightly higher memory footprint than "Bvh".
  *
  * - "Trbvh": High quality similar to Sbvh but with fast build performance. The Trbvh builder uses about 2.5 times the size of the final BVH for scratch space. A CPU-based Trbvh builder that does not have the memory constraints is available. OptiX includes an optional automatic fallback to the CPU version when out of GPU memory. Please refer to the Programming Guide for more details.
  *
  * - "MedianBvh": Deprecated in OptiX 4.0. This builder is now internally remapped to Trbvh.
  *
  * - "Lbvh": Deprecated in OptiX 4.0. This builder is now internally remapped to Trbvh.
  *
  * - "TriangleKdTree": Deprecated in OptiX 4.0. This builder is now internally remapped to Trbvh.
  *
  * @param[in]   acceleration   The acceleration structure handle
  * @param[in]   builder        String value specifying the builder type
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationSetBuilder was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationGetBuilder,
  * @ref rtAccelerationSetProperty
  *
  */
  RTresult RTAPI rtAccelerationSetBuilder(RTacceleration acceleration, const char* builder);

  /**
  * @brief Query the current builder from an acceleration structure
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationGetBuilder returns the name of the builder currently
  * used in the acceleration structure \a acceleration. If no builder has
  * been set for \a acceleration, an empty string is returned.
  * \a return_string will be set to point to the returned string. The
  * memory \a return_string points to will be valid until the next API
  * call that returns a string.
  *
  * @param[in]   acceleration    The acceleration structure handle
  * @param[out]  return_string   Return string buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationGetBuilder was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationSetBuilder
  *
  */
  RTresult RTAPI rtAccelerationGetBuilder(RTacceleration acceleration, const char** return_string);

  /**
  * Deprecated in OptiX 4.0. Setting a traverser is no longer necessary and will be ignored.
  *
  */
  RTresult RTAPI rtAccelerationSetTraverser(RTacceleration acceleration, const char* traverser);

  /**
  * Deprecated in OptiX 4.0.
  *
  */
  RTresult RTAPI rtAccelerationGetTraverser(RTacceleration acceleration, const char** return_string);

  /**
  * @brief Sets an acceleration structure property
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationSetProperty sets a named property value for an
  * acceleration structure. Properties can be used to fine tune the way an
  * acceleration structure is built, in order to achieve faster build
  * times or better ray tracing performance.  Properties are evaluated and
  * applied by the acceleration structure during build time, and
  * different builders recognize different properties. Setting a property
  * will never fail as long as \a acceleration is a valid
  * handle. Properties that are not recognized by an acceleration
  * structure will be ignored.
  *
  * The following is a list of the properties used by the individual builders:
  *
  * - "refit":
  * Available in: Trbvh, Bvh
  * If set to "1", the builder will only readjust the node bounds of the bounding
  * volume hierarchy instead of constructing it from scratch. Refit is only
  * effective if there is an initial BVH already in place, and the underlying
  * geometry has undergone relatively modest deformation.  In this case, the
  * builder delivers a very fast BVH update without sacrificing too much ray
  * tracing performance.  The default is "0".
  *
  * - "vertex_buffer_name":
  * Available in: Trbvh, Sbvh
  * The name of the buffer variable holding triangle vertex data.  Each vertex
  * consists of 3 floats.  The default is "vertex_buffer".
  *
  * - "vertex_buffer_stride":
  * Available in: Trbvh, Sbvh
  * The offset between two vertices in the vertex buffer, given in bytes.  The
  * default value is "0", which assumes the vertices are tightly packed.
  *
  * - "index_buffer_name":
  * Available in: Trbvh, Sbvh
  * The name of the buffer variable holding vertex index data. The entries in
  * this buffer are indices of type int, where each index refers to one entry in
  * the vertex buffer. A sequence of three indices represents one triangle. If no
  * index buffer is given, the vertices in the vertex buffer are assumed to be a
  * list of triangles, i.e. every 3 vertices in a row form a triangle.  The
  * default is "index_buffer".
  *
  * - "index_buffer_stride":
  * Available in: Trbvh, Sbvh
  * The offset between two indices in the index buffer, given in bytes.  The
  * default value is "0", which assumes the indices are tightly packed.
  *
  * - "chunk_size":
  * Available in: Trbvh
  * Number of bytes to be used for a partitioned acceleration structure build. If
  * no chunk size is set, or set to "0", the chunk size is chosen automatically.
  * If set to "-1", the chunk size is unlimited. The minimum chunk size is 64MB.
  * Please note that specifying a small chunk size reduces the peak-memory
  * footprint of the Trbvh but can result in slower rendering performance.
  *
  * @param[in]   acceleration   The acceleration structure handle
  * @param[in]   name           String value specifying the name of the property
  * @param[in]   value          String value specifying the value of the property
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationSetProperty was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationGetProperty,
  * @ref rtAccelerationSetBuilder,
  *
  */
  RTresult RTAPI rtAccelerationSetProperty(RTacceleration acceleration, const char* name, const char* value);

  /**
  * @brief Queries an acceleration structure property
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationGetProperty returns the value of the acceleration
  * structure property \a name.  See @ref rtAccelerationSetProperty for a
  * list of supported properties.  If the property name is not found, an
  * empty string is returned.  \a return_string will be set to point to
  * the returned string. The memory \a return_string points to will be
  * valid until the next API call that returns a string.
  *
  * @param[in]   acceleration    The acceleration structure handle
  * @param[in]   name            The name of the property to be queried
  * @param[out]  return_string   Return string buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationGetProperty was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationSetProperty,
  * @ref rtAccelerationSetBuilder,
  *
  */
  RTresult RTAPI rtAccelerationGetProperty(RTacceleration acceleration, const char* name, const char** return_string);

  /**
  * Deprecated in OptiX 4.0. Should not be called.
  *
  */
  RTresult RTAPI rtAccelerationGetDataSize(RTacceleration acceleration, RTsize* size);

  /**
  * Deprecated in OptiX 4.0. Should not be called.
  *
  */
  RTresult RTAPI rtAccelerationGetData(RTacceleration acceleration, void* data);

  /**
  * Deprecated in OptiX 4.0. Should not be called.
  *
  */
  RTresult RTAPI rtAccelerationSetData(RTacceleration acceleration, const void* data, RTsize size);

  /**
  * @brief Marks an acceleration structure as dirty
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationMarkDirty sets the dirty flag for \a acceleration.
  *
  * Any acceleration structure which is marked dirty will be rebuilt on a call to one of the @ref
  * rtContextLaunch "rtContextLaunch" functions, and its dirty flag will be reset.
  *
  * An acceleration structure which is not marked dirty will never be rebuilt, even if associated
  * groups, geometry, properties, or any other values have changed.
  *
  * Initially after creation, acceleration structures are marked dirty.
  *
  * @param[in]   acceleration   The acceleration structure handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationMarkDirty was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationIsDirty,
  * @ref rtContextLaunch
  *
  */
  RTresult RTAPI rtAccelerationMarkDirty(RTacceleration acceleration);

  /**
  * @brief Returns the dirty flag of an acceleration structure
  *
  * @ingroup AccelerationStructure
  *
  * <B>Description</B>
  *
  * @ref rtAccelerationIsDirty returns whether the acceleration structure is currently marked dirty.
  * If the flag is set, a nonzero value will be returned in \a *dirty. Otherwise, zero is returned.
  *
  * Any acceleration structure which is marked dirty will be rebuilt on a call to one of the @ref
  * rtContextLaunch "rtContextLaunch" functions, and its dirty flag will be reset.
  *
  * An acceleration structure which is not marked dirty will never be rebuilt, even if associated
  * groups, geometry, properties, or any other values have changed.
  *
  * Initially after creation, acceleration structures are marked dirty.
  *
  * @param[in]   acceleration   The acceleration structure handle
  * @param[out]  dirty          Returned dirty flag
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtAccelerationIsDirty was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtAccelerationMarkDirty,
  * @ref rtContextLaunch
  *
  */
  RTresult RTAPI rtAccelerationIsDirty(RTacceleration acceleration, int* dirty);

/************************************
 **
 **    GeometryInstance object
 **
 ***********************************/

  /**
  * @brief Creates a new geometry instance node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceCreate creates a new geometry instance node within a context. \a context
  * specifies the target context, and should be a value returned by @ref rtContextCreate.
  * Sets \a *geometryinstance to the handle of a newly created geometry instance within \a context.
  * Returns @ref RT_ERROR_INVALID_VALUE if \a geometryinstance is \a NULL.
  *
  * @param[in]   context            Specifies the rendering context of the GeometryInstance node
  * @param[out]  geometryinstance   New GeometryInstance node handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceDestroy,
  * @ref rtGeometryInstanceDestroy,
  * @ref rtGeometryInstanceGetContext
  *
  */
  RTresult RTAPI rtGeometryInstanceCreate(RTcontext context, RTgeometryinstance* geometryinstance);

  /**
  * @brief Destroys a geometry instance node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceDestroy removes \a geometryinstance from its context and deletes it.  \a
  * geometryinstance should be a value returned by @ref rtGeometryInstanceCreate.  Associated
  * variables declared via @ref rtGeometryInstanceDeclareVariable are destroyed, but no child graph
  * nodes are destroyed.  After the call, \a geometryinstance is no longer a valid handle.
  *
  * @param[in]   geometryinstance   Handle of the geometry instance node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceCreate
  *
  */
  RTresult RTAPI rtGeometryInstanceDestroy(RTgeometryinstance geometryinstance);

  /**
  * @brief Checks a GeometryInstance node for internal consistency
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceValidate checks \a geometryinstance for completeness. If \a geomertryinstance or
  * any of the objects attached to \a geometry are not valid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   geometryinstance   GeometryInstance node of a model sub-tree to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceCreate
  *
  */
  RTresult RTAPI rtGeometryInstanceValidate(RTgeometryinstance geometryinstance);

  /**
  * @brief Returns the context associated with a geometry instance node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceGetContext queries a geometry instance node for its associated context.
  * \a geometryinstance specifies the geometry node to query, and should be a value returned by
  * @ref rtGeometryInstanceCreate. Sets \a *context to the context
  * associated with \a geometryinstance.
  *
  * @param[in]   geometryinstance   Specifies the geometry instance
  * @param[out]  context            Handle for queried context
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceGetContext
  *
  */
  RTresult RTAPI rtGeometryInstanceGetContext(RTgeometryinstance geometryinstance, RTcontext* context);

  /**
  * @brief Attaches a Geometry node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceSetGeometry attaches a Geometry node to a GeometryInstance.
  * Only \a one Geometry node can be attached to a GeometryInstance. However, it is
  * at any time possible to attach a different Geometry node.
  *
  * @param[in]   geometryinstance   GeometryInstance node handle to attach geometry
  * @param[in]   geometry           Geometry handle to attach to \a geometryinstance
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceSetGeometry was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceGetGeometry
  *
  */
  RTresult RTAPI rtGeometryInstanceSetGeometry(RTgeometryinstance geometryinstance, RTgeometry geometry);

  /**
  * @brief Returns the attached Geometry node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceGetGeometry sets \a geometry to the handle of the attached Geometry node.
  * If no Geometry node is attached, @ref RT_ERROR_INVALID_VALUE is returned, else @ref RT_SUCCESS.
  *
  * @param[in]   geometryinstance   GeometryInstance node handle to query geometry
  * @param[out]  geometry           Handle to attached Geometry node
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceGetGeometry was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceCreate,
  * @ref rtGeometryInstanceDestroy,
  * @ref rtGeometryInstanceValidate,
  * @ref rtGeometryInstanceSetGeometry
  *
  */
  RTresult RTAPI rtGeometryInstanceGetGeometry(RTgeometryinstance geometryinstance, RTgeometry* geometry);

  /**
  * @brief Sets the number of materials
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceSetMaterialCount sets the number of materials \a count that will be
  * attached to \a geometryinstance. The number of attached materials can be changed at any
  * time.  Increasing the number of materials will not modify already assigned materials.
  * Decreasing the number of materials will not modify the remaining already assigned
  * materials.
  *
  * @param[in]   geometryinstance   GeometryInstance node to set number of materials
  * @param[in]   count              Number of materials to be set
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceSetMaterialCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceGetMaterialCount
  *
  */
  RTresult RTAPI rtGeometryInstanceSetMaterialCount(RTgeometryinstance geometryinstance, unsigned int count);

  /**
  * @brief Returns the number of attached materials
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceGetMaterialCount returns for \a geometryinstance the number of attached
  * Material nodes \a count. The number of materies can be set with @ref
  * rtGeometryInstanceSetMaterialCount.
  *
  * @param[in]   geometryinstance   GeometryInstance node to query from the number of materials
  * @param[out]  count              Number of attached materials
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceGetMaterialCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceSetMaterialCount
  *
  */
  RTresult RTAPI rtGeometryInstanceGetMaterialCount(RTgeometryinstance geometryinstance, unsigned int* count);

  /**
  * @brief Sets a material
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceSetMaterial attaches \a material to \a geometryinstance at position \a index
  * in its internal Material node list.  \a index must be in the range \a 0 to @ref
  * rtGeometryInstanceGetMaterialCount \a - 1.
  *
  * @param[in]   geometryinstance   GeometryInstance node for which to set a material
  * @param[in]   index              Index into the material list
  * @param[in]   material           Material handle to attach to \a geometryinstance
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceSetMaterial was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceGetMaterialCount,
  * @ref rtGeometryInstanceSetMaterialCount
  *
  */
  RTresult RTAPI rtGeometryInstanceSetMaterial(RTgeometryinstance geometryinstance, unsigned int index, RTmaterial material);

  /**
  * @brief Returns a material handle
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceGetMaterial returns handle \a material for the Material node at position
  * \a index in the material list of \a geometryinstance. Returns @ref RT_ERROR_INVALID_VALUE if \a
  * index is invalid.
  *
  * @param[in]   geometryinstance   GeometryInstance node handle to query material
  * @param[in]   index              Index of material
  * @param[out]  material           Handle to material
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceGetMaterial was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceGetMaterialCount,
  * @ref rtGeometryInstanceSetMaterial
  *
  */
  RTresult RTAPI rtGeometryInstanceGetMaterial(RTgeometryinstance geometryinstance, unsigned int index, RTmaterial* material);

  /**
  * @brief Declares a new named variable associated with a geometry node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceDeclareVariable declares a new variable associated with a geometry
  * instance node. \a geometryinstance specifies the target geometry node, and should be a value
  * returned by @ref rtGeometryInstanceCreate. \a name specifies the name of the variable, and
  * should be a \a NULL-terminated string. If there is currently no variable associated with \a
  * geometryinstance named \a name, a new variable named \a name will be created and associated with
  * \a geometryinstance.  After the call, \a *v will be set to the handle of the newly-created
  * variable.  Otherwise, \a *v will be set to \a NULL. After declaration, the variable can be
  * queried with @ref rtGeometryInstanceQueryVariable or @ref rtGeometryInstanceGetVariable. A
  * declared variable does not have a type until its value is set with one of the @ref rtVariableSet
  * functions. Once a variable is set, its type cannot be changed anymore.
  *
  * @param[in]   geometryinstance   Specifies the associated GeometryInstance node
  * @param[in]   name               The name that identifies the variable
  * @param[out]  v                  Returns a handle to a newly declared variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceDeclareVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref Variables,
  * @ref rtGeometryInstanceQueryVariable,
  * @ref rtGeometryInstanceGetVariable,
  * @ref rtGeometryInstanceRemoveVariable
  *
  */
  RTresult RTAPI rtGeometryInstanceDeclareVariable(RTgeometryinstance geometryinstance, const char* name, RTvariable* v);

  /**
  * @brief Returns a handle to a named variable of a geometry node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceQueryVariable queries the handle of a geometry instance node's named
  * variable.  \a geometryinstance specifies the target geometry instance node, as returned by
  * @ref rtGeometryInstanceCreate. \a name specifies the name of the variable, and should be a \a
  * \a NULL -terminated string. If \a name is the name of a variable attached to \a geometryinstance,
  * returns a handle to that variable in \a *v, otherwise \a NULL.  Geometry instance variables have
  * to be declared with @ref rtGeometryInstanceDeclareVariable before they can be queried.
  *
  * @param[in]   geometryinstance   The GeometryInstance node to query from a variable
  * @param[in]   name               The name that identifies the variable to be queried
  * @param[out]  v                  Returns the named variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceQueryVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceDeclareVariable,
  * @ref rtGeometryInstanceRemoveVariable,
  * @ref rtGeometryInstanceGetVariableCount,
  * @ref rtGeometryInstanceGetVariable
  *
  */
  RTresult RTAPI rtGeometryInstanceQueryVariable(RTgeometryinstance geometryinstance, const char* name, RTvariable* v);

  /**
  * @brief Removes a named variable from a geometry instance node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceRemoveVariable removes a named variable from a geometry instance. The
  * target geometry instance is specified by \a geometryinstance, which should be a value returned
  * by @ref rtGeometryInstanceCreate. The variable to be removed is specified by \a v, which should
  * be a value returned by @ref rtGeometryInstanceDeclareVariable. Once a variable has been removed
  * from this geometry instance, another variable with the same name as the removed variable may be
  * declared.
  *
  * @param[in]   geometryinstance   The GeometryInstance node from which to remove a variable
  * @param[in]   v                  The variable to be removed
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceRemoveVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextRemoveVariable,
  * @ref rtGeometryInstanceDeclareVariable
  *
  */
  RTresult RTAPI rtGeometryInstanceRemoveVariable(RTgeometryinstance geometryinstance, RTvariable v);

  /**
  * @brief Returns the number of attached variables
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceGetVariableCount queries the number of variables attached to a geometry instance.
  * \a geometryinstance specifies the geometry instance, and should be a value returned by @ref rtGeometryInstanceCreate.
  * After the call, the number of variables attached to \a geometryinstance is returned to \a *count.
  *
  * @param[in]   geometryinstance   The GeometryInstance node to query from the number of attached variables
  * @param[out]  count              Returns the number of attached variables
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceGetVariableCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryInstanceGetVariableCount,
  * @ref rtGeometryInstanceDeclareVariable,
  * @ref rtGeometryInstanceRemoveVariable
  *
  */
  RTresult RTAPI rtGeometryInstanceGetVariableCount(RTgeometryinstance geometryinstance, unsigned int* count);

  /**
  * @brief Returns a handle to an indexed variable of a geometry instance node
  *
  * @ingroup GeometryInstance
  *
  * <B>Description</B>
  *
  * @ref rtGeometryInstanceGetVariable queries the handle of a geometry instance's indexed variable.
  * \a geometryinstance specifies the target geometry instance and should be a value returned by
  * @ref rtGeometryInstanceCreate. \a index specifies the index of the variable, and should be a
  * value less than @ref rtGeometryInstanceGetVariableCount. If \a index is the index of a variable
  * attached to \a geometryinstance, returns a handle to that variable in \a *v, and \a NULL
  * otherwise. \a *v must be declared first with @ref rtGeometryInstanceDeclareVariable before it
  * can be queried.
  *
  * @param[in]   geometryinstance   The GeometryInstance node from which to query a variable
  * @param[in]   index              The index that identifies the variable to be queried
  * @param[out]  v                  Returns handle to indexed variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtGeometryInstanceGetVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryDeclareVariable,
  * @ref rtGeometryGetVariableCount,
  * @ref rtGeometryRemoveVariable,
  * @ref rtGeometryQueryVariable
  *
  */
  RTresult RTAPI rtGeometryInstanceGetVariable(RTgeometryinstance geometryinstance, unsigned int index, RTvariable* v);

/************************************
 **
 **    Geometry object
 **
 ***********************************/

  /**
  * @brief Creates a new geometry node
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryCreate creates a new geometry node within a context. \a context
  * specifies the target context, and should be a value returned by @ref rtContextCreate.
  * Sets \a *geometry to the handle of a newly created geometry within \a context.
  * Returns @ref RT_ERROR_INVALID_VALUE if \a geometry is \a NULL.
  *
  * @param[in]   context    Specifies the rendering context of the Geometry node
  * @param[out]  geometry   New Geometry node handle
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryDestroy,
  * @ref rtGeometrySetBoundingBoxProgram,
  * @ref rtGeometrySetIntersectionProgram
  *
  */
  RTresult RTAPI rtGeometryCreate(RTcontext context, RTgeometry* geometry);

  /**
  * @brief Destroys a geometry node
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryDestroy removes \a geometry from its context and deletes it.  \a geometry should
  * be a value returned by @ref rtGeometryCreate.  Associated variables declared via
  * @ref rtGeometryDeclareVariable are destroyed, but no child graph nodes are destroyed.  After the
  * call, \a geometry is no longer a valid handle.
  *
  * @param[in]   geometry   Handle of the geometry node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryCreate,
  * @ref rtGeometrySetPrimitiveCount,
  * @ref rtGeometryGetPrimitiveCount
  *
  */
  RTresult RTAPI rtGeometryDestroy(RTgeometry geometry);

  /**
  * @brief Validates the geometry nodes integrity
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryValidate checks \a geometry for completeness. If \a geometry or any of the
  * objects attached to \a geometry are not valid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   geometry   The geometry node to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextValidate
  *
  */
  RTresult RTAPI rtGeometryValidate(RTgeometry geometry);

  /**
  * @brief Returns the context associated with a geometry node
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetContext queries a geometry node for its associated context.  \a geometry
  * specifies the geometry node to query, and should be a value returned by @ref
  * rtGeometryCreate. Sets \a *context to the context associated with \a geometry.
  *
  * @param[in]   geometry   Specifies the geometry to query
  * @param[out]  context    The context associated with \a geometry
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryCreate
  *
  */
  RTresult RTAPI rtGeometryGetContext(RTgeometry geometry, RTcontext* context);

  /**
  * @brief Sets the number of primitives
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometrySetPrimitiveCount sets the number of primitives \a num_primitives in \a geometry.
  *
  * @param[in]   geometry         The geometry node for which to set the number of primitives
  * @param[in]   num_primitives   The number of primitives
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometrySetPrimitiveCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGetPrimitiveCount
  *
  */
  RTresult RTAPI rtGeometrySetPrimitiveCount(RTgeometry geometry, unsigned int num_primitives);

  /**
  * @brief Returns the number of primitives
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetPrimitiveCount returns for \a geometry the number of set primitives. The
  * number of primitvies can be set with @ref rtGeometryGetPrimitiveCount.
  *
  * @param[in]   geometry         Geometry node to query from the number of primitives
  * @param[out]  num_primitives   Number of primitives
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetPrimitiveCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometrySetPrimitiveCount
  *
  */
  RTresult RTAPI rtGeometryGetPrimitiveCount(RTgeometry geometry, unsigned int* num_primitives);

  /**
  * @brief Sets the primitive index offset
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometrySetPrimitiveIndexOffset sets the primitive index offset
  * \a index_offset in \a geometry.  In the past, a @ref Geometry object's primitive
  * index range always started at zero (e.g., a Geometry with \a N primitives would
  * have a primitive index range of [0,N-1]).  The index offset is used to allow
  * @ref Geometry objects to have primitive index ranges starting at non-zero
  * positions (e.g., a Geometry with \a N primtives and and index offset of \a M
  * would have a primitive index range of [M,M+N-1]).  This feature enables the
  * sharing of vertex index buffers between multiple @ref Geometry objects.
  *
  * @param[in]   geometry       The geometry node for which to set the primitive index offset
  * @param[in]   index_offset   The primitive index offset
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGeometrySetPrimitiveIndexOffset was introduced in OptiX 3.5.
  *
  * <B>See also</B>
  * @ref rtGeometryGetPrimitiveIndexOffset
  *
  */
  RTresult RTAPI rtGeometrySetPrimitiveIndexOffset(RTgeometry geometry, unsigned int index_offset);

  /**
  * @brief Returns the current primitive index offset
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetPrimitiveIndexOffset returns for \a geometry the primitive index offset. The
  * primitive index offset can be set with @ref rtGeometrySetPrimitiveIndexOffset.
  *
  * @param[in]   geometry       Geometry node to query for the primitive index offset
  * @param[out]  index_offset   Primitive index offset
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetPrimitiveIndexOffset was introduced in OptiX 3.5.
  *
  * <B>See also</B>
  * @ref rtGeometrySetPrimitiveIndexOffset
  *
  */
  RTresult RTAPI rtGeometryGetPrimitiveIndexOffset(RTgeometry geometry, unsigned int* index_offset);

  /**
  * @brief Sets the bounding box program
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometrySetBoundingBoxProgram sets for \a geometry the \a program that computes an axis aligned bounding box
  * for each attached primitive to \a geometry. RTprogram's can be either generated with @ref rtProgramCreateFromPTXFile or
  * @ref rtProgramCreateFromPTXString. A bounding box program is mandatory for every geometry node.
  *
  * @param[in]   geometry   The geometry node for which to set the bounding box program
  * @param[in]   program    Handle to the bounding box program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtGeometrySetBoundingBoxProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGetBoundingBoxProgram,
  * @ref rtProgramCreateFromPTXFile,
  * @ref rtProgramCreateFromPTXString
  *
  */
  RTresult RTAPI rtGeometrySetBoundingBoxProgram(RTgeometry geometry, RTprogram program);

  /**
  * @brief Returns the attached bounding box program
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetBoundingBoxProgram returns the handle \a program for
  * the attached bounding box program of \a geometry.
  *
  * @param[in]   geometry   Geometry node handle from which to query program
  * @param[out]  program    Handle to attached bounding box program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetBoundingBoxProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometrySetBoundingBoxProgram
  *
  */
  RTresult RTAPI rtGeometryGetBoundingBoxProgram(RTgeometry geometry, RTprogram* program);

  /**
  * @brief Sets the intersection program
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometrySetIntersectionProgram sets for \a geometry the \a program that performs ray primitive intersections.
  * RTprogram's can be either generated with @ref rtProgramCreateFromPTXFile or @ref rtProgramCreateFromPTXString. An intersection
  * program is mandatory for every geometry node.
  *
  * @param[in]   geometry   The geometry node for which to set the intersection program
  * @param[in]   program    A handle to the ray primitive intersection program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtGeometrySetIntersectionProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGetIntersectionProgram,
  * @ref rtProgramCreateFromPTXFile,
  * @ref rtProgramCreateFromPTXString
  *
  */
  RTresult RTAPI rtGeometrySetIntersectionProgram(RTgeometry geometry, RTprogram program);

  /**
  * @brief Returns the attached intersection program
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetIntersectionProgram returns in \a program a handle of the attached intersection program.
  *
  * @param[in]   geometry   Geometry node handle to query program
  * @param[out]  program    Handle to attached intersection program
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetIntersectionProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometrySetIntersectionProgram,
  * @ref rtProgramCreateFromPTXFile,
  * @ref rtProgramCreateFromPTXString
  *
  */
  RTresult RTAPI rtGeometryGetIntersectionProgram(RTgeometry geometry, RTprogram* program);

  /**
  * Deprecated in OptiX 4.0. Calling this function has no effect.
  *
  */
  RTresult RTAPI rtGeometryMarkDirty(RTgeometry geometry);

  /**
  * Deprecated in OptiX 4.0. Calling this function has no effect.
  *
  */
  RTresult RTAPI rtGeometryIsDirty(RTgeometry geometry, int* dirty);

  /**
  * @brief Declares a new named variable associated with a geometry instance
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryDeclareVariable declares a new variable associated with a geometry node. \a
  * geometry specifies the target geometry node, and should be a value returned by @ref
  * rtGeometryCreate. \a name specifies the name of the variable, and should be a \a NULL-terminated
  * string. If there is currently no variable associated with \a geometry named \a name, a new
  * variable named \a name will be created and associated with \a geometry.  Returns the handle of
  * the newly-created variable in \a *v or \a NULL otherwise.  After declaration, the variable can
  * be queried with @ref rtGeometryQueryVariable or @ref rtGeometryGetVariable. A declared variable
  * does not have a type until its value is set with one of the @ref rtVariableSet functions. Once a
  * variable is set, its type cannot be changed anymore.
  *
  * @param[in]   geometry   Specifies the associated Geometry node
  * @param[in]   name       The name that identifies the variable
  * @param[out]  v          Returns a handle to a newly declared variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_REDECLARED
  * - @ref RT_ERROR_ILLEGAL_SYMBOL
  *
  * <B>History</B>
  *
  * @ref rtGeometryDeclareVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref Variables,
  * @ref rtGeometryQueryVariable,
  * @ref rtGeometryGetVariable,
  * @ref rtGeometryRemoveVariable
  *
  */
  RTresult RTAPI rtGeometryDeclareVariable(RTgeometry geometry, const char* name, RTvariable* v);

  /**
  * @brief Returns a handle to a named variable of a geometry node
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryQueryVariable queries the handle of a geometry node's named variable.
  * \a geometry specifies the target geometry node and should be a value returned
  * by @ref rtGeometryCreate. \a name specifies the name of the variable, and should
  * be a \a NULL-terminated string. If \a name is the name of a variable attached to
  * \a geometry, returns a handle to that variable in \a *v or \a NULL otherwise. Geometry
  * variables must be declared with @ref rtGeometryDeclareVariable before they can be queried.
  *
  * @param[in]   geometry   The geometry node to query from a variable
  * @param[in]   name       The name that identifies the variable to be queried
  * @param[out]  v          Returns the named variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtGeometryQueryVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryDeclareVariable,
  * @ref rtGeometryRemoveVariable,
  * @ref rtGeometryGetVariableCount,
  * @ref rtGeometryGetVariable
  *
  */
  RTresult RTAPI rtGeometryQueryVariable(RTgeometry geometry, const char* name, RTvariable* v);

  /**
  * @brief Removes a named variable from a geometry node
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryRemoveVariable removes a named variable from a geometry node. The
  * target geometry is specified by \a geometry, which should be a value
  * returned by @ref rtGeometryCreate. The variable to remove is specified by
  * \a v, which should be a value returned by @ref rtGeometryDeclareVariable.
  * Once a variable has been removed from this geometry node, another variable with the
  * same name as the removed variable may be declared.
  *
  * @param[in]   geometry   The geometry node from which to remove a variable
  * @param[in]   v          The variable to be removed
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtGeometryRemoveVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextRemoveVariable
  *
  */
  RTresult RTAPI rtGeometryRemoveVariable(RTgeometry geometry, RTvariable v);

  /**
  * @brief Returns the number of attached variables
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetVariableCount queries the number of variables attached to a geometry node.
  * \a geometry specifies the geometry node, and should be a value returned by @ref rtGeometryCreate.
  * After the call, the number of variables attached to \a geometry is returned to \a *count.
  *
  * @param[in]   geometry   The Geometry node to query from the number of attached variables
  * @param[out]  count      Returns the number of attached variables
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetVariableCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryGetVariableCount,
  * @ref rtGeometryDeclareVariable,
  * @ref rtGeometryRemoveVariable
  *
  */
  RTresult RTAPI rtGeometryGetVariableCount(RTgeometry geometry, unsigned int* count);

  /**
  * @brief Returns a handle to an indexed variable of a geometry node
  *
  * @ingroup Geometry
  *
  * <B>Description</B>
  *
  * @ref rtGeometryGetVariable queries the handle of a geometry node's indexed variable.
  * \a geometry specifies the target geometry and should be a value returned
  * by @ref rtGeometryCreate. \a index specifies the index of the variable, and
  * should be a value less than @ref rtGeometryGetVariableCount. If \a index is the
  * index of a variable attached to \a geometry, returns its handle in \a *v or \a NULL otherwise.
  * \a *v must be declared first with @ref rtGeometryDeclareVariable before it can be queried.
  *
  * @param[in]   geometry   The geometry node from which to query a variable
  * @param[in]   index      The index that identifies the variable to be queried
  * @param[out]  v          Returns handle to indexed variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtGeometryGetVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtGeometryDeclareVariable,
  * @ref rtGeometryGetVariableCount,
  * @ref rtGeometryRemoveVariable,
  * @ref rtGeometryQueryVariable
  *
  */
  RTresult RTAPI rtGeometryGetVariable(RTgeometry geometry, unsigned int index, RTvariable* v);

/************************************
 **
 **    Material object
 **
 ***********************************/

  /**
  * @brief Creates a new material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialCreate creates a new material within a context. \a context specifies the target
  * context, as returned by @ref rtContextCreate. Sets \a *material to the handle of a newly
  * created material within \a context. Returns @ref RT_ERROR_INVALID_VALUE if \a material is \a NULL.
  *
  * @param[in]   context    Specifies a context within which to create a new material
  * @param[out]  material   Returns a newly created material
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtMaterialCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialDestroy,
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtMaterialCreate(RTcontext context, RTmaterial* material);

  /**
  * @brief Destroys a material object
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialDestroy removes \a material from its context and deletes it.  \a material should
  * be a value returned by @ref rtMaterialCreate.  Associated variables declared via @ref
  * rtMaterialDeclareVariable are destroyed, but no child graph nodes are destroyed.  After the
  * call, \a material is no longer a valid handle.
  *
  * @param[in]   material   Handle of the material node to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtMaterialDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialDestroy(RTmaterial material);

  /**
  * @brief Verifies the state of a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialValidate checks \a material for completeness. If \a material or
  * any of the objects attached to \a material are not valid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   material   Specifies the material to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtMaterialValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialValidate(RTmaterial material);

  /**
  * @brief Returns the context associated with a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialGetContext queries a material for its associated context.
  * \a material specifies the material to query, and should be a value returned by
  * @ref rtMaterialCreate. If both parameters are valid, \a *context
  * sets to the context associated with \a material. Otherwise, the call
  * has no effect and returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   material   Specifies the material to query
  * @param[out]  context    Returns the context associated with the material
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtMaterialGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialGetContext(RTmaterial material, RTcontext* context);

  /**
  * @brief Sets the closest hit program associated with a (material, ray type) tuple
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialSetClosestHitProgram specifies a closest hit program to associate
  * with a (material, ray type) tuple. \a material specifies the material of
  * interest and should be a value returned by @ref rtMaterialCreate.
  * \a ray_type_index specifies the type of ray to which the program applies and
  * should be a value less than the value returned by @ref rtContextGetRayTypeCount.
  * \a program specifies the target closest hit program which applies to
  * the tuple (\a material, \a ray_type_index) and should be a value returned by
  * either @ref rtProgramCreateFromPTXString or @ref rtProgramCreateFromPTXFile.
  *
  * @param[in]   material         Specifies the material of the (material, ray type) tuple to modify
  * @param[in]   ray_type_index   Specifies the ray type of the (material, ray type) tuple to modify
  * @param[in]   program          Specifies the closest hit program to associate with the (material, ray type) tuple
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtMaterialSetClosestHitProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialGetClosestHitProgram,
  * @ref rtMaterialCreate,
  * @ref rtContextGetRayTypeCount,
  * @ref rtProgramCreateFromPTXString,
  * @ref rtProgramCreateFromPTXFile
  *
  */
  RTresult RTAPI rtMaterialSetClosestHitProgram(RTmaterial material, unsigned int ray_type_index, RTprogram program);

  /**
  * @brief Returns the closest hit program associated with a (material, ray type) tuple
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialGetClosestHitProgram queries the closest hit program associated
  * with a (material, ray type) tuple. \a material specifies the material of
  * interest and should be a value returned by @ref rtMaterialCreate.
  * \a ray_type_index specifies the target ray type and should be a value
  * less than the value returned by @ref rtContextGetRayTypeCount.
  * If all parameters are valid, \a *program sets to the handle of the
  * any hit program associated with the tuple (\a material, \a ray_type_index).
  * Otherwise, the call has no effect and returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   material         Specifies the material of the (material, ray type) tuple to query
  * @param[in]   ray_type_index   Specifies the type of ray of the (material, ray type) tuple to query
  * @param[out]  program          Returns the closest hit program associated with the (material, ray type) tuple
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtMaterialGetClosestHitProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialSetClosestHitProgram,
  * @ref rtMaterialCreate,
  * @ref rtContextGetRayTypeCount
  *
  */
  RTresult RTAPI rtMaterialGetClosestHitProgram(RTmaterial material, unsigned int ray_type_index, RTprogram* program);

  /**
  * @brief Sets the any hit program associated with a (material, ray type) tuple
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialSetAnyHitProgram specifies an any hit program to associate with a
  * (material, ray type) tuple. \a material specifies the target material and
  * should be a value returned by @ref rtMaterialCreate. \a ray_type_index specifies
  * the type of ray to which the program applies and should be a value less than
  * the value returned by @ref rtContextGetRayTypeCount. \a program specifies the
  * target any hit program which applies to the tuple (\a material,
  * \a ray_type_index) and should be a value returned by either
  * @ref rtProgramCreateFromPTXString or @ref rtProgramCreateFromPTXFile.
  *
  * @param[in]   material         Specifies the material of the (material, ray type) tuple to modify
  * @param[in]   ray_type_index   Specifies the type of ray of the (material, ray type) tuple to modify
  * @param[in]   program          Specifies the any hit program to associate with the (material, ray type) tuple
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtMaterialSetAnyHitProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialGetAnyHitProgram,
  * @ref rtMaterialCreate,
  * @ref rtContextGetRayTypeCount,
  * @ref rtProgramCreateFromPTXString,
  * @ref rtProgramCreateFromPTXFile
  *
  */
  RTresult RTAPI rtMaterialSetAnyHitProgram(RTmaterial material, unsigned int ray_type_index, RTprogram program);

  /**
  * @brief Returns the any hit program associated with a (material, ray type) tuple
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialGetAnyHitProgram queries the any hit program associated
  * with a (material, ray type) tuple. \a material specifies the material of
  * interest and should be a value returned by @ref rtMaterialCreate.
  * \a ray_type_index specifies the target ray type and should be a value
  * less than the value returned by @ref rtContextGetRayTypeCount.
  * if all parameters are valid, \a *program sets to the handle of the
  * any hit program associated with the tuple (\a material, \a ray_type_index).
  * Otherwise, the call has no effect and returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   material         Specifies the material of the (material, ray type) tuple to query
  * @param[in]   ray_type_index   Specifies the type of ray of the (material, ray type) tuple to query
  * @param[out]  program          Returns the any hit program associated with the (material, ray type) tuple
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtMaterialGetAnyHitProgram was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialSetAnyHitProgram,
  * @ref rtMaterialCreate,
  * @ref rtContextGetRayTypeCount
  *
  */
  RTresult RTAPI rtMaterialGetAnyHitProgram(RTmaterial material, unsigned int ray_type_index, RTprogram* program);

  /**
  * @brief Declares a new named variable to be associated with a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialDeclareVariable declares a new variable to be associated with a material.
  * \a material specifies the target material, and should be a value returned by @ref
  * rtMaterialCreate. \a name specifies the name of the variable, and should be a \a NULL-terminated
  * string. If there is currently no variable associated with \a material named \a name, and \a v is
  * not \a NULL, a new variable named \a name will be created and associated with \a material and \a
  * *v will be set to the handle of the newly-created variable. Otherwise, this call has no effect
  * and returns either @ref RT_ERROR_INVALID_VALUE if either \a name or \a v is \a NULL or @ref
  * RT_ERROR_VARIABLE_REDECLARED if \a name is the name of an existing variable associated with the
  * material.
  *
  * @param[in]   material   Specifies the material to modify
  * @param[in]   name       Specifies the name of the variable
  * @param[out]  v          Returns a handle to a newly declared variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_REDECLARED
  * - @ref RT_ERROR_ILLEGAL_SYMBOL
  *
  * <B>History</B>
  *
  * @ref rtMaterialDeclareVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialGetVariable,
  * @ref rtMaterialQueryVariable,
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialDeclareVariable(RTmaterial material, const char* name, RTvariable* v);

  /**
  * @brief Queries for the existence of a named variable of a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialQueryVariable queries for the existence of a material's named variable. \a
  * material specifies the target material and should be a value returned by @ref rtMaterialCreate.
  * \a name specifies the name of the variable, and should be a \a NULL-terminated
  * string. If \a material is a valid material and \a name is the name of a variable attached to \a
  * material, \a *v is set to a handle to that variable after the call. Otherwise, \a *v is set to
  * \a NULL. If \a material is not a valid material, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   material   Specifies the material to query
  * @param[in]   name       Specifies the name of the variable to query
  * @param[out]  v          Returns a the named variable, if it exists
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtMaterialQueryVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialGetVariable,
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialQueryVariable(RTmaterial material, const char* name, RTvariable* v);

  /**
  * @brief Removes a variable from a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialRemoveVariable removes a variable from a material. The material of
  * interest is specified by \a material, which should be a value returned by
  * @ref rtMaterialCreate. The variable to remove is specified by \a v, which
  * should be a value returned by @ref rtMaterialDeclareVariable. Once a variable
  * has been removed from this material, another variable with the same name as the
  * removed variable may be declared. If \a material does not refer to a valid material,
  * this call has no effect and returns @ref RT_ERROR_INVALID_VALUE. If \a v is not
  * a valid variable or does not belong to \a material, this call has no effect and
  * returns @ref RT_ERROR_INVALID_VALUE or @ref RT_ERROR_VARIABLE_NOT_FOUND, respectively.
  *
  * @param[in]   material   Specifies the material to modify
  * @param[in]   v          Specifies the variable to remove
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtMaterialRemoveVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialDeclareVariable,
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialRemoveVariable(RTmaterial material, RTvariable v);

  /**
  * @brief Returns the number of variables attached to a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialGetVariableCount queries the number of variables attached to a
  * material. \a material specifies the material, and should be a value returned by
  * @ref rtMaterialCreate. After the call, if both parameters are valid, the number
  * of variables attached to \a material is returned to \a *count. Otherwise, the
  * call has no effect and returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   material   Specifies the material to query
  * @param[out]  count      Returns the number of variables
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtMaterialGetVariableCount was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialGetVariableCount(RTmaterial material, unsigned int* count);

  /**
  * @brief Returns a handle to an indexed variable of a material
  *
  * @ingroup Material
  *
  * <B>Description</B>
  *
  * @ref rtMaterialGetVariable queries the handle of a material's indexed variable.  \a material
  * specifies the target material and should be a value returned by @ref rtMaterialCreate. \a index
  * specifies the index of the variable, and should be a value less than
  * @ref rtMaterialGetVariableCount. If \a material is a valid material and \a index is the index of a
  * variable attached to \a material, \a *v is set to a handle to that variable. Otherwise, \a *v is
  * set to \a NULL and either @ref RT_ERROR_INVALID_VALUE or @ref RT_ERROR_VARIABLE_NOT_FOUND is
  * returned depending on the validity of \a material, or \a index, respectively.
  *
  * @param[in]   material   Specifies the material to query
  * @param[in]   index      Specifies the index of the variable to query
  * @param[out]  v          Returns the indexed variable
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_VARIABLE_NOT_FOUND
  *
  * <B>History</B>
  *
  * @ref rtMaterialGetVariable was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtMaterialQueryVariable,
  * @ref rtMaterialGetVariableCount,
  * @ref rtMaterialCreate
  *
  */
  RTresult RTAPI rtMaterialGetVariable(RTmaterial material, unsigned int index, RTvariable* v);

/************************************
 **
 **    TextureSampler object
 **
 ***********************************/

  /**
  * @brief Creates a new texture sampler object
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerCreate allocates a texture sampler object.
  * Sets \a *texturesampler to the handle of a newly created texture sampler within \a context.
  * Returns @ref RT_ERROR_INVALID_VALUE if \a texturesampler is \a NULL.
  *
  * @param[in]   context          The context the texture sampler object will be created in
  * @param[out]  texturesampler   The return handle to the new texture sampler object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerCreate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerDestroy
  *
  */
  RTresult RTAPI rtTextureSamplerCreate(RTcontext context, RTtexturesampler* texturesampler);

  /**
  * @brief Destroys a texture sampler object
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerDestroy removes \a texturesampler from its context and deletes it.
  * \a texturesampler should be a value returned by @ref rtTextureSamplerCreate.
  * After the call, \a texturesampler is no longer a valid handle.
  * Any API object that referenced \a texturesampler will have its reference invalidated.
  *
  * @param[in]   texturesampler   Handle of the texture sampler to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerCreate
  *
  */
  RTresult RTAPI rtTextureSamplerDestroy(RTtexturesampler texturesampler);

  /**
  * @brief Validates the state of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerValidate checks \a texturesampler for completeness.  If \a texturesampler does not have buffers
  * attached to all of its MIP levels and array slices or if the filtering modes are incompatible with the current
  * MIP level and array slice configuration then returns @ref RT_ERROR_INVALID_CONTEXT.
  *
  * @param[in]   texturesampler   The texture sampler to be validated
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextValidate
  *
  */
  RTresult RTAPI rtTextureSamplerValidate(RTtexturesampler texturesampler);

  /**
  * @brief Gets the context object that created this texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetContext returns a handle to the context object that was used to create
  * \a texturesampler.  If \a context is \a NULL, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried for its context
  * @param[out]  context          The return handle for the context object of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtTextureSamplerGetContext(RTtexturesampler texturesampler, RTcontext* context);

  /**
  * Deprecated in OptiX 3.9. Use @ref rtBufferSetMipLevelCount instead.
  *
  */
  RTresult RTAPI rtTextureSamplerSetMipLevelCount(RTtexturesampler texturesampler, unsigned int num_mip_levels);

  /**
  * Deprecated in OptiX 3.9. Use @ref rtBufferGetMipLevelCount instead.
  *
  */
  RTresult RTAPI rtTextureSamplerGetMipLevelCount(RTtexturesampler texturesampler, unsigned int* num_mip_levels);

  /**
  * Deprecated in OptiX 3.9. Use texture samplers with layered buffers instead. See @ref rtBufferCreate.
  *
  */
  RTresult RTAPI rtTextureSamplerSetArraySize(RTtexturesampler texturesampler, unsigned int num_textures_in_array);

  /**
  * Deprecated in OptiX 3.9. Use texture samplers with layered buffers instead. See @ref rtBufferCreate.
  *
  */
  RTresult RTAPI rtTextureSamplerGetArraySize(RTtexturesampler texturesampler, unsigned int* num_textures_in_array);

  /**
  * @brief Sets the wrapping mode of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetWrapMode sets the wrapping mode of
  * \a texturesampler to \a wrapmode for the texture dimension specified
  * by \a dimension.  \a wrapmode can take one of the following values:
  *
  *  - @ref RT_WRAP_REPEAT
  *  - @ref RT_WRAP_CLAMP_TO_EDGE
  *  - @ref RT_WRAP_MIRROR
  *  - @ref RT_WRAP_CLAMP_TO_BORDER
  *
  * The wrapping mode controls the behavior of the texture sampler as
  * texture coordinates wrap around the range specified by the indexing
  * mode.  These values mirror the CUDA behavior of textures.
  * See CUDA programming guide for details.
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   dimension        Dimension of the texture
  * @param[in]   wrapmode         The new wrap mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetWrapMode was introduced in OptiX 1.0.
  * @ref RT_WRAP_MIRROR and @ref RT_WRAP_CLAMP_TO_BORDER were introduced in OptiX 3.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetWrapMode
  *
  */
  RTresult RTAPI rtTextureSamplerSetWrapMode(RTtexturesampler texturesampler, unsigned int dimension, RTwrapmode wrapmode);

  /**
  * @brief Gets the wrap mode of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetWrapMode gets the texture wrapping mode of \a texturesampler and stores it in \a *wrapmode.
  * See @ref rtTextureSamplerSetWrapMode for a list of values @ref RTwrapmode can take.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[in]   dimension        Dimension for the wrapping
  * @param[out]  wrapmode         The return handle for the wrap mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetWrapMode was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetWrapMode
  *
  */
  RTresult RTAPI rtTextureSamplerGetWrapMode(RTtexturesampler texturesampler, unsigned int dimension, RTwrapmode* wrapmode);

  /**
  * @brief Sets the filtering modes of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetFilteringModes sets the minification, magnification and MIP mapping filter modes for \a texturesampler.
  * RTfiltermode must be one of the following values:
  *
  *  - @ref RT_FILTER_NEAREST
  *  - @ref RT_FILTER_LINEAR
  *  - @ref RT_FILTER_NONE
  *
  * These filter modes specify how the texture sampler will interpolate
  * buffer data that has been attached to it.  \a minification and
  * \a magnification must be one of @ref RT_FILTER_NEAREST or
  * @ref RT_FILTER_LINEAR.  \a mipmapping may be any of the three values but
  * must be @ref RT_FILTER_NONE if the texture sampler contains only a
  * single MIP level or one of @ref RT_FILTER_NEAREST or @ref RT_FILTER_LINEAR
  * if the texture sampler contains more than one MIP level.
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   minification     The new minification filter mode of the texture sampler
  * @param[in]   magnification    The new magnification filter mode of the texture sampler
  * @param[in]   mipmapping       The new MIP mapping filter mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetFilteringModes was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetFilteringModes
  *
  */
  RTresult RTAPI rtTextureSamplerSetFilteringModes(RTtexturesampler texturesampler, RTfiltermode  minification, RTfiltermode  magnification, RTfiltermode mipmapping);

  /**
  * @brief Gets the filtering modes of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetFilteringModes gets the minification, magnification and MIP mapping filtering modes from
  * \a texturesampler and stores them in \a *minification, \a *magnification and \a *mipmapping, respectively.  See
  * @ref rtTextureSamplerSetFilteringModes for the values @ref RTfiltermode may take.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[out]  minification     The return handle for the minification filtering mode of the texture sampler
  * @param[out]  magnification    The return handle for the magnification filtering mode of the texture sampler
  * @param[out]  mipmapping       The return handle for the MIP mapping filtering mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetFilteringModes was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetFilteringModes
  *
  */
  RTresult RTAPI rtTextureSamplerGetFilteringModes(RTtexturesampler texturesampler, RTfiltermode* minification, RTfiltermode* magnification, RTfiltermode* mipmapping);

  /**
  * @brief Sets the maximum anisotropy of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetMaxAnisotropy sets the maximum anisotropy of \a texturesampler to \a value.  A float
  * value specifies the maximum anisotropy ratio to be used when doing anisotropic filtering. This value will be clamped to the range [1,16]
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   value            The new maximum anisotropy level of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetMaxAnisotropy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetMaxAnisotropy
  *
  */
  RTresult RTAPI rtTextureSamplerSetMaxAnisotropy(RTtexturesampler texturesampler, float value);

  /**
  * @brief Gets the maximum anisotropy level for a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetMaxAnisotropy gets the maximum anisotropy level for \a texturesampler and stores
  * it in \a *value.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[out]  value            The return handle for the maximum anisotropy level of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetMaxAnisotropy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetMaxAnisotropy
  *
  */
  RTresult RTAPI rtTextureSamplerGetMaxAnisotropy(RTtexturesampler texturesampler, float* value);

  /**
  * @brief Sets the minimum and the maximum MIP level access range of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetMipLevelClamp sets lower end and the upper end of the MIP level range to clamp access to.
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   minLevel         The new minimum mipmap level of the texture sampler
  * @param[in]   maxLevel         The new maximum mipmap level of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetMipLevelClamp was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetMipLevelClamp
  *
  */
  RTresult RTAPI rtTextureSamplerSetMipLevelClamp(RTtexturesampler texturesampler, float minLevel, float maxLevel);

  /**
  * @brief Gets the minimum and the maximum MIP level access range for a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetMipLevelClamp gets the minimum and the maximum MIP level access range for \a texturesampler and stores
  * it in \a *minLevel and \a maxLevel.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[out]  minLevel         The return handle for the minimum mipmap level of the texture sampler
  * @param[out]  maxLevel         The return handle for the maximum mipmap level of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetMipLevelClamp was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetMipLevelClamp
  *
  */
  RTresult RTAPI rtTextureSamplerGetMipLevelClamp(RTtexturesampler texturesampler, float* minLevel, float* maxLevel);

  /**
  * @brief Sets the mipmap offset of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetMipLevelBias sets the offset to be applied to the calculated mipmap level.
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   value            The new mipmap offset of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetMipLevelBias was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetMipLevelBias
  *
  */
  RTresult RTAPI rtTextureSamplerSetMipLevelBias(RTtexturesampler texturesampler, float value);

  /**
  * @brief Gets the mipmap offset for a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetMipLevelBias gets the mipmap offset for \a texturesampler and stores
  * it in \a *value.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[out]  value            The return handle for the mipmap offset of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetMipLevelBias was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetMipLevelBias
  *
  */
  RTresult RTAPI rtTextureSamplerGetMipLevelBias(RTtexturesampler texturesampler, float* value);

  /**
  * @brief Sets the read mode of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetReadMode sets the data read mode of \a texturesampler to \a readmode.
  * \a readmode can take one of the following values:
  *
  *  - @ref RT_TEXTURE_READ_ELEMENT_TYPE
  *  - @ref RT_TEXTURE_READ_NORMALIZED_FLOAT
  *  - @ref RT_TEXTURE_READ_ELEMENT_TYPE_SRGB 
  *  - @ref RT_TEXTURE_READ_NORMALIZED_FLOAT_SRGB
  *
  * @ref RT_TEXTURE_READ_ELEMENT_TYPE_SRGB and @ref RT_TEXTURE_READ_NORMALIZED_FLOAT_SRGB were introduced in OptiX 3.9 
  * and apply sRGB to linear conversion during texture read for 8-bit integer buffer formats.
  * \a readmode controls the returned value of the texture sampler when it is used to sample
  * textures.  @ref RT_TEXTURE_READ_ELEMENT_TYPE will return data of the type of the underlying
  * buffer objects.  @ref RT_TEXTURE_READ_NORMALIZED_FLOAT will return floating point values
  * normalized by the range of the underlying type.  If the underlying type is floating point,
  * @ref RT_TEXTURE_READ_NORMALIZED_FLOAT and @ref RT_TEXTURE_READ_ELEMENT_TYPE are equivalent,
  * always returning the unmodified floating point value.
  *
  * For example, a texture sampler that samples a buffer of type @ref RT_FORMAT_UNSIGNED_BYTE with
  * a read mode of @ref RT_TEXTURE_READ_NORMALIZED_FLOAT will convert integral values from the
  * range [0,255] to floating point values in the range [0,1] automatically as the buffer is
  * sampled from.
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   readmode         The new read mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetReadMode was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetReadMode
  *
  */
  RTresult RTAPI rtTextureSamplerSetReadMode(RTtexturesampler texturesampler, RTtexturereadmode readmode);

  /**
  * @brief Gets the read mode of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetReadMode gets the read mode of \a texturesampler and stores it in \a *readmode.
  * See @ref rtTextureSamplerSetReadMode for a list of values @ref RTtexturereadmode can take.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[out]  readmode         The return handle for the read mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetReadMode was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetReadMode
  *
  */
  RTresult RTAPI rtTextureSamplerGetReadMode(RTtexturesampler texturesampler, RTtexturereadmode* readmode);

  /**
  * @brief Sets whether texture coordinates for this texture sampler are normalized
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetIndexingMode sets the indexing mode of \a texturesampler to \a indexmode.  \a indexmode
  * can take on one of the following values:
  *
  *  - @ref RT_TEXTURE_INDEX_NORMALIZED_COORDINATES,
  *  - @ref RT_TEXTURE_INDEX_ARRAY_INDEX
  *
  * These values are used to control the interpretation of texture coordinates.  If the index mode is set to
  * @ref RT_TEXTURE_INDEX_NORMALIZED_COORDINATES, the texture is parameterized over [0,1].  If the index
  * mode is set to @ref RT_TEXTURE_INDEX_ARRAY_INDEX then texture coordinates are interpreted as array indices
  * into the contents of the underlying buffer objects.
  *
  * @param[in]   texturesampler   The texture sampler object to be changed
  * @param[in]   indexmode        The new indexing mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetIndexingMode was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetIndexingMode
  *
  */
  RTresult RTAPI rtTextureSamplerSetIndexingMode(RTtexturesampler texturesampler, RTtextureindexmode indexmode);

  /**
  * @brief Gets the indexing mode of a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetIndexingMode gets the indexing mode of \a texturesampler and stores it in \a *indexmode.
  * See @ref rtTextureSamplerSetIndexingMode for the values @ref RTtextureindexmode may take.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried
  * @param[out]  indexmode        The return handle for the indexing mode of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetIndexingMode was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetIndexingMode
  *
  */
  RTresult RTAPI rtTextureSamplerGetIndexingMode(RTtexturesampler texturesampler, RTtextureindexmode* indexmode);

  /**
  * @brief Attaches a buffer object to a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerSetBuffer attaches \a buffer to \a texturesampler.
  *
  * @param[in]   texturesampler      The texture sampler object that will contain the buffer
  * @param[in]   deprecated0         Deprecated in OptiX 3.9, must be 0
  * @param[in]   deprecated1         Deprecated in OptiX 3.9, must be 0
  * @param[in]   buffer              The buffer to be attached to the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerSetBuffer was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerGetBuffer
  *
  */
  RTresult RTAPI rtTextureSamplerSetBuffer(RTtexturesampler texturesampler, unsigned int deprecated0, unsigned int deprecated1, RTbuffer buffer);
    
  /**
  * @brief Gets a buffer object handle from a texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetBuffer gets a buffer object from
  * \a texturesampler and
  * stores it in \a *buffer.
  *
  * @param[in]   texturesampler      The texture sampler object to be queried for the buffer
  * @param[in]   deprecated0         Deprecated in OptiX 3.9, must be 0
  * @param[in]   deprecated1         Deprecated in OptiX 3.9, must be 0
  * @param[out]  buffer              The return handle to the buffer attached to the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetBuffer was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerSetBuffer
  *
  */
  RTresult RTAPI rtTextureSamplerGetBuffer(RTtexturesampler texturesampler, unsigned int deprecated0, unsigned int deprecated1, RTbuffer* buffer);

  /**
  * @brief Returns the texture ID of this texture sampler
  *
  * @ingroup TextureSampler
  *
  * <B>Description</B>
  *
  * @ref rtTextureSamplerGetId returns a handle to the texture sampler
  * \a texturesampler to be used in OptiX programs on the device to
  * reference the associated texture. The returned ID cannot be used on
  * the host side. If \a texture_id is \a NULL, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   texturesampler   The texture sampler object to be queried for its ID
  * @param[out]  texture_id       The returned device-side texture ID of the texture sampler
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtTextureSamplerGetId was introduced in OptiX 3.0.
  *
  * <B>See also</B>
  * @ref rtTextureSamplerCreate
  *
  */
  RTresult RTAPI rtTextureSamplerGetId(RTtexturesampler texturesampler, int* texture_id);

/************************************
 **
 **    Buffer object
 **
 ***********************************/

  /**
  * @brief Creates a new buffer object
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferCreate allocates and returns a new handle to a new buffer object in \a *buffer associated
  * with \a context. The backing storage of the buffer is managed by OptiX. A buffer is specified by a bitwise
  * \a or combination of a \a type and \a flags in \a bufferdesc. The supported types are:
  *
  * -  @ref RT_BUFFER_INPUT
  * -  @ref RT_BUFFER_OUTPUT
  * -  @ref RT_BUFFER_INPUT_OUTPUT
  * -  @ref RT_BUFFER_PROGRESSIVE_STREAM
  *
  * The type values are used to specify the direction of data flow from the host to the OptiX devices.
  * @ref RT_BUFFER_INPUT specifies that the host may only write to the buffer and the device may only read from the buffer.
  * @ref RT_BUFFER_OUTPUT specifies the opposite, read only access on the host and write only access on the device.
  * Devices and the host may read and write from buffers of type @ref RT_BUFFER_INPUT_OUTPUT.  Reading or writing to
  * a buffer of the incorrect type (e.g., the host writing to a buffer of type @ref RT_BUFFER_OUTPUT) is undefined.
  * @ref RT_BUFFER_PROGRESSIVE_STREAM is used to receive stream updates generated by progressive launches (see @ref rtContextLaunchProgressive2D).
  *
  * The supported flags are:
  *
  * -  @ref RT_BUFFER_GPU_LOCAL
  * -  @ref RT_BUFFER_COPY_ON_DIRTY
  * -  @ref RT_BUFFER_LAYERED
  * -  @ref RT_BUFFER_CUBEMAP
  *
  * If RT_BUFFER_LAYERED flag is set, buffer depth specifies the number of layers, not the depth of a 3D buffer.
  * If RT_BUFFER_CUBEMAP flag is set, buffer depth specifies the number of cube faces, not the depth of a 3D buffer.
  * See details in @ref rtBufferSetSize3D 
  *
  * Flags can be used to optimize data transfers between the host and its devices. The flag @ref RT_BUFFER_GPU_LOCAL can only be
  * used in combination with @ref RT_BUFFER_INPUT_OUTPUT. @ref RT_BUFFER_INPUT_OUTPUT and @ref RT_BUFFER_GPU_LOCAL used together specify a buffer
  * that allows the host to \a only write, and the device to read \a and write data. The written data will never be visible
  * on the host side and will generally not be visible on other devices.
  *
  * If @ref rtBufferGetDevicePointer has been called for a single device for a given buffer,
  * the user can change the buffer's content on that device through the pointer. OptiX must then synchronize the new buffer contents to all devices.
  * These synchronization copies occur at every @ref rtContextLaunch "rtContextLaunch", unless the buffer is created with @ref RT_BUFFER_COPY_ON_DIRTY.
  * In this case, @ref rtBufferMarkDirty can be used to notify OptiX that the buffer has been dirtied and must be synchronized.
  *
  * Returns @ref RT_ERROR_INVALID_VALUE if \a buffer is \a NULL.
  *
  * @param[in]   context      The context to create the buffer in
  * @param[in]   bufferdesc   Bitwise \a or combination of the \a type and \a flags of the new buffer
  * @param[out]  buffer       The return handle for the buffer object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferCreate was introduced in OptiX 1.0.
  *
  * @ref RT_BUFFER_GPU_LOCAL was introduced in OptiX 2.0.
  *
  * <B>See also</B>
  * @ref rtBufferCreateFromGLBO,
  * @ref rtBufferDestroy,
  * @ref rtBufferMarkDirty
  * @ref rtBufferBindProgressiveStream
  *
  */
  RTresult RTAPI rtBufferCreate(RTcontext context, unsigned int bufferdesc, RTbuffer* buffer);

  /**
  * @brief Destroys a buffer object
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferDestroy removes \a buffer from its context and deletes it.
  * \a buffer should be a value returned by @ref rtBufferCreate.
  * After the call, \a buffer is no longer a valid handle.
  * Any API object that referenced \a buffer will have its reference invalidated.
  *
  * @param[in]   buffer   Handle of the buffer to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferDestroy was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferCreate,
  * @ref rtBufferCreateFromGLBO
  *
  */
  RTresult RTAPI rtBufferDestroy(RTbuffer buffer);

  /**
  * @brief Validates the state of a buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferValidate checks \a buffer for completeness.  If \a buffer has not had its dimensionality, size or format
  * set, this call will return @ref RT_ERROR_INVALID_CONTEXT.
  *
  * @param[in]   buffer   The buffer to validate
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferValidate was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferCreate,
  * @ref rtBufferCreateFromGLBO
  * @ref rtContextValidate
  *
  */
  RTresult RTAPI rtBufferValidate(RTbuffer buffer);

  /**
  * @brief Returns the context object that created this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetContext returns a handle to the context that created \a buffer in \a *context.
  * If \a *context is \a NULL, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   buffer    The buffer to be queried for its context
  * @param[out]  context   The return handle for the buffer's context
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetContext was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtContextCreate
  *
  */
  RTresult RTAPI rtBufferGetContext(RTbuffer buffer, RTcontext* context);

  /**
  * @brief Sets the format of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetFormat changes the \a format of \a buffer to the specified value.
  * The data elements of the buffer will have the specified type and can either be
  * vector formats, or a user-defined type whose size is specified with
  * @ref rtBufferSetElementSize. Possible values for \a format are:
  *
  *   - @ref RT_FORMAT_HALF
  *   - @ref RT_FORMAT_HALF2
  *   - @ref RT_FORMAT_HALF3
  *   - @ref RT_FORMAT_HALF4
  *   - @ref RT_FORMAT_FLOAT
  *   - @ref RT_FORMAT_FLOAT2
  *   - @ref RT_FORMAT_FLOAT3
  *   - @ref RT_FORMAT_FLOAT4
  *   - @ref RT_FORMAT_BYTE
  *   - @ref RT_FORMAT_BYTE2
  *   - @ref RT_FORMAT_BYTE3
  *   - @ref RT_FORMAT_BYTE4
  *   - @ref RT_FORMAT_UNSIGNED_BYTE
  *   - @ref RT_FORMAT_UNSIGNED_BYTE2
  *   - @ref RT_FORMAT_UNSIGNED_BYTE3
  *   - @ref RT_FORMAT_UNSIGNED_BYTE4
  *   - @ref RT_FORMAT_SHORT
  *   - @ref RT_FORMAT_SHORT2
  *   - @ref RT_FORMAT_SHORT3
  *   - @ref RT_FORMAT_SHORT4
  *   - @ref RT_FORMAT_UNSIGNED_SHORT
  *   - @ref RT_FORMAT_UNSIGNED_SHORT2
  *   - @ref RT_FORMAT_UNSIGNED_SHORT3
  *   - @ref RT_FORMAT_UNSIGNED_SHORT4
  *   - @ref RT_FORMAT_INT
  *   - @ref RT_FORMAT_INT2
  *   - @ref RT_FORMAT_INT3
  *   - @ref RT_FORMAT_INT4
  *   - @ref RT_FORMAT_UNSIGNED_INT
  *   - @ref RT_FORMAT_UNSIGNED_INT2
  *   - @ref RT_FORMAT_UNSIGNED_INT3
  *   - @ref RT_FORMAT_UNSIGNED_INT4
  *   - @ref RT_FORMAT_USER
  *
  * @param[in]   buffer   The buffer to have its format set
  * @param[in]   format   The target format of the buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferSetFormat was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetFormat,
  * @ref rtBufferGetFormat,
  * @ref rtBufferGetFormat,
  * @ref rtBufferGetElementSize,
  * @ref rtBufferSetElementSize
  *
  */
  RTresult RTAPI rtBufferSetFormat(RTbuffer buffer, RTformat format);

  /**
  * @brief Gets the format of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetFormat returns, in \a *format, the format of \a buffer.  See @ref rtBufferSetFormat for a listing
  * of @ref RTbuffer values.
  *
  * @param[in]   buffer   The buffer to be queried for its format
  * @param[out]  format   The return handle for the buffer's format
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetFormat was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetFormat,
  * @ref rtBufferGetFormat
  *
  */
  RTresult RTAPI rtBufferGetFormat(RTbuffer buffer, RTformat* format);

  /**
  * @brief Modifies the size in bytes of a buffer's individual elements
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetElementSize modifies the size in bytes of a buffer's user-formatted
  * elements. The target buffer is specified by \a buffer, which should be a
  * value returned by @ref rtBufferCreate and should have format @ref RT_FORMAT_USER.
  * The new size of the buffer's individual elements is specified by
  * \a element_size and should not be 0. If the buffer has
  * format @ref RT_FORMAT_USER, and \a element_size is not 0, then the buffer's individual
  * element size is set to \a element_size and all storage associated with the buffer is reset.
  * Otherwise, this call has no effect and returns either @ref RT_ERROR_TYPE_MISMATCH if
  * the buffer does not have format @ref RT_FORMAT_USER or @ref RT_ERROR_INVALID_VALUE if the
  * buffer has format @ref RT_FORMAT_USER but \a element_size is 0.
  *
  * @param[in]   buffer            Specifies the buffer to be modified
  * @param[in]   size_of_element   Specifies the new size in bytes of the buffer's individual elements
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_TYPE_MISMATCH
  *
  * <B>History</B>
  *
  * @ref rtBufferSetElementSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferGetElementSize,
  * @ref rtBufferCreate
  *
  */
  RTresult RTAPI rtBufferSetElementSize(RTbuffer buffer, RTsize size_of_element);

  /**
  * @brief Returns the size of a buffer's individual elements
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetElementSize queries the size of a buffer's elements. The target buffer
  * is specified by \a buffer, which should be a value returned by
  * @ref rtBufferCreate. The size, in bytes, of the buffer's
  * individual elements is returned in \a *element_size_return.
  * Returns @ref RT_ERROR_INVALID_VALUE if given a \a NULL pointer.
  *
  * @param[in]   buffer                Specifies the buffer to be queried
  * @param[out]  size_of_element       Returns the size of the buffer's individual elements
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_UNKNOWN
  *
  * <B>History</B>
  *
  * @ref rtBufferGetElementSize was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetElementSize,
  * @ref rtBufferCreate
  *
  */
  RTresult RTAPI rtBufferGetElementSize(RTbuffer buffer, RTsize* size_of_element);

  /**
  * @brief Sets the width and dimensionality of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetSize1D sets the dimensionality of \a buffer to 1 and sets its width to
  * \a width.
  * Fails with @ref RT_ERROR_ALREADY_MAPPED if called on a buffer that is mapped.
  *
  * @param[in]   buffer   The buffer to be resized
  * @param[in]   width    The width of the resized buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferSetSize1D was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferSetSize1D(RTbuffer buffer, RTsize width);

  /**
  * @brief Get the width of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetSize1D stores the width of \a buffer in \a *width.
  *
  * @param[in]   buffer   The buffer to be queried for its dimensions
  * @param[out]  width    The return handle for the buffer's width
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetSize1D was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetSize1D(RTbuffer buffer, RTsize* width);

  /**
  * @brief Sets the width, height and dimensionality of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetSize2D sets the dimensionality of \a buffer to 2 and sets its width
  * and height to \a width and \a height, respectively.  If \a width or \a height is
  * zero, they both must be zero.
  * Fails with @ref RT_ERROR_ALREADY_MAPPED if called on a buffer that is mapped.
  *
  * @param[in]   buffer   The buffer to be resized
  * @param[in]   width    The width of the resized buffer
  * @param[in]   height   The height of the resized buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferSetSize2D was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferSetSize2D(RTbuffer buffer, RTsize width, RTsize height);

  /**
  * @brief Gets the width and height of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetSize2D stores the width and height of \a buffer in \a *width and
  * \a *height, respectively.
  *
  * @param[in]   buffer   The buffer to be queried for its dimensions
  * @param[out]  width    The return handle for the buffer's width
  * @param[out]  height   The return handle for the buffer's height
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetSize2D was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetSize2D(RTbuffer buffer, RTsize* width, RTsize* height);

  /**
  * @brief Sets the width, height, depth and dimensionality of a buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetSize3D sets the dimensionality of \a buffer to 3 and sets its width,
  * height and depth to \a width, \a height and \a depth, respectively.  If \a width,
  * \a height or \a depth is zero, they all must be zero.
  *
  * A 1D layered mipmapped buffer is allocated if \a height is 1 and the @ref RT_BUFFER_LAYERED flag was set at buffer creating. The number of layers is determined by the \a depth.
  * A 2D layered mipmapped buffer is allocated if the @ref RT_BUFFER_LAYERED flag was set at buffer creating. The number of layers is determined by the \a depth.
  * A cubemap mipmapped buffer is allocated if the @ref RT_BUFFER_CUBEMAP flag was set at buffer creating. \a width must be equal to \a height and the number of cube faces is determined by the \a depth, 
  * it must be six or a multiple of six, if the @ref RT_BUFFER_LAYERED flag was also set.
  * Layered, mipmapped and cubemap buffers are supported only as texture buffers.
  *
  * Fails with @ref RT_ERROR_ALREADY_MAPPED if called on a buffer that is mapped.
  *
  * @param[in]   buffer   The buffer to be resized
  * @param[in]   width    The width of the resized buffer
  * @param[in]   height   The height of the resized buffer
  * @param[in]   depth    The depth of the resized buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferSetSize3D was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferSetSize3D(RTbuffer buffer, RTsize width, RTsize height, RTsize depth);
      
  /**
  * @brief Sets the MIP level count of a buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetMipLevelCount sets the number of MIP levels to \a levels. The default number of MIP levels is 1.
  * Fails with @ref RT_ERROR_ALREADY_MAPPED if called on a buffer that is mapped. 
  *
  * @param[in]   buffer   The buffer to be resized
  * @param[in]   width    The width of the resized buffer
  * @param[in]   levels   Number of mip levels
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferSetMipLevelCount was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferSetMipLevelCount(RTbuffer buffer, unsigned int levels);


  /**
  * @brief Gets the width, height and depth of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetSize3D stores the width, height and depth of \a buffer in \a *width,
  * \a *height and \a *depth, respectively.
  *
  * @param[in]   buffer   The buffer to be queried for its dimensions
  * @param[out]  width    The return handle for the buffer's width
  * @param[out]  height   The return handle for the buffer's height
  * @param[out]  depth    The return handle for the buffer's depth
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetSize3D was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetSize3D(RTbuffer buffer, RTsize* width, RTsize* height, RTsize* depth);

  /**
  * @brief Gets the width of buffer specific MIP level
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetMipLevelSize1D stores the width of \a buffer in \a *width.
  *
  * @param[in]   buffer   The buffer to be queried for its dimensions
  * @param[in]   level    The buffer MIP level index to be queried for its dimensions
  * @param[out]  width    The return handle for the buffer's width
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferGetMipLevelSize1D was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetMipLevelSize1D(RTbuffer buffer, unsigned int level, RTsize* width);

  /**
  * @brief Gets the width, height of buffer specific MIP level
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetMipLevelSize2D stores the width, height of \a buffer in \a *width and
  * \a *height respectively.
  *
  * @param[in]   buffer   The buffer to be queried for its dimensions
  * @param[in]   level    The buffer MIP level index to be queried for its dimensions
  * @param[out]  width    The return handle for the buffer's width
  * @param[out]  height   The return handle for the buffer's height
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetMipLevelSize2D was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetMipLevelSize2D(RTbuffer buffer, unsigned int level, RTsize* width, RTsize* height);


  /**
  * @brief Gets the width, height and depth of buffer specific MIP level
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetMipLevelSize3D stores the width, height and depth of \a buffer in \a *width,
  * \a *height and \a *depth, respectively.
  *
  * @param[in]   buffer   The buffer to be queried for its dimensions
  * @param[in]   level    The buffer MIP level index to be queried for its dimensions
  * @param[out]  width    The return handle for the buffer's width
  * @param[out]  height   The return handle for the buffer's height
  * @param[out]  depth    The return handle for the buffer's depth
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferGetMipLevelSize3D was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetMipLevelSize3D(RTbuffer buffer, unsigned int level, RTsize* width, RTsize* height, RTsize* depth);


  /**
  * @brief Sets the dimensionality and dimensions of a buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferSetSizev sets the dimensionality of \a buffer to \a dimensionality and
  * sets the dimensions of the buffer to the values stored at *\a dims, which must contain
  * a number of values equal to \a dimensionality.  If any of values of \a dims is zero
  * they must all be zero.
  *
  * @param[in]   buffer           The buffer to be resized
  * @param[in]   dimensionality   The dimensionality the buffer will be resized to
  * @param[in]   dims             The array of sizes for the dimension of the resize
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferSetSizev was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferSetSizev(RTbuffer buffer, unsigned int dimensionality, const RTsize* dims);

  /**
  * @brief Gets the dimensions of this buffer
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetSizev stores the dimensions of \a buffer in \a *dims.  The number of
  * dimensions returned is specified by \a dimensionality.  The storage at \a dims must be
  * large enough to hold the number of requested buffer dimensions.
  *
  * @param[in]   buffer           The buffer to be queried for its dimensions
  * @param[in]   dimensionality   The number of requested dimensions
  * @param[out]  dims             The array of dimensions to store to
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *  
  * @ref rtBufferGetSizev was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetMipLevelCount,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D
  *
  */
  RTresult RTAPI rtBufferGetSizev(RTbuffer buffer, unsigned int dimensionality, RTsize* dims);

  /**
  * @brief Gets the dimensionality of this buffer object
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetDimensionality returns the dimensionality of \a buffer in \a
  * *dimensionality.  The value returned will be one of 1, 2 or 3, corresponding to 1D, 2D
  * and 3D buffers, respectively.
  *
  * @param[in]   buffer           The buffer to be queried for its dimensionality
  * @param[out]  dimensionality   The return handle for the buffer's dimensionality
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetDimensionality was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * \a rtBufferSetSize{1-2-3}D
  *
  */
  RTresult RTAPI rtBufferGetDimensionality(RTbuffer buffer, unsigned int* dimensionality);
   
  /**
  * @brief Gets the number of mipmap levels of this buffer object
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetMipLevelCount returns the number of mipmap levels. Default number of MIP levels is 1.
  *
  * @param[in]   buffer           The buffer to be queried for its number of mipmap levels
  * @param[out]  level            The return number of mipmap levels
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferGetMipLevelCount was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferSetMipLevelCount,
  * @ref rtBufferSetSize1D,
  * @ref rtBufferSetSize2D,
  * @ref rtBufferSetSize3D,
  * @ref rtBufferSetSizev,
  * @ref rtBufferGetMipLevelSize1D,
  * @ref rtBufferGetMipLevelSize2D,
  * @ref rtBufferGetMipLevelSize3D,
  * @ref rtBufferGetSize1D,
  * @ref rtBufferGetSize2D,
  * @ref rtBufferGetSize3D,
  * @ref rtBufferGetSizev
  *
  */
  RTresult RTAPI rtBufferGetMipLevelCount(RTbuffer buffer, unsigned int* level);

  /**
  * @brief Maps a buffer object to the host
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferMap returns a pointer, accessible by the host, in \a *user_pointer that
  * contains a mapped copy of the contents of \a buffer.  The memory pointed to by \a *user_pointer
  * can be written to or read from, depending on the type of \a buffer.  For
  * example, this code snippet demonstrates creating and filling an input buffer with
  * floats.
  *
  *@code
  *  RTbuffer buffer;
  *  float* data;
  *  rtBufferCreate(context, RT_BUFFER_INPUT, &buffer);
  *  rtBufferSetFormat(buffer, RT_FORMAT_FLOAT);
  *  rtBufferSetSize1D(buffer, 10);
  *  rtBufferMap(buffer, (void*)&data);
  *  for(int i = 0; i < 10; ++i)
  *    data[i] = 4.f * i;
  *  rtBufferUnmap(buffer);
  *@endcode
  * If \a buffer has already been mapped, returns @ref RT_ERROR_ALREADY_MAPPED.
  * If \a buffer has size zero, the returned pointer is undefined
  *
  * Note that this call does not stop a progressive render if called on a stream buffer.
  *
  * @param[in]   buffer         The buffer to be mapped
  * @param[out]  user_pointer   Return handle to a user pointer where the buffer will be mapped to
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferMap was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferUnmap,
  * @ref rtBufferMapEx,
  * @ref rtBufferUnmapEx
  *
  */
  RTresult RTAPI rtBufferMap(RTbuffer buffer, void** user_pointer);

  /**
  * @brief Unmaps a buffer's storage from the host
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferUnmap unmaps a buffer from the host after a call to @ref rtBufferMap.  @ref rtContextLaunch "rtContextLaunch" cannot be called
  * while buffers are still mapped to the host.  A call to @ref rtBufferUnmap that does not follow a matching @ref rtBufferMap
  * call will return @ref RT_ERROR_INVALID_VALUE.
  *
  * Note that this call does not stop a progressive render if called with a stream buffer.
  *
  * @param[in]   buffer   The buffer to unmap
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferUnmap was introduced in OptiX 1.0.
  *
  * <B>See also</B>
  * @ref rtBufferMap,
  * @ref rtBufferMapEx,
  * @ref rtBufferUnmapEx
  *
  */
  RTresult RTAPI rtBufferUnmap(RTbuffer buffer);

  /**
  * @brief Maps mipmap level of buffer object to the host
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferMapEx makes the buffer contents available on the host, either by returning a pointer in \a *optix_owned, or by copying the contents
  * to a memory location pointed to by \a user_owned. Calling @ref rtBufferMapEx with proper map flags can result in better performance than using @ref rtBufferMap, because
  * fewer synchronization copies are required in certain situations.
  * @ref rtBufferMapEx with \a map_flags = @ref RT_BUFFER_MAP_READ_WRITE and \a leve = 0 is equivalent to @ref rtBufferMap.
  *
  * Note that this call does not stop a progressive render if called on a stream buffer.
  *
  * @param[in]   buffer         The buffer to be mapped
  * @param[in]   map_flags      Map flags, see below
  * @param[in]   level          The mipmap level to be mapped
  * @param[in]   user_owned     Not yet supported. Must be NULL
  * @param[out]  optix_owned    Return handle to a user pointer where the buffer will be mapped to
  *
  * The following flags are supported for map_flags. They are mutually exclusive:
  *
  * -  @ref RT_BUFFER_MAP_READ
  * -  @ref RT_BUFFER_MAP_WRITE
  * -  @ref RT_BUFFER_MAP_READ_WRITE
  * -  @ref RT_BUFFER_MAP_WRITE_DISCARD
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_ALREADY_MAPPED
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferMapEx was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferMap,
  * @ref rtBufferUnmap,
  * @ref rtBufferUnmapEx
  *
  */
  RTresult RTAPI rtBufferMapEx(RTbuffer buffer, unsigned int map_flags, unsigned int level, void* user_owned, void** optix_owned);

  /**
  * @brief Unmaps mipmap level storage from the host
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferUnmapEx unmaps buffer level from the host after a call to @ref rtBufferMapEx.  @ref rtContextLaunch "rtContextLaunch" cannot be called
  * while buffers are still mapped to the host.  A call to @ref rtBufferUnmapEx that does not follow a matching @ref rtBufferMapEx
  * call will return @ref RT_ERROR_INVALID_VALUE. @ref rtBufferUnmap is equivalent to @ref rtBufferUnmapEx with \a level = 0.
  *
  * Note that this call does not stop a progressive render if called with a stream buffer.
  *
  * @param[in]   buffer   The buffer to unmap
  * @param[in]   level    The mipmap level to unmap
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_CONTEXT
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_MEMORY_ALLOCATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtBufferUnmapEx was introduced in OptiX 3.9.
  *
  * <B>See also</B>
  * @ref rtBufferMap,
  * @ref rtBufferUnmap,
  * @ref rtBufferMapEx
  *
  */
  RTresult RTAPI rtBufferUnmapEx(RTbuffer buffer, unsigned int level);

  /**
  * @brief Gets an id suitable for use with buffers of buffers
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetId returns an ID for the provided buffer.  The returned ID is used on
  * the device to reference the buffer.  It needs to be copied into a buffer of type @ref
  * RT_FORMAT_BUFFER_ID or used in a @ref rtBufferId object.. If \a *buffer_id is \a NULL
  * or the \a buffer is not a valid RTbuffer, returns @ref
  * RT_ERROR_INVALID_VALUE.  @ref RT_BUFFER_ID_NULL can be used as a sentinal for a
  * non-existent buffer, since this value will never be returned as a valid buffer id.
  *
  * @param[in]   buffer      The buffer to be queried for its id
  * @param[out]  buffer_id   The returned ID of the buffer
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferGetId was introduced in OptiX 3.5.
  *
  * <B>See also</B>
  * @ref rtContextGetBufferFromId
  *
  */
  RTresult RTAPI rtBufferGetId(RTbuffer buffer, int* buffer_id);

  /**
  * @brief Gets an RTbuffer corresponding to the buffer id
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtContextGetBufferFromId returns a handle to the buffer in \a *buffer corresponding to
  * the \a buffer_id supplied.  If \a buffer_id does not map to a valid buffer handle,
  * \a *buffer is \a NULL or if \a context is invalid, returns @ref RT_ERROR_INVALID_VALUE.
  *
  * @param[in]   context     The context the buffer should be originated from
  * @param[in]   buffer_id   The ID of the buffer to query
  * @param[out]  buffer      The return handle for the buffer object corresponding to the buffer_id
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtContextGetBufferFromId was introduced in OptiX 3.5.
  *
  * <B>See also</B>
  * @ref rtBufferGetId
  *
  */
  RTresult RTAPI rtContextGetBufferFromId(RTcontext context, int buffer_id, RTbuffer* buffer);

  /**
  * @brief Check whether stream buffer content has been updated by a Progressive Launch
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * Returns whether or not the result of a progressive launch in \a buffer has been updated
  * since the last time this function was called. A client application should use this call in its
  * main render/display loop to poll for frame refreshes after initiating a progressive launch. If \a subframe_count and
  * \a max_subframes are non-null, they will be filled with the corresponding counters if and
  * only if \a ready returns 1.
  *
  * Note that this call does not stop a progressive render.
  *
  * @param[in]   buffer             The stream buffer to be queried
  * @param[out]  ready              Ready flag. Will be set to 1 if an update is available, or 0 if no update is available.
  * @param[out]  subframe_count     The number of subframes accumulated in the latest result
  * @param[out]  max_subframes      The \a max_subframes parameter as specified in the call to @ref rtContextLaunchProgressive2D
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferGetProgressiveUpdateReady was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtContextLaunchProgressive2D
  *
  */
  RTresult RTAPI rtBufferGetProgressiveUpdateReady(RTbuffer buffer, int* ready, unsigned int* subframe_count, unsigned int* max_subframes);

  /**
  * @brief Bind a stream buffer to an output buffer source
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * Binds an output buffer to a progressive stream. The output buffer thereby becomes the
  * data source for the stream. To form a valid output/stream pair, the stream buffer must be
  * of format @ref RT_FORMAT_UNSIGNED_BYTE4, and the output buffer must be of format @ref RT_FORMAT_FLOAT3 or @ref RT_FORMAT_FLOAT4.
  * The use of @ref RT_FORMAT_FLOAT4 is recommended for performance reasons, even if the fourth component is unused.
  * The output buffer must be of type @ref RT_BUFFER_OUTPUT; it may not be of type @ref RT_BUFFER_INPUT_OUTPUT.
  *
  * @param[in]   stream             The stream buffer for which the source is to be specified
  * @param[in]   source             The output buffer to function as the stream's source
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferBindProgressiveStream was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtBufferCreate
  * @ref rtBufferSetAttribute
  * @ref rtBufferGetAttribute
  *
  */
  RTresult RTAPI rtBufferBindProgressiveStream(RTbuffer stream, RTbuffer source);

  /**
  * @brief Set a buffer attribute
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * Sets a buffer attribute. Currently, all available attributes refer to stream buffers only,
  * and attempting to set them on a non-stream buffer will generate an error.
  *
  * Each attribute can have a different size.  The sizes are given in the following list:
  *
  *   - @ref RT_BUFFER_ATTRIBUTE_STREAM_FORMAT      strlen(input_string)
  *   - @ref RT_BUFFER_ATTRIBUTE_STREAM_BITRATE     sizeof(int)
  *   - @ref RT_BUFFER_ATTRIBUTE_STREAM_FPS         sizeof(int)
  *   - @ref RT_BUFFER_ATTRIBUTE_STREAM_GAMMA       sizeof(float)
  *
  * @ref RT_BUFFER_ATTRIBUTE_STREAM_FORMAT sets the encoding format used for streams sent over the network, specified as a string.
  * The default is "auto". Various other common stream and image formats are available (e.g. "h264", "png"). This
  * attribute has no effect if the progressive API is used locally.
  *
  * @ref RT_BUFFER_ATTRIBUTE_STREAM_BITRATE sets the target bitrate for streams sent over the network, if the stream format supports
  * it. The data is specified as a 32-bit integer. The default is 5000000. This attribute has no
  * effect if the progressive API is used locally or if the stream format does not support
  * variable bitrates.
  *
  * @ref RT_BUFFER_ATTRIBUTE_STREAM_FPS sets the target update rate per second for streams sent over the network, if the stream
  * format supports it. The data is specified as a 32-bit integer. The default is 30. This
  * attribute has no effect if the progressive API is used locally or if the stream format does
  * not support variable framerates.
  *
  * @ref RT_BUFFER_ATTRIBUTE_STREAM_GAMMA sets the gamma value for the built-in tonemapping operator. The data is specified as a
  * 32-bit float, the default is 1.0. Tonemapping is executed before encoding the
  * accumulated output into the stream, i.e. on the server side if remote rendering is used.
  * See the section on Buffers below for more details.
  *
  * @param[in]   buffer             The buffer on which to set the attribute
  * @param[in]   attrib             The attribute to set
  * @param[in]   size               The size of the attribute value, in bytes
  * @param[in]   p                  Pointer to the attribute value
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferSetAttribute was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtBufferGetAttribute
  *
  */
  RTresult RTAPI rtBufferSetAttribute(RTbuffer buffer, RTbufferattribute attrib, RTsize size, void* p);

  /**
  * @brief Query a buffer attribute
  *
  * @ingroup Buffer
  *
  * <B>Description</B>
  *
  * @ref rtBufferGetAttribute is used to query buffer attributes. For a list of available attributes, please refer to @ref rtBufferSetAttribute.
  *
  * @param[in]   buffer             The buffer to query the attribute from
  * @param[in]   attrib             The attribute to query
  * @param[in]   size               The size of the attribute value, in bytes. For string attributes, this is the maximum buffer size the returned string will use (including a terminating null character).
  * @param[out]  p                  Pointer to the attribute value to be filled in. Must point to valid memory of at least \a size bytes.
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtBufferGetAttribute was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtBufferSetAttribute
  *
  */
  RTresult RTAPI rtBufferGetAttribute(RTbuffer buffer, RTbufferattribute attrib, RTsize size, void* p);

  /**
  * @brief Create a device for remote rendering on VCAs
  *
  * @ingroup RemoteDevice
  *
  * <B>Description</B>
  *
  * Establishes a connection to a remote OptiX device, e.g. a VCA or cluster of VCAs. This
  * opens a connection to the cluster manager software running at \a address, using username
  * and password as authentication strings.
  * \a address is a WebSocket URL of the form "ws://localhost:80" or "wss://localhost:443",
  * \a username and \a password as plain text strings for authenticating on the remote device.
  * If successful, it initializes a new @ref RTremotedevice object.
  *
  * In order to use this newly created remote device, a rendering instance needs to be
  * configured by selecting a software configuration and reserving a number of nodes
  * in the VCA. See @ref rtRemoteDeviceReserve for more details.
  *
  * After a rendering instance is properly initialized, a remote device must be associated
  * with a context to be used. Calling @ref rtContextSetDevices creates this association. Any
  * further OptiX calls will be directed to the remote device.
  *
  * @param[in]   url            The WebSocket URL to connect to
  * @param[in]   username       Username in plain text
  * @param[in]   password       Password in plain text
  * @param[out]  remote_dev     A handle to the new remote device object
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  * - @ref RT_ERROR_CONNECTION_FAILED
  * - @ref RT_ERROR_AUTHENTICATION_FAILED
  *
  * <B>History</B>
  *
  * @ref rtRemoteDeviceCreate was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtRemoteDeviceDestroy
  * @ref rtRemoteDeviceGetAttribute
  * @ref rtRemoteDeviceReserve
  * @ref rtRemoteDeviceRelease
  * @ref rtContextSetRemoteDevice
  *
  */
  RTresult RTAPI rtRemoteDeviceCreate(const char* url, const char* username, const char* password, RTremotedevice* remote_dev);

  /**
  * @brief Destroys a remote device
  *
  * @ingroup RemoteDevice
  *
  * <B>Description</B>
  *
  * Closes the network connection to the remote device and destroys the corresponding @ref RTremotedevice object.
  *
  * @param[in]   remote_dev     The remote device object to destroy
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtRemoteDeviceDestroy was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtRemoteDeviceCreate
  * @ref rtRemoteDeviceGetAttribute
  * @ref rtRemoteDeviceReserve
  * @ref rtRemoteDeviceRelease
  * @ref rtContextSetRemoteDevice
  *
  */
  RTresult RTAPI rtRemoteDeviceDestroy(RTremotedevice remote_dev);

  /**
  * @brief Queries attributes of a remote device
  *
  * @ingroup RemoteDevice
  *
  * <B>Description</B>
  *
  * In order to gather information about a remote device, several attributes can be queried through @ref rtRemoteDeviceGetAttribute.
  *
  * Each attribute can have a different size.  The sizes are given in the following list:
  *
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_CLUSTER_URL          size of provided destination buffer
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_HEAD_NODE_URL        size of provided destination buffer
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_CONFIGURATIONS   sizeof(int)
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_CONFIGURATIONS       size of provided destination buffer
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_STATUS               sizeof(RTremotedevicestatus)
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_TOTAL_NODES      sizeof(int)
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_FREE_NODES       sizeof(int)
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_RESERVED_NODES   sizeof(int)
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NAME                 size of provided destination buffer
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_GPUS             sizeof(int)
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_GPU_TOTAL_MEMORY     sizeof(RTsize)
  *
  * The following attributes can be queried when a remote device is connected:
  *
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_CLUSTER_URL
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_CONFIGURATIONS
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_CONFIGURATIONS
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_STATUS
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_TOTAL_NODES
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_FREE_NODES
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NAME
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_GPU_TOTAL_MEMORY
  *
  * The following attributes require a valid reservation to be queried:
  *
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_HEAD_NODE_URL
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_RESERVED_NODES
  *   - @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_GPUS
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_CLUSTER_URL
  * The URL of the Cluster Manager associated with this remote device.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_HEAD_NODE_URL
  * The URL of the rendering instance being used, once it has been reserved and initialized.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_CONFIGURATIONS
  * Number of compatible software configurations available in the remote device.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_CONFIGURATIONS
  * Base entry for a list of compatible software configurations in the device. A configuration is a text description for
  * a software package installed in the remote device, intended as a guide to the user in selecting from the pool of
  * compatible configurations. This list is already filtered and it only contains entries on the remote device compatible
  * with the client library being used.
  * Each entry can be accessed as the attribute (RT_REMOTEDEVICE_ATTRIBUTE_CONFIGURATIONS + index),
  * with index being zero-based.
  * The configuration description for the given index is copied into the destination buffer. A suggested size for the destination
  * buffer is 256 characters.
  * The number of entries in the list is given by the value of @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_CONFIGURATIONS.
  * Only configurations compatible with the client version being used are listed.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_STATUS
  * Returns the current status of the remote device, as one of the following:
  *    - @ref RT_REMOTEDEVICE_STATUS_READY          The remote device is ready for use.
  *    - @ref RT_REMOTEDEVICE_STATUS_CONNECTED      The remote device is connected to a cluster manager, but no reservation exists.
  *    - @ref RT_REMOTEDEVICE_STATUS_RESERVED       The remote device has a rendering instance reserved, but it is not yet ready.
  *    - @ref RT_REMOTEDEVICE_STATUS_DISCONNECTED   The remote device has disconnected.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_TOTAL_NODES
  * Total number of nodes in the cluster of VCAs.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_FREE_NODES
  * Number of free nodes available.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_RESERVED_NODES
  * Number of nodes used by the current reservation.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_NUM_GPUS
  * Number of GPUs used by the current reservation.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_NAME
  * Common name assigned the Remote Device.
  *
  * @ref RT_REMOTEDEVICE_ATTRIBUTE_GPU_TOTAL_MEMORY
  * Total amount of memory on each GPU, in bytes.
  *
  * @param[in]   remote_dev     The remote device to query
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtRemoteDeviceGetAttribute was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtRemoteDeviceCreate
  * @ref rtRemoteDeviceReserve
  * @ref rtRemoteDeviceRelease
  * @ref rtContextSetRemoteDevice
  *
  */
  RTresult RTAPI rtRemoteDeviceGetAttribute(RTremotedevice remote_dev, RTremotedeviceattribute attrib, RTsize size, void* p);

  /**
  * @brief Reserve nodes for rendering on a remote device
  *
  * @ingroup RemoteDevice
  *
  * <B>Description</B>
  *
  * Reserves nodes in the remote device to form a rendering instance. Receives \a num_nodes
  * as the number of nodes to reserve, and \a configuration as the index of the software
  * package to use for the created instance. Both the number of available nodes and the list
  * of available configurations in a remote device can be retrieved by
  * @ref rtRemoteDeviceGetAttribute.
  *
  * After successfully reserving the nodes, the @ref RT_REMOTEDEVICE_ATTRIBUTE_STATUS
  * attribute should be polled repeatedly. The rendering instance is ready for use when that
  * attribute is set to RT_REMOTE_DEVICE_STATUS_READY.
  *
  * Only a single reservation per remote device and user can exist at any given time (i.e. a
  * user can have only one rendering instance per remote device). This includes
  * reservations performed through other means, like previous runs that were not properly
  * released, or manual reservations over the cluster manager web interface.
  *
  * @param[in]   remote_dev     The remote device on which to reserve nodes
  * @param[in]   num_nodes      The number of nodes to reserve
  * @param[in]   configuration  The index of the software configuration to use
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtRemoteDeviceReserve was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtRemoteDeviceCreate
  * @ref rtRemoteDeviceGetAttribute
  * @ref rtRemoteDeviceRelease
  * @ref rtContextSetRemoteDevice
  *
  */
  RTresult RTAPI rtRemoteDeviceReserve(RTremotedevice remote_dev, unsigned int num_nodes, unsigned int configuration);

  /**
  * @brief Release reserved nodes on a remote device
  *
  * @ingroup RemoteDevice
  *
  * <B>Description</B>
  *
  * Releases an existing reservation on the remote device. The rendering instance on the
  * remote device is destroyed, and all its remote context information is lost. Further OptiX
  * calls will no longer be directed to the device. A new reservation can take place.
  *
  * @param[in]   remote_dev     The remote device on which the reservation was made
  *
  * <B>Return values</B>
  *
  * Relevant return values:
  * - @ref RT_SUCCESS
  * - @ref RT_ERROR_INVALID_VALUE
  *
  * <B>History</B>
  *
  * @ref rtRemoteDeviceRelease was introduced in OptiX 3.8.
  *
  * <B>See also</B>
  * @ref rtRemoteDeviceCreate
  * @ref rtRemoteDeviceGetAttribute
  * @ref rtRemoteDeviceReserve
  * @ref rtContextSetRemoteDevice
  *
  */
  RTresult RTAPI rtRemoteDeviceRelease(RTremotedevice remote_dev);

#ifdef __cplusplus
}
#endif

#endif /* __optix_optix_host_h__ */

