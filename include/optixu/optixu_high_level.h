/**
* \file optixu_high_level.h
* \brief Simple API for rendering in OptiX
*/

/**
*
* \ingroup rthRender
*
* \brief API for rendering in OptiX
*
* Maybe choose defaults and let user override by setting OptiX variable
*/

#ifndef __optixu_optix_high_level_h__
#define __optixu_optix_high_level_h__

#include <vector_types.h>

#include "../optix.h"
#include "optixu_matrix_namespace.h"
#include "optixu_high_level_shared.h"

// TODO: My lazy, unformatted function-parameter format style has gotten
// mixed up with Dave's tabbed style.  Pick one (probably tabbed) and make
// all conform.
// TODO: Make this header work when included both in C and C++ code
// TODO: The interface would be much nicer with float3 function parameters
// instead of float param[3].  This problem is related to the problem with
// including in both C and C++: I'm getting 'unresolved external symbol' 
// errors with float3 args, so using float arg[3] is a stop-gap workaround.
/* Geometry */

// TODO: Learn Doxygen notations and use them to full advantage, for example for
// argument names


/**
 * \ingroup rthContext
 *
 * Create a context for use with the rth API, with all the scene variables
 * needed by rth declared and initialized.  If the default values are
 * undesirable, they can be modified by their OptiX varibles.  These are,
 * with their default values:
 *
 *    int   rth_max_depth         = 10
 *    float rth_importance_cutoff = 0.01f
 *    float rth_scene_epsilon     = 0.01f
 *
 * At this point in time, this does not set up the entry point (ray generation)
 * program or count.
 */
RTresult RTAPI rthContext(RTcontext* context);

// TODO: Implement 'packed data' handling if signalled by 'format'
/**
* \ingroup rthRender
*
* Create an OptiX Geometry object for a list of triangles and associate it with
* a context.  Triangles are given as "triangle" soup, that is, one vertex
* specification in 'vertex_buffer' corresponds to exactly one triangle's vertex,
* and to no other triangle.  Thus, the first three vertices belong to the first
* triangle, the second three to the second triangle, and so forth.  The
* 'format' parameter gives the layout of how each different Vertex
* attribute, i.e.  vertex(position), normal, texture coordinates, and color, is
* layed out in the 'vertex_buffer' buffer.
*
* The type of each component of each attribute is assumed to be 'float'
*
* Any parameter that is not specified per vertex here but is needed by the
* geometry may be specified by a variable attached to the RTgeometry.
* Any parameter that is not specified per vertex here but is needed by the
* material may be specified by a variable attached to the RTmaterial.
*
*   \param        context         The context to attach the new geometry to
*   \param[out]   geom_object     Return pointer to new OptiX Geometry object
*   \param        vertex_buffer   The buffer containing all vertices
*   \param        triangle_count  How many triangles to create
*   \param        format   The format of how vertices are stored
*/
RTresult RTAPI rthGeometryTriangleList(
  RTcontext               context,
  RTgeometry*             geom_object,
  RTbuffer                vertex_buffer,
  unsigned int            triangle_count,
  RTHvertexBufferFormat   format);

// argument names
// TODO: implemented 'packed data' handling if signaled by 'format'
/**
* \ingroup rthRender
* 
* Create an OptiX Geometry object for a list of indexed vertices, and associate
* it with a context.  Triangles are given as three indices into a buffer of
* pre-defined vertices; thus, a 'vertex_buffer' is provided which defines an
* available set of vertices, and 'index_buffer' defines which vertices
* are used for the three points of each triangle.  Thus, the indices of triangle
* 0 are found in entry 0, 1, and 2 of the 'index_buffer, and in general
* those of triangle i are found at entries 3*i+0, 3*i+1, and 3*i+2.
*
*   \param        context                 The context to associate the geometry with
*   \param[out]   geom_object             Return pointer to the new OptiX Geometry object
*   \param        vertex_buffer           The data of available vertices
*   \param        index_buffer            Defines which vertices are used by which triangles
*   \param        triangle_count          How many triangles to create
*   \param        offset_count            How the vertex data is layed out in vertex_buffer
*/
RTresult RTAPI rthGeometryIndexedTriangles(
  RTcontext                 context,
  RTgeometry*               geom_object,
  RTbuffer                  vertex_buffer,
  RTbuffer                  index_buffer,
  unsigned int              triangle_count,
  RTHvertexBufferFormat     format);

