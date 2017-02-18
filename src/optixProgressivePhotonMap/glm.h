/*    
 *  GLM library.  Wavefront .obj file format reader/writer/manipulator.
 *
 *  Written by Nate Robins, 1997.
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 */

#ifndef RTRT_GLM_H
#define RTRT_GLM_H

/* includes */

/* defines */
#if 0
#define GLM_NONE     (0)    /* render with only vertices */
#define GLM_FLAT     (1 << 0)    /* render with facet normals */
#define GLM_SMOOTH   (1 << 1)    /* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)    /* render with texture coords */
#define GLM_COLOR    (1 << 3)    /* render with colors */
#define GLM_MATERIAL (1 << 4)    /* render with materials */
#endif

enum {
  GLM_NONE           = (0),
  GLM_FLAT           = (1 << 0),
  GLM_SMOOTH         = (1 << 1),
  GLM_TEXTURE        = (1 << 2),
  GLM_COLOR          = (1 << 3),
  GLM_MATERIAL       = (1 << 4),
  GLM_FLAT_SHADE     = (1 << 5),
  GLM_SPECULAR_SHADE = (1 << 6)
};

enum Sizes {
  MaxStringLength    = 128
};

/* structs */

/* GLMmaterial: Structure that defines a material in a model. 
*/
typedef struct _GLMmaterial
{
  char* name;        /* name of material */
  float diffuse[4];       // Kd diffuse component
  float ambient[4];       // Ka ambient component
  float specular[4];      // Ks specular component
  float emissive[4];      // emissive component
  float shininess;        // Ns specular exponent
  float refraction;       // Tr
  float alpha;            // d
  float reflectivity;     // reflection
  int   shader;           // illum

  // Texture maps, zero length if not specified.
  char  ambient_map [MaxStringLength]; // map_Ka
  char  diffuse_map [MaxStringLength]; // map_Kd
  char  specular_map[MaxStringLength]; // map_Ks
  char  dissolve_map[MaxStringLength]; // map_D

  // Scaling for texture maps (initialized to 0 for not set)
  float ambient_map_scaling[2];
  float diffuse_map_scaling[2];
  float specular_map_scaling[2];
  float dissolve_map_scaling[2];

} GLMmaterial;

/* GLMtriangle: Structure that defines a triangle in a model.
*/
typedef struct {
  unsigned int vindices[3];    /* array of triangle vertex indices */
  unsigned int nindices[3];    /* array of triangle normal indices */
  unsigned int tindices[3];    /* array of triangle texcoord indices*/
  unsigned int findex;        /* index of triangle facet normal */
} GLMtriangle;

/* GLMgroup: Structure that defines a group in a model.
*/
typedef struct _GLMgroup {
  char*             name;    /* name of this group */
  unsigned int      numtriangles;  /* number of triangles in this group */
  unsigned int*     triangles;          /* array of triangle indices */
  unsigned int      material;           /* index to material for group */
  char*             mtlname; /*name of the material for this group*/
  struct _GLMgroup* next;    /* pointer to next group in model */
} GLMgroup;

/* GLMmodel: Structure that defines a model.
*/
typedef struct {
  char*    pathname;      /* path to this model */
  char*    mtllibname;      /* name of the material library */

  unsigned int   numvertices;    /* number of vertices in model */
  float* vertices;      /* array of vertices
                                           [x1,y1,z1,x2,y2,z2...] */
  unsigned char* vertexColors;          /* array of vertex colors */

  unsigned int   numnormals;    /* number of normals in model */
  float* normals;      /* array of normals */

  unsigned int   numtexcoords;    /* number of texcoords in model */
  float* texcoords;      /* array of texture coordinates */

  unsigned int   numfacetnorms;    /* number of facetnorms in model */
  float* facetnorms;      /* array of facetnorms */

  unsigned int       numtriangles;  /* number of triangles in model */
  GLMtriangle* triangles;    /* array of triangles */

  unsigned int       nummaterials;  /* number of materials in model */
  GLMmaterial* materials;    /* array of materials */

  /* This is the thing you will want to iterate over.  Each group has
     an associated list of triangles, material, and name. */
  unsigned int       numgroups;    /* number of groups in model */
  GLMgroup*    groups;      /* linked list of groups */

  float position[3];      /* position of the model */

  bool usePerVertexColors;             /* Are there per vertex colors? */

} GLMmodel;


/* public functions */

/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
float
glmUnitize(GLMmodel* model);

/*
 * glmBoundingBox: Calculates the min/max positions of the model
 */
void
glmBoundingBox(GLMmodel *model, float *minpos, float *maxpos);


/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 floats (float dimensions[3])
 */
void
glmDimensions(GLMmodel* model, float* dimensions);

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
void
glmScale(GLMmodel* model, float scale);

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 * 
 * model - properly initialized GLMmodel structure 
 */
void
glmReverseWinding(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
void
glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
void
glmVertexNormals(GLMmodel* model, float angle);

/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
void
glmLinearTexture(GLMmodel* model);

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
void
glmSpheremapTexture(GLMmodel* model);

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
void glmDelete(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 *
 * returns 0 if there was a problem reading the file.
 */
GLMmodel* glmReadOBJ(const char* filename);

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE    -  write only vertices
 *            GLM_FLAT    -  write facet normals
 *            GLM_SMOOTH  -  write vertex normals
 *            GLM_TEXTURE -  write texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 *
 * returns 1 if there was error, 0 otherwise.
 * 
 */
int
glmWriteOBJ(GLMmodel* model, char* filename, unsigned int mode);

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
void
glmWeld(GLMmodel* model, float epsilon);


#endif 