// TODO: Implement the previously documented contract:
//     "If all the vertices share an attribute value, set the OptiX variable for
//     that value on the returned Geometry."
//
/**
* \ingroup rthRender
* Create an OptiX Geometry object for spheres and associate a context with
* it.  
* TODO: Document parameters
* TODO: Document that the .cu code will expect 'spheres_buffer' to have element
*       size equal to 1, so client code should use rtBufferSetFormat( ...,
*       RT_FORMAT_BYTE, ... ) or rtBufferSetElementSize(..., 1, ...) (or
*       somehow fix the design or code so that it doesn't have to, so it's more
*       straightforward to use custom client-side 'struct's). 
*/
RTresult RTAPI rthGeometrySpheres(
  RTcontext               context,
  RTgeometry*             geom_object,
  RTbuffer                spheres_buffer,
  unsigned int            sphere_count,
  RTHsphereBufferFormat   format
);


/* Miss programs */

RTresult RTAPI rthMissConstantColor(
  RTcontext         context,
  RTprogram*        miss_program,
  float             color[3]);

RTresult RTAPI rthMissGradient(
  RTcontext         context,
  RTprogram*        miss_program,
  float             color_up[3],
  float             color_down[3],
  float             up[3]);

// TODO: Dave's original interface here used a 'sun_direction' vector instead
// of the 'sun_phi' and 'sun_theta' angles used here.  I now use angles here
// because it's simpler with the RTHpreethamSunSky class copied from sutil.
// It would be good to make an actual design decision about which is best, or
// maybe offer both.  This should also be improved so the user can predict
// which theta values will map to which longitudes in their own coordinate space.
// TODO: Also, Dave's interface provided a 'sun_color' parameter, but the
// RTHpreethamSunSky class computes a sun color based on internal physical data
// tables and the given sun sky position, so we're using that for now.
// Make a design decision about which is best (or both) for users.
/**
 * An additional OptiX variable, "sun_sky_sky_scale" can be set on the returned
 * program, which controls how the Preetham sky computed colors are scaled
 * to display colors.  This function sets it to an initial default value of 
 * 4.0/100.0.
 */
RTresult RTAPI rthMissSunSky(
  RTcontext         context,
  RTprogram*        miss_program,
  float             overcast,
  float             sun_theta,
  float             sun_phi,
  float             sky_up[3]);

RTresult RTAPI rthMissEnvironmentMap(
  RTcontext         context,
  RTprogram*        miss_object,
  RTtexturesampler  env_map,
  optix::float3     up);
/* Render Mode
  *
  * The Render Mode indicates the ray generation program (and may also influence the closest hit and any hit programs)
  * Render Mode specifies the rendering algorithm (Whitted for now) and the anti-aliasing approach.
  */

/**
* \ingroup rthRender
* \brief Antialiasing algorithms
*
* Most of these will need parameters.
*/
typedef enum {
  RTH_AA_POINT_SAMPLE = 0,  /*!< One sample at each pixel center */
  RTH_AA_SUPERSAMPLING,     /*!< Supersampling */
  RTH_AA_PATH_SPACE_FILTER, /*!< Path space filtering antialiasing */
  RTH_AA_TXAA,              /*!< Morphological antialiasing */
} RTHantialiasingtype;

/* Non-recursive ray traced rendering. May be a special case of Whitted with ray depths of 0? */
RTresult RTAPI rthRenderModeRayCast(
  RTcontext            context,
  RTprogram*           render_mode_object,
  RTbuffer             output_buffer,
  RTHantialiasingtype  aa_type);

/* 1980s Whitted-style recursive ray tracing with reflection, refraction, and shadow rays */
RTresult RTAPI rthRenderModeRecursive(
  RTcontext            context,
  RTprogram*           render_mode_object,
  RTbuffer             output_buffer,
  RTHantialiasingtype  aa_type);


// TODO: I haven't figured out the best way to merge rendering modes with ray
// generation and material programs.

// TODO: Figure out how to have Doxygen use this comment for the \inGroup
// rthRayGeneration as a whole, not just for individual methods.
// TODO: Switch rth_ray_generation_write_output()'s color parameter to type
// float4, to allow alpha-channel rendering as well?  This would require 
// implementing alpha-channel rendering throughout the renderer.
/**
 * A pinhole camera
 *
 * Returns the new pinhole camera ray generatino program, but does not attach it
 * to the context as the entry point.
 *
 * All RayGeneration programs rely on a separate Callable Program to write
 * the rendered color in the proper format to the output buffer.  This allows
 * The renderer to be used with any kind of ouput buffer format, so long as 
 * the OptiX callable program "rth_write_output" is set on
 * the RayGeneration program.  The callable program returns 'void' and takes
 * arguments 'uint2 index' and 'float3 color', where float3 channel values are
 * rgb values in the range [0.0f, 1.0f].
 *
 * A number of 'rthOutputWriter*()' functions are provided for common cases.
 * If 'ray_gen_prog' is a Ray Generation program returned by one of the 
 * rthRayGeneration*() methods, then its output writer callable program could be
 * assigned using code such as:
 *
 *    RTprogram output_writer_prog;
 *    rthOutputWriterRGBByte4(context, &output_writer_prog);
 *    rthSetOutputWriter(ray_gen_prog, output_writer_prog);
 *
 * For completely custom output writers, a custom .cu file must be provided
 * with the callable program, which must declare the common buffer
 * "rth_output_buffer" in the appropriate format.  On the host side, the API
 * function rthOutputWriterCreateBuffer() can then be used to create and bind
 * the output buffer to the program with the appropriate format and sizes.
 * For example,
 *
 *    /////////////////
 *    // Device code
 *    #include <optix.h>
 *    RT_CALLABLE_PROGRAM output_cmyk_byte4(uint2 out_index, float3 color)
 *    {
 *      rtBuffer<uchar4, 2> rth_output_buffer;
 *      rth_output_buffer[out_index] = convert_cmyk(color);
 *    }
 *
 *    ///////////////////
 *    // Host code
 *    RTprogram cmyk_output_prog;
 *    rtProgramCreateFromPTX...( context,
 *                               <.ptx string or path to custom .ptx>,
 *                               <name of callable program within .ptx>,
 *                               &cmyk_output_prog );
 *    rthOutputWriterCreateBuffer( cmyk_output_prog,
 *                                 sizeof(uchar4),
 *                                 <desired buffer width>,
 *                                 <desired buffer height> );
 */
RTresult RTAPI rthRayGenerationPinholeCamera(
  RTcontext            context,
  RTprogram*           ray_generation_program);


/** 
 * Sets the camera pose for the ray generation program, using a 4x4
 * transformation matrix.
 *
 * Transforms the camera from the assumed home position at the origin, with the
 * camera's U axis pointing along world x, V along world y, and W along world z,
 * with the look-along vector being -z.
 *
 * Assumes the context already has declared the OptiX variables
 * "rth_camera_eye", "rth_camera_u", "rth_camera_v", and "rth_camera_w".
 *
 * Assumes 'camera_transformation' is in row-major order.
 */
// TODO: Document somewhere that this should be used at least once before
// rendering, otherwise the camera will have an unitinitialized or underfined
// starting pose.
// TODO: Possibly provide facility for a different assumed home basis of i,j,k
// TODO: Provide for using a column-major camera_transform
RTresult RTAPI rthCameraSetPoseMatrix4x4(
  RTprogram            ray_gen_prog,
  const float*         camera_transform);

/**
 * Sets the camera pose via its eye position, "screen-left" (u) axis,
 * "screen-up" axis (v), and "screen-out" axis (w).  Each axis vector should 
 * be pre-normalized.
 *
 * The camera will look along w.
 */
RTresult RTAPI rthCameraSetPoseEyeUVW(
  RTprogram             ray_gen_program,
  const float           eye[3],
  const float           u[3],
  const float           v[3],
  const float           w[3]);

/* Output writer programs for ray generators */

/**
 * \inGroup rthOutputWriter
 *
 * // TODO: document exact format this will write in
 */
RTresult RTAPI rthOutputWriterByte4RGB(
  RTcontext             context,
  RTprogram*            writer_prog);

/**
 * \inGroup rthOutputWriter
 */
RTresult RTAPI rthOutputWriterFloat3RGB(
  RTcontext             context,
  RTprogram*            writer_prog);

/**
 * Attaches an image output buffer to the given OutputWriter program.
 *
 * The buffer must have the same format as what the OutputWriter writes.  I.e.
 * if the OutputWriter writes uchar4 entries, the buffer entries must be
 * uchar4
 * // TODO: Consider designing and implementing some checking/safety behaviors
 *          for this requirement, rather than leaving it at undefined bechavior.
 */
RTresult RTAPI rthOutputWriterSetOutputBuffer(
  RTprogram             output_writer_program,
  RTbuffer              buffer);

/* Materials */

/* Any hit programs are not specified by the app. They are implied by the render mode (for shadow rays) and the material. */

/* Map the surface normal directly to the output color (for debugging and performance testing) */
RTresult RTAPI rthMaterialNormalToColor(
  RTcontext            context,
  RTmaterial*          material);

/**
 * The following parameters can be set by way of OptiX variables of the same
 * name on the returned material.  Default values are shown here.
 * 
 *     TODO: rename to index_of_refraction
 *     TODO: Document these parameters better
 *     float refraction_index = 1.4f
 *     float fresnel_exponent = 4.0f
 *     float fresnel_minimum = 0.1f
 *     float fresnel_maximum = 1.0f
 *     float extinction[3] = {0.0f, 0.0f, 0.0f}
 *     float shadow_attenuation[3] = {1.0f, 1.0f, 1.0f}
 *     float importance_cutoff = 0.01f
 *     float cutoff_color[3] = {0.2f, 0.2f, 0.2f}
 *     int refraction_maxdepth = 10
 *     int reflection_maxdepth = 5
 */
RTresult RTAPI rthMaterialGlass(
  RTcontext            context,
  RTmaterial*          material,
  float                refraction_color[3],
  float                reflection_color[3]);
  /* TODO: Add: The reflection_color and refraction_color both overridden by the
   * vertex color, if there is one. */
  /* TODO: For this and all other API functions, push the return value pointer
   * to the last function parameter, for consistency with rt* API calls. */

/** 
 * Note: the returned material's closest hit program assumes the
 * RadianceRayPayload's 'importance' and 'depth' fields have been initialized
 * before rtTrace() is called.  In the partially implemented state of this API,
 * which doesn't yet offer ray generation programs, this means setting
 * 'importance' and 'depth' is the responsibility of user-written ray generation
 * programs
 */
RTresult RTAPI rthMaterialPhong(
  RTcontext            context,
  RTmaterial*          material,
  float                ambient[3],
  float                diffuse[3],   // How to specify texture vs. not?
  float                specular[3],
  float                reflectivity[3],
  float                specular_exponent);

/* Lights */

// TODO: Make function names more clear for what exactly they do
// (for example, rthRayGenerationCreate*() )
RTresult RTAPI rthPointLightBuffer(
  RTcontext            context,
  RTbuffer*            lights,
  unsigned int         count);

/* Other */

/**
 * Creates an exception program.  The returned exception_program should later
 * have an OutputWriter callable program assigned; this can be done using
 * rthSetOutputWriter().  This call does not attach the exception program
 * to any context: that will need to be done in client code.
 */
RTresult RTAPI rthException(
  RTcontext            context,
  RTprogram*           exception_program,
  float                exception_color[3]);


/* How to set up the right acceleration structure over the scene without additional API calls? */

/* Common functions */


/**
 * Assign an OutputWriter callable program to a Program that requires one, such
 * as a ray generation or exception program.
 */
RTresult RTAPI rthSetOutputWriter(
  RTprogram             program,
  RTprogram             output_writer_program);


#endif /* _optixu_optix_high_level.h */
