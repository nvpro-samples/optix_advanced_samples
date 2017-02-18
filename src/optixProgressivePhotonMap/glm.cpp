/*    
 *  GLM library.  Wavefront .obj file format reader/writer/manipulator.
 *
 *  Written by Nate Robins, 1997.
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 */


/* includes */
#include "glm.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <climits>
#include <stdexcept>
#include <string>

/* defines */
#define T(x) model->triangles[(x)]

static const char* default_group_name = "No Group";
static const char* default_material_name = "No Material";


/* enums */
enum { X, Y, Z, W }; /* elements of a vertex */


/* typedefs */

/* _GLMnode: general purpose node
*/
typedef struct _GLMnode {
  unsigned int     index;
  bool        averaged;
  struct _GLMnode* next;
} GLMnode;


/* private functions */

/* _glmMax: returns the maximum of two floats */

  static float
_glmMax(float a, float b) 
{
  if (a > b)
    return a;
  return b;
}

/* _glmAbs: returns the absolute value of a float */
  static float
_glmAbs(float f)
{
  if (f < 0)
    return -f;
  return f;
}

/* _glmDot: compute the dot product of two vectors
 *
 * u - array of 3 floats (float u[3])
 * v - array of 3 floats (float v[3])
 */
  static float
_glmDot(float* u, float* v)
{
  assert(u);
  assert(v);

  /* compute the dot product */
  return u[X] * v[X] + u[Y] * v[Y] + u[Z] * v[Z];
}

/* _glmCross: compute the cross product of two vectors
 *
 * u - array of 3 floats (float u[3])
 * v - array of 3 floats (float v[3])
 * n - array of 3 floats (float n[3]) to return the cross product in
 */
  static void
_glmCross(float* u, float* v, float* n)
{
  assert(u);
  assert(v);
  assert(n);

  /* compute the cross product (u x v for right-handed [ccw]) */
  n[X] = u[Y] * v[Z] - u[Z] * v[Y];
  n[Y] = u[Z] * v[X] - u[X] * v[Z];
  n[Z] = u[X] * v[Y] - u[Y] * v[X];
}

/* _glmNormalize: normalize a vector
 *
 * n - array of 3 floats (float n[3]) to be normalized
 */
  static void
_glmNormalize(float* n)
{
  float l;

  assert(n);

  /* normalize */
  l = 1.0f/sqrtf(n[X] * n[X] + n[Y] * n[Y] + n[Z] * n[Z]);
  n[0] *= l;
  n[1] *= l;
  n[2] *= l;
}

/* _glmEqual: compares two vectors and returns true if they are
 * equal (within a certain threshold) or false if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 floats (float u[3])
 * v - array of 3 floats (float v[3]) 
 */
  static bool
_glmEqual(float* u, float* v, float epsilon)
{
  if (_glmAbs(u[0] - v[0]) < epsilon &&
      _glmAbs(u[1] - v[1]) < epsilon &&
      _glmAbs(u[2] - v[2]) < epsilon) 
  {
    return true;
  }
  return false;
}

/* _glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors    - array of float[3]'s to be welded
 * numvectors - number of float[3]'s in vectors
 * epsilon    - maximum difference between vectors 
 *
 */
  static float*
_glmWeldVectors(float* vectors, unsigned int* numvectors, float epsilon)
{
  float* copies;
  unsigned int   copied;
  unsigned int   i, j;

  copies = (float*)malloc(sizeof(float) * 3 * (*numvectors + 1));
  memcpy(copies, vectors, (sizeof(float) * 3 * (*numvectors + 1)));

  copied = 1;
  for (i = 1; i <= *numvectors; i++) {
    for (j = 1; j <= copied; j++) {
      if (_glmEqual(&vectors[3 * i], &copies[3 * j], epsilon)) {
        goto duplicate;
      }
    }

    /* must not be any duplicates -- add to the copies array */
    copies[3 * copied + 0] = vectors[3 * i + 0];
    copies[3 * copied + 1] = vectors[3 * i + 1];
    copies[3 * copied + 2] = vectors[3 * i + 2];
    j = copied;    /* pass this along for below */
    copied++;

duplicate:
    /* set the first component of this vector to point at the correct
       index into the new copies array */
    vectors[3 * i + 0] = (float)j;
  }

  *numvectors = copied-1;
  return copies;
}

/* _glmFindGroup: Find a group in the model
*/
  static GLMgroup*
_glmFindGroup(GLMmodel* model, char* name)
{
  GLMgroup* group;

  assert(model);

  group = model->groups;
  while(group) {
    if (!strcmp(name, group->name))
      break;
    group = group->next;
  }

  return group;
}

/* _glmAddGroup: Add a group to the model
*/
  static GLMgroup*
_glmAddGroup(GLMmodel* model, char* name)
{
  GLMgroup* group;

  group = _glmFindGroup(model, name);
  if (!group) {
    group = (GLMgroup*)malloc(sizeof(GLMgroup));
    group->name = strdup(name);
    group->material = 0;
    group->mtlname = 0;
    group->numtriangles = 0;
    group->triangles = NULL;
    group->next = model->groups;
    model->groups = group;
    model->numgroups++;
  }


  return group;
}

/* _glmFindGroup: Find a material in the model
*/
  static unsigned int
_glmFindMaterial(GLMmodel* model, char* name)
{
  unsigned int i;

  for (i = 0; i < model->nummaterials; i++) {
    if (model->materials[i].name && !strcmp(model->materials[i].name, name))
      goto found;
  }

  /* didn't find the name, so set it as the default material */
  /* printf("_glmFindMaterial():  can't find material \"%s\".\n", name); */
  i = 0;

found:
  return i;
}


/* _glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * The return value should be free'd.
 */
  static char*
_glmDirName(char* path)
{
  char* dir;
  char* s;
  char* s1;
  char* s2;

  dir = strdup(path);

  s1 = strrchr(dir, '\\');
  s2 = strrchr(dir, '/');

  s = (s1 > s2) ? s1 : s2;

  if (s)
    s[1] = '\0';
  else
    dir[0] = '\0';

  return dir;
}


/* _glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
  static int
_glmReadMTL(GLMmodel* model, char* name)
{
  FILE* file;
  char* dir;
  char* filename;
  char  buf[2048];
  unsigned int nummaterials, i;

  dir = _glmDirName(model->pathname);
  filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
  strcpy(filename, dir);
  strcat(filename, name);
  free(dir);

  /* open the file */
  file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "_glmReadMTL() failed: can't open material file \"%s\".\n",
        filename);
    free(filename);
    return 1;
  }
  free(filename);

  /* count the number of materials in the file */
  nummaterials = 1;
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
      case '#':       /* comment */
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
      case 'n':       /* newmtl */
        fgets(buf, sizeof(buf), file);
        nummaterials++;
        sscanf(buf, "%s %s", buf, buf);
        break;
      default:
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
    }
  }

  rewind(file);

  /* allocate memory for the materials */
  model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
  model->nummaterials = nummaterials;

  /* set the default material */
  for (i = 0; i < nummaterials; i++) {
    model->materials[i].name = NULL;
    model->materials[i].shininess = 0;
    model->materials[i].refraction = 1;
    model->materials[i].alpha      = 1;
    model->materials[i].shader     = GLM_FLAT_SHADE;
    model->materials[i].reflectivity = 0;

    model->materials[i].diffuse[0] = 0.7f;
    model->materials[i].diffuse[1] = 0.7f;
    model->materials[i].diffuse[2] = 0.7f;
    model->materials[i].diffuse[3] = 1.0f;
    model->materials[i].ambient[0] = 0.2f;
    model->materials[i].ambient[1] = 0.2f;
    model->materials[i].ambient[2] = 0.2f;
    model->materials[i].ambient[3] = 1.0f;
    model->materials[i].specular[0] = 0.0f;
    model->materials[i].specular[1] = 0.0f;
    model->materials[i].specular[2] = 0.0f;
    model->materials[i].specular[3] = 1.0f;
    model->materials[i].emissive[0] = 0.0f;
    model->materials[i].emissive[1] = 0.0f;
    model->materials[i].emissive[2] = 0.0f;
    model->materials[i].emissive[3] = 1.0f;

    model->materials[i].ambient_map[0] = '\0';
    model->materials[i].diffuse_map[0] = '\0';
    model->materials[i].specular_map[0] = '\0';
    model->materials[i].dissolve_map[0] = '\0';

    model->materials[i].ambient_map_scaling[0] = 0;
    model->materials[i].ambient_map_scaling[1] = 0;
    model->materials[i].diffuse_map_scaling[0] = 0;
    model->materials[i].diffuse_map_scaling[1] = 0;
    model->materials[i].specular_map_scaling[0] = 0;
    model->materials[i].specular_map_scaling[1] = 0;
    model->materials[i].dissolve_map_scaling[0] = 0;
    model->materials[i].dissolve_map_scaling[1] = 0;
  }
  model->materials[0].name = strdup("NO_ASSIGNED_MATERIAL");

  /* now, read in the data */
  nummaterials = 0;
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
      case '#':       /* comment */
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
      case 'n':       /* newmtl */

        // Make sure the previous material has a name.
        assert( model->materials[nummaterials].name );

        // Read in the new material name.
        fgets(buf, sizeof(buf), file);
        sscanf(buf, "%s %s", buf, buf);
        nummaterials++;
        model->materials[nummaterials].name = strdup(buf);
        break;
      case 'N':
        fscanf(file, "%f", &model->materials[nummaterials].shininess);
        break;
      case 'T': // Tr
        fscanf(file, "%f", &model->materials[nummaterials].refraction);
        break;
      case 'd': // d
        fscanf(file, "%f", &model->materials[nummaterials].alpha);
        break;
      case 'i': // illum
        fscanf(file, "%d", &model->materials[nummaterials].shader);
        break;
      case 'r': // reflectivity
        fscanf(file, "%f", &model->materials[nummaterials].reflectivity);
        break;
      case 'e': // emissive
        fscanf(file, "%f %f %f",
            &model->materials[nummaterials].emissive[0],
            &model->materials[nummaterials].emissive[1],
            &model->materials[nummaterials].emissive[2]);
        break;
      case 'm':
        {
          char* map_name = 0;
          float* scaling = 0;
          // Determine which type of map.
          if (strcmp(buf,"map_Ka")==0) {
            map_name = model->materials[nummaterials].ambient_map;
            scaling = model->materials[nummaterials].ambient_map_scaling;
          } else if (strcmp(buf,"map_Kd")==0) {
            map_name = model->materials[nummaterials].diffuse_map;
            scaling = model->materials[nummaterials].diffuse_map_scaling;
          } else if (strcmp(buf,"map_Ks")==0) {
            map_name = model->materials[nummaterials].specular_map;
            scaling = model->materials[nummaterials].ambient_map_scaling;
          } else if (strcmp(buf,"map_D")==0) {
            map_name = model->materials[nummaterials].dissolve_map;
            scaling = model->materials[nummaterials].dissolve_map_scaling;
          } else {
            // We don't know what kind of map it is, so ignore it
            fprintf(stderr, "Unknown map: \"%s\" found at %s(%d)\n", buf,
                __FILE__, __LINE__);
            break;
          }

          char string_litteral[2048];
          sprintf(string_litteral, "%%%ds", (int)MaxStringLength-1);
          //fprintf(stderr, "string_litteral = %s\n", string_litteral);

          // Check to see if we have scaled textures or not
          fscanf(file, string_litteral, map_name);
          if (strcmp(map_name, "-s") == 0) {
            // pick up the float scaled textures
            fscanf(file, "%f %f", &scaling[0], &scaling[1]);
            // Now the name of the file
            fscanf(file, string_litteral, map_name);
            //fprintf(stderr, "      scale = %f %f ,", scaling[0], scaling[1]);
          }
          //fprintf(stderr, "name = %s\n", map_name);
        } // end case 'm'
        break;

      case 'K':
        switch(buf[1]) {
          case 'd':
            fscanf(file, "%f %f %f",
                &model->materials[nummaterials].diffuse[0],
                &model->materials[nummaterials].diffuse[1],
                &model->materials[nummaterials].diffuse[2]);
            break;
          case 's':
            fscanf(file, "%f %f %f",
                &model->materials[nummaterials].specular[0],
                &model->materials[nummaterials].specular[1],
                &model->materials[nummaterials].specular[2]);
            break;
          case 'a':
            fscanf(file, "%f %f %f",
                &model->materials[nummaterials].ambient[0],
                &model->materials[nummaterials].ambient[1],
                &model->materials[nummaterials].ambient[2]);
            break;
          default:
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        }
        break;
      default:
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
    }
  }

  // Make sure we found the same number of materials the second time around.
  // Note that glm adds a default material to the beginning of the array
  assert((nummaterials+1) == model->nummaterials);

  return 0;
}

/* _glmWriteMTL: write a wavefront material library file
 *
 * model      - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
  static int
_glmWriteMTL(GLMmodel* model, char* modelpath, char* mtllibname)
{
  FILE* file;
  char* dir;
  char* filename;
  GLMmaterial* material;
  unsigned int i;

  dir = _glmDirName(modelpath);
  filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(mtllibname)));
  strcpy(filename, dir);
  strcat(filename, mtllibname);
  free(dir);

  /* open the file */
  file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "_glmWriteMTL() failed: can't open file \"%s\".\n",
        filename);
    free(filename);
    return 1;
  }
  free(filename);

  /* spit out a header */
  fprintf(file, "#  \n");
  fprintf(file, "#  Wavefront MTL generated by GLM library\n");
  fprintf(file, "#  \n");
  fprintf(file, "#  GLM library copyright (C) 1997 by Nate Robins\n");
  fprintf(file, "#  email: ndr@pobox.com\n");
  fprintf(file, "#  www:   http://www.pobox.com/~ndr\n");
  fprintf(file, "#  \n\n");

  for (i = 0; i < model->nummaterials; i++) {
    material = &model->materials[i];
    fprintf(file, "newmtl %s\n", material->name);
    fprintf(file, "Ka %f %f %f\n", 
        material->ambient[0], material->ambient[1], material->ambient[2]);
    fprintf(file, "Kd %f %f %f\n", 
        material->diffuse[0], material->diffuse[1], material->diffuse[2]);
    fprintf(file, "Ks %f %f %f\n", 
        material->specular[0],material->specular[1],material->specular[2]);
    fprintf(file, "Ns %f\n", material->shininess);
    fprintf(file, "\n");
  }
  return 0;
}

/* _glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 */
  static int
_glmFirstPass(GLMmodel* model, FILE* file) 
{
  unsigned int    numvertices;    /* number of vertices in model */
  unsigned int    numnormals;     /* number of normals in model */
  unsigned int    numtexcoords;   /* number of texcoords in model */
  unsigned int    numtriangles;   /* number of triangles in model */



  GLMgroup* group;      /* current group */
  char*     current_group_base_name = strdup(default_group_name);
  char*     current_material_name = strdup(default_material_name);
  int       v, n, t;
  char      buf[2048];

  /* make a default group */
  group = _glmAddGroup(model, current_group_base_name);

  numvertices = numnormals = numtexcoords = numtriangles = 0;

  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
      case '#':     /* comment */
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
      case 'v':     /* v, vn, vt */
        switch(buf[1]) {
          case '\0':  { /* vertex */
                        /* eat up rest of line */
                        fgets(buf, sizeof(buf), file);

                        // TODO: Check if colors for this vertex
                        float vx,vy,vz;
                        int   val = -1;
                        sscanf(buf,"%f %f %f %d",&vx, &vy, &vz, &val);
                        if (val >= 0) {
                          model->usePerVertexColors = true;
                        }

                        numvertices++;
                        break;
                      }
          case 'n':   /* normal */
                      /* eat up rest of line */
                      fgets(buf, sizeof(buf), file);
                      numnormals++;
                      break;
          case 't':   /* texcoord */
                      /* eat up rest of line */
                      fgets(buf, sizeof(buf), file);
                      numtexcoords++;
                      break;
          default:
                      printf("_glmFirstPass(): Unknown token \"%s\".\n", buf);
                      /* Could error out here, but we'll just skip it for now.*/
                      /* return 1; */
                      break;
        }
        break;
      case 'm':
        fgets(buf, sizeof(buf), file);
        sscanf(buf, "%s %s", buf, buf);
        model->mtllibname = strdup(buf);
        if (_glmReadMTL(model, buf)) {

          break; /* Dont bail if MTL file not found */

          /* Uh oh.  Trouble reading in the material file. */
          if (current_group_base_name) free(current_group_base_name);
          if (current_material_name)   free(current_material_name);
          return 1;
        }
        break;
      case 'u':
        /* We need to create groups with their own materials */
        fgets(buf, sizeof(buf), file);
        sscanf(buf, "%s %s", buf, buf);
        if (current_material_name) free(current_material_name);
        current_material_name = strdup(buf);
        sprintf(buf, "%s_MAT_%s",
            current_group_base_name, current_material_name);
        group = _glmAddGroup(model, buf);
        break;
      case 'o':
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
      case 'g':       /* group */
        fgets(buf, sizeof(buf), file);
        sscanf(buf, "%s", buf);
        if (current_group_base_name) free(current_group_base_name);
        current_group_base_name = strdup(buf);
        sprintf(buf, "%s_MAT_%s",
            current_group_base_name, current_material_name);
        group = _glmAddGroup(model, buf);
        break;
      case 'f':       /* face */
        v = n = t = 0;
        fscanf(file, "%s", buf);
        /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
        if (strstr(buf, "//")) {
          /* v//n */
          sscanf(buf, "%d//%d", &v, &n);  
          fscanf(file, "%d//%d", &v, &n); 
          fscanf(file, "%d//%d", &v, &n); 
          numtriangles++;
          group->numtriangles++;
          while(fscanf(file, "%d//%d", &v, &n) > 0) {
            numtriangles++;
            group->numtriangles++;
          }
        } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) { 
          /* v/t/n */
          fscanf(file, "%d/%d/%d", &v, &t, &n); 
          fscanf(file, "%d/%d/%d", &v, &t, &n); 
          numtriangles++;
          group->numtriangles++;
          while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
            numtriangles++;
            group->numtriangles++;
          }
        } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
          /* v/t */
          fscanf(file, "%d/%d", &v, &t); 
          fscanf(file, "%d/%d", &v, &t); 
          numtriangles++;
          group->numtriangles++;
          while(fscanf(file, "%d/%d", &v, &t) > 0) {
            numtriangles++;
            group->numtriangles++;
          }
        } else {
          /* v */
          fscanf(file, "%d", &v);
          fscanf(file, "%d", &v);
          numtriangles++;
          group->numtriangles++;
          while(fscanf(file, "%d", &v) > 0) {
            numtriangles++;
            group->numtriangles++;
          }
        }
        break;

      default:
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
    }
  }

  // #if 0
  /* announce the model statistics */
  // printf(" Vertices: %d\n", numvertices);
  // printf(" Normals: %d\n", numnormals);
  // printf(" Texcoords: %d\n", numtexcoords);
  // printf(" Triangles: %d\n", numtriangles);
  // printf(" Groups: %d\n", model->numgroups);
  // #endif

  /* set the stats in the model structure */
  model->numvertices  = numvertices;
  model->numnormals   = numnormals;
  model->numtexcoords = numtexcoords;
  model->numtriangles = numtriangles;

  /* allocate memory for the triangles in each group */
  group = model->groups;
  while(group) {
    group->triangles = (unsigned int*)malloc(sizeof(unsigned int) * group->numtriangles);
    group->numtriangles = 0;
    group = group->next;
  }

  if (current_group_base_name) free(current_group_base_name);
  if (current_material_name)   free(current_material_name);
  return 0;
}

/* _glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 */
  static void
_glmSecondPass(GLMmodel* model, FILE* file) 
{
  unsigned int    numvertices;    /* number of vertices in model */
  unsigned int    numnormals;     /* number of normals in model */
  unsigned int    numtexcoords;   /* number of texcoords in model */
  unsigned int    numtriangles;   /* number of triangles in model */
  float*  vertices;     /* array of vertices  */
  unsigned char*  vertexColors;       /* array of vertex colors */
  float*  normals;      /* array of normals */
  float*  texcoords;     /* array of texture coordinates */
  GLMgroup* group;      /* current group pointer */
  unsigned int    material;     /* current material */
  char*           grpname;      /* current group base name */
  char*           mtlname;      /* current material name */
  int     v, n, t;
  char      buf[2048];

  /* set the pointer shortcuts */
  vertices     = model->vertices;
  vertexColors = model->vertexColors;
  normals      = model->normals;
  texcoords    = model->texcoords;
  group        = model->groups;

  /* on the second pass through the file, read all the data into the
     allocated arrays */
  numvertices = numnormals = numtexcoords = 1;
  numtriangles = 0;
  material = 0;
  grpname = strdup(default_group_name);
  mtlname = strdup(default_material_name);
  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {
      case '#':       /* comment */
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
      case 'v':       /* v, vn, vt */
        switch(buf[1]) {
          case '\0':      /* vertex */
            if (!model->usePerVertexColors) {
              fscanf(file, "%f %f %f", 
                  &vertices[3 * numvertices + X], 
                  &vertices[3 * numvertices + Y], 
                  &vertices[3 * numvertices + Z]);
            }
            else {
              int r,g,b;
              fscanf(file, "%f %f %f %d %d %d", 
                  &vertices[3 * numvertices + X], 
                  &vertices[3 * numvertices + Y], 
                  &vertices[3 * numvertices + Z], &r, &g, &b);
              vertexColors[3 * numvertices + X] = (unsigned char)r; 
              vertexColors[3 * numvertices + Y] = (unsigned char)g; 
              vertexColors[3 * numvertices + Z] = (unsigned char)b;
            }
            numvertices++;
            break;
          case 'n':       /* normal */
            fscanf(file, "%f %f %f", 
                &normals[3 * numnormals + X],
                &normals[3 * numnormals + Y], 
                &normals[3 * numnormals + Z]);
            numnormals++;
            break;
          case 't':       /* texcoord */
            fscanf(file, "%f %f", 
                &texcoords[2 * numtexcoords + X],
                &texcoords[2 * numtexcoords + Y]);
            numtexcoords++;
            break;
        }
        break;
      case 'u': // usemtl
        fgets(buf, sizeof(buf), file);
        sscanf(buf, "%s %s", buf, buf);
        if (mtlname) free(mtlname);
        mtlname  = strdup(buf);
        material = _glmFindMaterial(model, buf);
        sprintf(buf, "%s_MAT_%s",
            grpname, mtlname);
        group = _glmFindGroup(model, buf);
        group->material = material;
        if (group->mtlname) free(group->mtlname);
        group->mtlname = strdup(mtlname);
        break;
      case 'o': 
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
      case 'g':       /* group */
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        sscanf(buf, "%s", buf);
        if (grpname) free(grpname);
        grpname = strdup(buf);
        sprintf(buf, "%s_MAT_%s",
            grpname, mtlname);
        group = _glmFindGroup(model, buf);
        group->material = material;
        if (group->mtlname) free(group->mtlname);
        group->mtlname = strdup(mtlname);
        break;
      case 'f':       /* face */
        v = n = t = 0;
        fscanf(file, "%s", buf);
        /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
        if (strstr(buf, "//")) {
          /* v//n */
          sscanf(buf, "%d//%d", &v, &n);
          T(numtriangles).vindices[0] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[0] = n;
          T(numtriangles).tindices[0] = 0;
          fscanf(file, "%d//%d", &v, &n);
          T(numtriangles).vindices[1] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[1] = n;
          T(numtriangles).tindices[1] = 0;
          fscanf(file, "%d//%d", &v, &n);
          T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[2] = n;
          T(numtriangles).tindices[2] = 0;
          group->triangles[group->numtriangles++] = numtriangles;
          numtriangles++;
          while(fscanf(file, "%d//%d", &v, &n) > 0) {
            T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
            T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
            T(numtriangles).tindices[0] = 0;
            T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
            T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
            T(numtriangles).tindices[1] = 0;
            T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
            T(numtriangles).nindices[2] = n;
            T(numtriangles).tindices[2] = 0;
            group->triangles[group->numtriangles++] = numtriangles;
            numtriangles++;
          }
        } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
          /* v/t/n */
          T(numtriangles).vindices[0] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[0] = n;
          T(numtriangles).tindices[0] = t;
          fscanf(file, "%d/%d/%d", &v, &t, &n);
          T(numtriangles).vindices[1] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[1] = n;
          T(numtriangles).tindices[1] = t;
          fscanf(file, "%d/%d/%d", &v, &t, &n);
          T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[2] = n;
          T(numtriangles).tindices[2] = t;
          group->triangles[group->numtriangles++] = numtriangles;
          numtriangles++;
          while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
            T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
            T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
            T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
            T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
            T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
            T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
            T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
            T(numtriangles).nindices[2] = n;
            T(numtriangles).tindices[2] = t;
            group->triangles[group->numtriangles++] = numtriangles;
            numtriangles++;
          }
        } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
          /* v/t */
          T(numtriangles).vindices[0] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[0] = 0;
          T(numtriangles).tindices[0] = t;
          fscanf(file, "%d/%d", &v, &t);
          T(numtriangles).vindices[1] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[1] = 0;
          T(numtriangles).tindices[1] = t;
          fscanf(file, "%d/%d", &v, &t);
          T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[2] = 0;
          T(numtriangles).tindices[2] = t;
          group->triangles[group->numtriangles++] = numtriangles;
          numtriangles++;
          while(fscanf(file, "%d/%d", &v, &t) > 0) {
            T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
            T(numtriangles).nindices[0] = 0;
            T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
            T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
            T(numtriangles).nindices[1] = 0;
            T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
            T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
            T(numtriangles).nindices[2] = 0;
            T(numtriangles).tindices[2] = t;
            group->triangles[group->numtriangles++] = numtriangles;
            numtriangles++;
          }
        } else {
          /* v */
          sscanf(buf, "%d", &v);
          T(numtriangles).vindices[0] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[0] = 0;
          T(numtriangles).tindices[0] = 0;
          fscanf(file, "%d", &v);
          T(numtriangles).vindices[1] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[1] = 0;
          T(numtriangles).tindices[1] = 0;
          fscanf(file, "%d", &v);
          T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
          T(numtriangles).nindices[2] = 0;
          T(numtriangles).tindices[2] = 0;
          group->triangles[group->numtriangles++] = numtriangles;
          numtriangles++;
          while(fscanf(file, "%d", &v) > 0) {
            T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
            T(numtriangles).nindices[0] = 0;
            T(numtriangles).tindices[0] = 0;
            T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
            T(numtriangles).nindices[1] = 0;
            T(numtriangles).tindices[1] = 0;
            T(numtriangles).vindices[2] = (v >= 0) ? v : (numvertices + v);
            T(numtriangles).nindices[2] = 0;
            T(numtriangles).tindices[2] = 0;
            group->triangles[group->numtriangles++] = numtriangles;
            numtriangles++;
          }
        }
        break;

      default:
        /* eat up rest of line */
        fgets(buf, sizeof(buf), file);
        break;
    }
  } 

#if 0
  /* announce the memory requirements */
  printf(" Memory: %d bytes\n",
      numvertices  * 3*sizeof(float) +
      numnormals   * 3*sizeof(float) * (numnormals ? 1 : 0) +
      numtexcoords * 3*sizeof(float) * (numtexcoords ? 1 : 0) +
      numtriangles * sizeof(GLMtriangle));
#endif
  if (grpname) free(grpname);
  if (mtlname) free(mtlname);
}




/* public functions */

/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
  float
glmUnitize(GLMmodel* model)
{
  unsigned int  i;
  float maxx, minx, maxy, miny, maxz, minz;
  float cx, cy, cz, w, h, d;
  float scale;

  assert(model);
  assert(model->vertices);

  /* get the max/mins */
  maxx = minx = model->vertices[3 + X];
  maxy = miny = model->vertices[3 + Y];
  maxz = minz = model->vertices[3 + Z];
  for (i = 1; i <= model->numvertices; i++) {
    if (maxx < model->vertices[3 * i + X])
      maxx = model->vertices[3 * i + X];
    if (minx > model->vertices[3 * i + X])
      minx = model->vertices[3 * i + X];

    if (maxy < model->vertices[3 * i + Y])
      maxy = model->vertices[3 * i + Y];
    if (miny > model->vertices[3 * i + Y])
      miny = model->vertices[3 * i + Y];

    if (maxz < model->vertices[3 * i + Z])
      maxz = model->vertices[3 * i + Z];
    if (minz > model->vertices[3 * i + Z])
      minz = model->vertices[3 * i + Z];
  }

  /* calculate model width, height, and depth */
  w = _glmAbs(maxx) + _glmAbs(minx);
  h = _glmAbs(maxy) + _glmAbs(miny);
  d = _glmAbs(maxz) + _glmAbs(minz);

  /* calculate center of the model */
  cx = (maxx + minx) / 2.0f;
  cy = (maxy + miny) / 2.0f;
  cz = (maxz + minz) / 2.0f;

  /* calculate unitizing scale factor */
  scale = 2.0f / _glmMax(_glmMax(w, h), d);

  /* translate around center then scale */
  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i + X] -= cx;
    model->vertices[3 * i + Y] -= cy;
    model->vertices[3 * i + Z] -= cz;
    model->vertices[3 * i + X] *= scale;
    model->vertices[3 * i + Y] *= scale;
    model->vertices[3 * i + Z] *= scale;
  }

  return scale;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 floats (float dimensions[3])
 */
  void
glmDimensions(GLMmodel* model, float* dimensions)
{
  unsigned int i;
  float maxx, minx, maxy, miny, maxz, minz;

  assert(model);
  assert(model->vertices);
  assert(dimensions);

  /* get the max/mins */
  maxx = minx = model->vertices[3 + X];
  maxy = miny = model->vertices[3 + Y];
  maxz = minz = model->vertices[3 + Z];
  for (i = 1; i <= model->numvertices; i++) {
    if (maxx < model->vertices[3 * i + X])
      maxx = model->vertices[3 * i + X];
    if (minx > model->vertices[3 * i + X])
      minx = model->vertices[3 * i + X];

    if (maxy < model->vertices[3 * i + Y])
      maxy = model->vertices[3 * i + Y];
    if (miny > model->vertices[3 * i + Y])
      miny = model->vertices[3 * i + Y];

    if (maxz < model->vertices[3 * i + Z])
      maxz = model->vertices[3 * i + Z];
    if (minz > model->vertices[3 * i + Z])
      minz = model->vertices[3 * i + Z];
  }

  /* calculate model width, height, and depth */
  dimensions[X] = _glmAbs(maxx) + _glmAbs(minx);
  dimensions[Y] = _glmAbs(maxy) + _glmAbs(miny);
  dimensions[Z] = _glmAbs(maxz) + _glmAbs(minz);
}

/*
 * glmBoundingBox: Calculates the min/max positions of the model
 */
  void
glmBoundingBox(GLMmodel *model, float *minpos, float *maxpos)
{
  unsigned int i;
  float maxx, minx, maxy, miny, maxz, minz;

  assert(model);
  assert(model->vertices);
  assert(minpos);
  assert(maxpos);

  /* get the max/mins */
  maxx = minx = model->vertices[3 + X];
  maxy = miny = model->vertices[3 + Y];
  maxz = minz = model->vertices[3 + Z];
  for (i = 1; i <= model->numvertices; i++) {
    if (maxx < model->vertices[3 * i + X])
      maxx = model->vertices[3 * i + X];
    if (minx > model->vertices[3 * i + X])
      minx = model->vertices[3 * i + X];

    if (maxy < model->vertices[3 * i + Y])
      maxy = model->vertices[3 * i + Y];
    if (miny > model->vertices[3 * i + Y])
      miny = model->vertices[3 * i + Y];

    if (maxz < model->vertices[3 * i + Z])
      maxz = model->vertices[3 * i + Z];
    if (minz > model->vertices[3 * i + Z])
      minz = model->vertices[3 * i + Z];
  }

  minpos[0] = minx;
  minpos[1] = miny;
  minpos[2] = minz;
  maxpos[0] = maxx;
  maxpos[1] = maxy;
  maxpos[2] = maxz;

}

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
  void
glmScale(GLMmodel* model, float scale)
{
  unsigned int i;

  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i + X] *= scale;
    model->vertices[3 * i + Y] *= scale;
    model->vertices[3 * i + Z] *= scale;
  }
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 * 
 * model - properly initialized GLMmodel structure 
 */
  void
glmReverseWinding(GLMmodel* model)
{
  unsigned int i, swap;

  assert(model);

  for (i = 0; i < model->numtriangles; i++) {
    swap = T(i).vindices[0];
    T(i).vindices[0] = T(i).vindices[2];
    T(i).vindices[2] = swap;

    if (model->numnormals) {
      swap = T(i).nindices[0];
      T(i).nindices[0] = T(i).nindices[2];
      T(i).nindices[2] = swap;
    }

    if (model->numtexcoords) {
      swap = T(i).tindices[0];
      T(i).tindices[0] = T(i).tindices[2];
      T(i).tindices[2] = swap;
    }
  }

  /* reverse facet normals */
  for (i = 1; i <= model->numfacetnorms; i++) {
    model->facetnorms[3 * i + X] = -model->facetnorms[3 * i + X];
    model->facetnorms[3 * i + Y] = -model->facetnorms[3 * i + Y];
    model->facetnorms[3 * i + Z] = -model->facetnorms[3 * i + Z];
  }

  /* reverse vertex normals */
  for (i = 1; i <= model->numnormals; i++) {
    model->normals[3 * i + X] = -model->normals[3 * i + X];
    model->normals[3 * i + Y] = -model->normals[3 * i + Y];
    model->normals[3 * i + Z] = -model->normals[3 * i + Z];
  }
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
  void
glmFacetNormals(GLMmodel* model)
{
  unsigned int  i;
  float u[3];
  float v[3];

  assert(model);
  assert(model->vertices);

  /* clobber any old facetnormals */
  if (model->facetnorms)
    free(model->facetnorms);

  /* allocate memory for the new facet normals */
  model->numfacetnorms = model->numtriangles;
  model->facetnorms = (float*)malloc(sizeof(float) *
      3 * (model->numfacetnorms + 1));

  for (i = 0; i < model->numtriangles; i++) {
    model->triangles[i].findex = i+1;

    u[X] = model->vertices[3 * T(i).vindices[1] + X] -
      model->vertices[3 * T(i).vindices[0] + X];
    u[Y] = model->vertices[3 * T(i).vindices[1] + Y] -
      model->vertices[3 * T(i).vindices[0] + Y];
    u[Z] = model->vertices[3 * T(i).vindices[1] + Z] -
      model->vertices[3 * T(i).vindices[0] + Z];

    v[X] = model->vertices[3 * T(i).vindices[2] + X] -
      model->vertices[3 * T(i).vindices[0] + X];
    v[Y] = model->vertices[3 * T(i).vindices[2] + Y] -
      model->vertices[3 * T(i).vindices[0] + Y];
    v[Z] = model->vertices[3 * T(i).vindices[2] + Z] -
      model->vertices[3 * T(i).vindices[0] + Z];

    _glmCross(u, v, &model->facetnorms[3 * (i+1)]);
    _glmNormalize(&model->facetnorms[3 * (i+1)]);
  }
}

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
glmVertexNormals(GLMmodel* model, float angle)
{
  GLMnode*  node;
  GLMnode*  tail;
  GLMnode** members;
  float*  normals;
  unsigned int    numnormals;
  float   average[3];
  float   dot, cos_angle;
  unsigned int    i, avg;

  assert(model);
  assert(model->facetnorms);

  /* calculate the cosine of the angle (in degrees) */
  cos_angle = cosf(angle * (float)M_PI / 180.0f);

  /* nuke any previous normals */
  if (model->normals)
    free(model->normals);

  /* allocate space for new normals */
  model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
  model->normals = (float*)malloc(sizeof(float)* 3* (model->numnormals+1));

  /* allocate a structure that will hold a linked list of triangle
     indices for each vertex */
  members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
  for (i = 1; i <= model->numvertices; i++)
    members[i] = NULL;

  /* for every triangle, create a node for each vertex in it */
  for (i = 0; i < model->numtriangles; i++) {
    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[0]];
    members[T(i).vindices[0]] = node;

    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[1]];
    members[T(i).vindices[1]] = node;

    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[2]];
    members[T(i).vindices[2]] = node;
  }

  /* calculate the average normal for each vertex */
  numnormals = 1;
  for (i = 1; i <= model->numvertices; i++) {
    /* calculate an average normal for this vertex by averaging the
       facet normal of every triangle this vertex is in */
    node = members[i];
    if (!node)
      fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
    average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
    avg = 0;
    while (node) {
      /* only average if the dot product of the angle between the two
         facet normals is greater than the cosine of the threshold
         angle -- or, said another way, the angle between the two
         facet normals is less than (or equal to) the threshold angle */
      dot = _glmDot(&model->facetnorms[3 * T(node->index).findex],
          &model->facetnorms[3 * T(members[i]->index).findex]);
      if (dot > cos_angle) {
        node->averaged = true;
        average[0] += model->facetnorms[3 * T(node->index).findex + 0];
        average[1] += model->facetnorms[3 * T(node->index).findex + 1];
        average[2] += model->facetnorms[3 * T(node->index).findex + 2];
        avg = 1;      /* we averaged at least one normal! */
      } else {
        node->averaged = false;
      }
      node = node->next;
    }

    if (avg) {
      /* normalize the averaged normal */
      _glmNormalize(average);

      /* add the normal to the vertex normals list */
      model->normals[3 * numnormals + 0] = average[0];
      model->normals[3 * numnormals + 1] = average[1];
      model->normals[3 * numnormals + 2] = average[2];
      avg = numnormals;
      numnormals++;
    }

    /* set the normal of this vertex in each triangle it is in */
    node = members[i];
    while (node) {
      if (node->averaged) {
        /* if this node was averaged, use the average normal */
        if (T(node->index).vindices[0] == i)
          T(node->index).nindices[0] = avg;
        else if (T(node->index).vindices[1] == i)
          T(node->index).nindices[1] = avg;
        else if (T(node->index).vindices[2] == i)
          T(node->index).nindices[2] = avg;
      } else {
        /* if this node wasn't averaged, use the facet normal */
        model->normals[3 * numnormals + 0] = 
          model->facetnorms[3 * T(node->index).findex + 0];
        model->normals[3 * numnormals + 1] = 
          model->facetnorms[3 * T(node->index).findex + 1];
        model->normals[3 * numnormals + 2] = 
          model->facetnorms[3 * T(node->index).findex + 2];
        if (T(node->index).vindices[0] == i)
          T(node->index).nindices[0] = numnormals;
        else if (T(node->index).vindices[1] == i)
          T(node->index).nindices[1] = numnormals;
        else if (T(node->index).vindices[2] == i)
          T(node->index).nindices[2] = numnormals;
        numnormals++;
      }
      node = node->next;
    }
  }

  model->numnormals = numnormals - 1;

  /* free the member information */
  for (i = 1; i <= model->numvertices; i++) {
    node = members[i];
    while (node) {
      tail = node;
      node = node->next;
      free(tail);
    }
  }
  free(members);

  /* pack the normals array (we previously allocated the maximum
     number of normals that could possibly be created (numtriangles *
     3), so get rid of some of them (usually alot unless none of the
     facet normals were averaged)) */
  normals = model->normals;
  model->normals = (float*)malloc(sizeof(float)* 3* (model->numnormals+1));
  for (i = 1; i <= model->numnormals; i++) {
    model->normals[3 * i + 0] = normals[3 * i + 0];
    model->normals[3 * i + 1] = normals[3 * i + 1];
    model->normals[3 * i + 2] = normals[3 * i + 2];
  }
  free(normals);

  // printf("glmVertexNormals(): %u normals generated\n", model->numnormals);
}


/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
  void
glmLinearTexture(GLMmodel* model)
{
  GLMgroup *group;
  float dimensions[3];
  float x, y, scalefactor;
  unsigned int i;

  assert(model);

  if (model->texcoords)
    free(model->texcoords);
  model->numtexcoords = model->numvertices;
  model->texcoords=(float*)malloc(sizeof(float)*2*(model->numtexcoords+1));

  glmDimensions(model, dimensions);
  scalefactor = 2.0f / 
    _glmAbs(_glmMax(_glmMax(dimensions[0], dimensions[1]), dimensions[2]));

  /* do the calculations */
  for(i = 1; i <= model->numvertices; i++) {
    x = model->vertices[3 * i + 0] * scalefactor;
    y = model->vertices[3 * i + 2] * scalefactor;
    model->texcoords[2 * i + 0] = (x + 1.0f) / 2.0f;
    model->texcoords[2 * i + 1] = (y + 1.0f) / 2.0f;
  }

  /* go through and put texture coordinate indices in all the triangles */
  group = model->groups;
  while(group) {
    for(i = 0; i < group->numtriangles; i++) {
      T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
      T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
      T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
    }    
    group = group->next;
  }

#if 0
  printf("glmLinearTexture(): generated %d linear texture coordinates\n",
      model->numtexcoords);
#endif
}

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
glmSpheremapTexture(GLMmodel* model)
{
  GLMgroup* group;
  float theta, phi, rho, x, y, z, r;
  unsigned int i;

  assert(model);
  assert(model->normals);

  if (model->texcoords)
    free(model->texcoords);
  model->numtexcoords = model->numnormals;
  model->texcoords=(float*)malloc(sizeof(float)*2*(model->numtexcoords+1));

  /* do the calculations */
  for (i = 1; i <= model->numnormals; i++) {
    z = model->normals[3 * i + 0];  /* re-arrange for pole distortion */
    y = model->normals[3 * i + 1];
    x = model->normals[3 * i + 2];
    r = sqrtf((x * x) + (y * y));
    rho = sqrtf((r * r) + (z * z));

    if(r == 0.0f) {
      theta = 0.0f;
      phi = 0.0f;
    } else {
      if(z == 0.0)
        phi = 3.14159265f / 2.0f;
      else
        phi = acosf(z / rho);

#if WE_DONT_NEED_THIS_CODE
      if(x == 0.0f)
        theta = 3.14159265f / 2.0f;  /* asin(y / r); */
      else
        theta = acosf(x / r);
#endif

      if(y == 0.0f)
        theta = 3.141592365f / 2.0f;  /* acos(x / r); */
      else
        theta = asinf(y / r) + (3.14159265f / 2.0f);
    }

    model->texcoords[2 * i + 0] = theta / 3.14159265f;
    model->texcoords[2 * i + 1] = phi / 3.14159265f;
  }

  /* go through and put texcoord indices in all the triangles */
  group = model->groups;
  while(group) {
    for (i = 0; i < group->numtriangles; i++) {
      T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
      T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
      T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
    }
    group = group->next;
  }

#if 0  
  printf("glmSpheremapTexture(): generated %d spheremap texture coordinates\n",
      model->numtexcoords);
#endif
}

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
  void
glmDelete(GLMmodel* model)
{
  GLMgroup* group;
  unsigned int i;

  assert(model);

  if (model->pathname)     free(model->pathname);
  if (model->mtllibname)   free(model->mtllibname);
  if (model->vertices)     free(model->vertices);
  if (model->vertexColors) free(model->vertexColors);
  if (model->normals)      free(model->normals);
  if (model->texcoords)    free(model->texcoords);
  if (model->facetnorms)   free(model->facetnorms);
  if (model->triangles)    free(model->triangles);
  if (model->materials) {
    for (i = 0; i < model->nummaterials; i++)
      if (model->materials[i].name) free(model->materials[i].name);
    free(model->materials);
  }
  while(model->groups) {
    group = model->groups;
    /* Take the group off the linked list. */
    model->groups = model->groups->next;
    if (group->name) free(group->name);
    if (group->triangles) free(group->triangles);
    if (group->mtlname) free(group->mtlname);
    free(group);
  }

  free(model);
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.  
 */
  GLMmodel* 
glmReadOBJ(const char* filename)
{
  GLMmodel* model;
  FILE*     file;

  /* open the file */
  file = fopen(filename, "r");
  if (!file) {
    throw std::runtime_error(std::string("glmReadOBJ() failed: can't open data file: ") + filename);
  }

#if 0
  /* announce the model name */
  printf("Model: %s\n", filename);
#endif

  /* allocate a new model */
  model = (GLMmodel*)malloc(sizeof(GLMmodel));
  model->pathname      = strdup(filename);
  model->mtllibname    = NULL;
  model->numvertices   = 0;
  model->vertices      = NULL;
  model->vertexColors  = NULL;
  model->numnormals    = 0;
  model->normals       = NULL;
  model->numtexcoords  = 0;
  model->texcoords     = NULL;
  model->numfacetnorms = 0;
  model->facetnorms    = NULL;
  model->numtriangles  = 0;
  model->triangles     = NULL;
  model->nummaterials  = 0;
  model->materials     = NULL;
  model->numgroups     = 0;
  model->groups        = NULL;
  model->position[0]   = 0.0;
  model->position[1]   = 0.0;
  model->position[2]   = 0.0;
  model->usePerVertexColors = 0;
  /* make a first pass through the file to get a count of the number
     of vertices, normals, texcoords & triangles */
  if (_glmFirstPass(model, file)) {
    /* There was a problem here, so cleanup and exit. */
    glmDelete(model);
    fclose(file);
    return 0;
  }

  /* allocate memory */
  model->vertices = (float*)malloc(sizeof(float) *
      3 * (model->numvertices + 1));
  model->vertexColors = (unsigned char*)malloc(sizeof(unsigned char) *
      3 * (model->numvertices + 1));
  model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
      model->numtriangles);
  if (model->numnormals) {
    model->normals = (float*)malloc(sizeof(float) *
        3 * (model->numnormals + 1));
  }
  if (model->numtexcoords) {
    model->texcoords = (float*)malloc(sizeof(float) *
        2 * (model->numtexcoords + 1));
  }

  /* rewind to beginning of file and read in the data this pass */
  rewind(file);

  _glmSecondPass(model, file);

  /* close the file */
  fclose(file);

  return model;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.  
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
  int
glmWriteOBJ(GLMmodel* model, char* filename, unsigned int mode)
{
  unsigned int    i;
  FILE*     file;
  GLMgroup* group;

  assert(model);

  /* do a bit of warning */
  if (mode & GLM_FLAT && !model->facetnorms) {
    printf("glmWriteOBJ() warning: flat normal output requested "
        "with no facet normals defined.\n");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_SMOOTH && !model->normals) {
    printf("glmWriteOBJ() warning: smooth normal output requested "
        "with no normals defined.\n");
    mode &= ~GLM_SMOOTH;
  }
  if (mode & GLM_TEXTURE && !model->texcoords) {
    printf("glmWriteOBJ() warning: texture coordinate output requested "
        "with no texture coordinates defined.\n");
    mode &= ~GLM_TEXTURE;
  }
  if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
    printf("glmWriteOBJ() warning: flat normal output requested "
        "and smooth normal output requested (using smooth).\n");
    mode &= ~GLM_FLAT;
  }

  /* open the file */
  file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "glmWriteOBJ() failed: can't open file \"%s\" to write.\n",
        filename);
    return 1;
  }

  /* spit out a header */
  fprintf(file, "#  \n");
  fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
  fprintf(file, "#  \n");
  fprintf(file, "#  GLM library copyright (C) 1997 by Nate Robins\n");
  fprintf(file, "#  email: ndr@pobox.com\n");
  fprintf(file, "#  www:   http://www.pobox.com/~ndr\n");
  fprintf(file, "#  \n");

  if (mode & GLM_MATERIAL && model->mtllibname) {
    fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
    if (_glmWriteMTL(model, filename, model->mtllibname)) {
      /* Problem opening up the material file for output. */
      fclose(file);
      return 1;
    }
  }

  /* spit out the vertices */
  fprintf(file, "\n");
  fprintf(file, "# %u vertices\n", model->numvertices);
  for (i = 1; i <= model->numvertices; i++) {
    fprintf(file, "v %f %f %f\n", 
        model->vertices[3 * i + 0],
        model->vertices[3 * i + 1],
        model->vertices[3 * i + 2]);
  }

  /* spit out the smooth/flat normals */
  if (mode & GLM_SMOOTH) {
    fprintf(file, "\n");
    fprintf(file, "# %u normals\n", model->numnormals);
    for (i = 1; i <= model->numnormals; i++) {
      fprintf(file, "vn %f %f %f\n", 
          model->normals[3 * i + 0],
          model->normals[3 * i + 1],
          model->normals[3 * i + 2]);
    }
  } else if (mode & GLM_FLAT) {
    fprintf(file, "\n");
    fprintf(file, "# %u normals\n", model->numfacetnorms);
    for (i = 1; i <= model->numnormals; i++) {
      fprintf(file, "vn %f %f %f\n", 
          model->facetnorms[3 * i + 0],
          model->facetnorms[3 * i + 1],
          model->facetnorms[3 * i + 2]);
    }
  }

  /* spit out the texture coordinates */
  if (mode & GLM_TEXTURE) {
    fprintf(file, "\n");
    fprintf(file, "# %u texcoords\n", model->numtexcoords);
    for (i = 1; i <= model->numtexcoords; i++) {
      fprintf(file, "vt %f %f\n", 
          model->texcoords[2 * i + 0],
          model->texcoords[2 * i + 1]);
    }
  }

  fprintf(file, "\n");
  fprintf(file, "# %u groups\n", model->numgroups);
  fprintf(file, "# %u faces (triangles)\n", model->numtriangles);
  fprintf(file, "\n");

  group = model->groups;
  while(group) {
    fprintf(file, "g %s\n", group->name);
    if (mode & GLM_MATERIAL)
      fprintf(file, "usemtl %s\n", model->materials[group->material].name);
    for (i = 0; i < group->numtriangles; i++) {
      if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
        fprintf(file, "f %u/%u/%u %u/%u/%u %u/%u/%u\n",
            T(group->triangles[i]).vindices[0], 
            T(group->triangles[i]).nindices[0], 
            T(group->triangles[i]).tindices[0],
            T(group->triangles[i]).vindices[1],
            T(group->triangles[i]).nindices[1],
            T(group->triangles[i]).tindices[1],
            T(group->triangles[i]).vindices[2],
            T(group->triangles[i]).nindices[2],
            T(group->triangles[i]).tindices[2]);
      } else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
        fprintf(file, "f %u/%u %u/%u %u/%u\n",
            T(group->triangles[i]).vindices[0],
            T(group->triangles[i]).findex,
            T(group->triangles[i]).vindices[1],
            T(group->triangles[i]).findex,
            T(group->triangles[i]).vindices[2],
            T(group->triangles[i]).findex);
      } else if (mode & GLM_TEXTURE) {
        fprintf(file, "f %u/%u %u/%u %u/%u\n",
            T(group->triangles[i]).vindices[0],
            T(group->triangles[i]).tindices[0],
            T(group->triangles[i]).vindices[1],
            T(group->triangles[i]).tindices[1],
            T(group->triangles[i]).vindices[2],
            T(group->triangles[i]).tindices[2]);
      } else if (mode & GLM_SMOOTH) {
        fprintf(file, "f %u//%u %u//%u %u//%u\n",
            T(group->triangles[i]).vindices[0],
            T(group->triangles[i]).nindices[0],
            T(group->triangles[i]).vindices[1],
            T(group->triangles[i]).nindices[1],
            T(group->triangles[i]).vindices[2], 
            T(group->triangles[i]).nindices[2]);
      } else if (mode & GLM_FLAT) {
        fprintf(file, "f %u//%u %u//%u %u//%u\n",
            T(group->triangles[i]).vindices[0], 
            T(group->triangles[i]).findex,
            T(group->triangles[i]).vindices[1],
            T(group->triangles[i]).findex,
            T(group->triangles[i]).vindices[2],
            T(group->triangles[i]).findex);
      } else {
        fprintf(file, "f %u %u %u\n",
            T(group->triangles[i]).vindices[0],
            T(group->triangles[i]).vindices[1],
            T(group->triangles[i]).vindices[2]);
      }
    }
    fprintf(file, "\n");
    group = group->next;
  }

  fclose(file);
  return 0;
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
  void
glmWeld(GLMmodel* model, float epsilon)
{
  float* vectors;
  float* copies;
  unsigned int   numvectors;
  unsigned int   i;

  /* vertices */
  numvectors = model->numvertices;
  vectors    = model->vertices;
  copies = _glmWeldVectors(vectors, &numvectors, epsilon);

  printf("glmWeld(): %u redundant vertices.\n", 
      model->numvertices - numvectors - 1);

  for (i = 0; i < model->numtriangles; i++) {
    T(i).vindices[0] = (unsigned int)vectors[3 * T(i).vindices[0] + 0];
    T(i).vindices[1] = (unsigned int)vectors[3 * T(i).vindices[1] + 0];
    T(i).vindices[2] = (unsigned int)vectors[3 * T(i).vindices[2] + 0];
  }

  /* free space for old vertices */
  free(vectors);

  /* allocate space for the new vertices */
  model->numvertices = numvectors;
  model->vertices = (float*)malloc(sizeof(float) * 
      3 * (model->numvertices + 1));

  /* copy the optimized vertices into the actual vertex list */
  for (i = 1; i <= model->numvertices; i++) {
    model->vertices[3 * i + 0] = copies[3 * i + 0];
    model->vertices[3 * i + 1] = copies[3 * i + 1];
    model->vertices[3 * i + 2] = copies[3 * i + 2];
  }

  free(copies);
}

#if 0   /** This is left in only as a reference to how to get to the data. */
/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.  
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
  GLvoid
glmDraw(GLMmodel* model, unsigned int mode)
{
  unsigned int i;
  GLMgroup* group;

  assert(model);
  assert(model->vertices);

  /* do a bit of warning */
  if (mode & GLM_FLAT && !model->facetnorms) {
    printf("glmDraw() warning: flat render mode requested "
        "with no facet normals defined.\n");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_SMOOTH && !model->normals) {
    printf("glmDraw() warning: smooth render mode requested "
        "with no normals defined.\n");
    mode &= ~GLM_SMOOTH;
  }
  if (mode & GLM_TEXTURE && !model->texcoords) {
    printf("glmDraw() warning: texture render mode requested "
        "with no texture coordinates defined.\n");
    mode &= ~GLM_TEXTURE;
  }
  if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
    printf("glmDraw() warning: flat render mode requested "
        "and smooth render mode requested (using smooth).\n");
    mode &= ~GLM_FLAT;
  }
  if (mode & GLM_COLOR && !model->materials) {
    printf("glmDraw() warning: color render mode requested "
        "with no materials defined.\n");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_MATERIAL && !model->materials) {
    printf("glmDraw() warning: material render mode requested "
        "with no materials defined.\n");
    mode &= ~GLM_MATERIAL;
  }
  if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
    printf("glmDraw() warning: color and material render mode requested "
        "using only material mode\n");
    mode &= ~GLM_COLOR;
  }
  if (mode & GLM_COLOR)
    glEnable(GL_COLOR_MATERIAL);
  if (mode & GLM_MATERIAL)
    glDisable(GL_COLOR_MATERIAL);

  glPushMatrix();
  glTranslatef(model->position[0], model->position[1], model->position[2]);

  glBegin(GL_TRIANGLES);
  group = model->groups;
  while (group) {
    if (mode & GLM_MATERIAL) {
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
          model->materials[group->material].ambient);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
          model->materials[group->material].diffuse);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
          model->materials[group->material].specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 
          model->materials[group->material].shininess);
    }

    if (mode & GLM_COLOR) {
      glColor3fv(model->materials[group->material].diffuse);
    }

    for (i = 0; i < group->numtriangles; i++) {
      if (mode & GLM_FLAT)
        glNormal3fv(&model->facetnorms[3 * T(group->triangles[i]).findex]);

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * T(group->triangles[i]).nindices[0]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2*T(group->triangles[i]).tindices[0]]);

      if (model->usePerVertexColors) {
        glColor3ubv( &model->vertexColors[3 * T(group->triangles[i]).vindices[0]] );
      }

      glVertex3fv(&model->vertices[3 * T(group->triangles[i]).vindices[0]]);
#if 0
      printf("%f %f %f\n", 
          model->vertices[3 * T(group->triangles[i]).vindices[0] + X],
          model->vertices[3 * T(group->triangles[i]).vindices[0] + Y],
          model->vertices[3 * T(group->triangles[i]).vindices[0] + Z]);
#endif

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * T(group->triangles[i]).nindices[1]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2*T(group->triangles[i]).tindices[1]]);
      if (model->usePerVertexColors) {
        glColor3ubv( &model->vertexColors[3 * T(group->triangles[i]).vindices[1]] );
      }
      glVertex3fv(&model->vertices[3 * T(group->triangles[i]).vindices[1]]);
#if 0
      printf("%f %f %f\n", 
          model->vertices[3 * T(group->triangles[i]).vindices[1] + X],
          model->vertices[3 * T(group->triangles[i]).vindices[1] + Y],
          model->vertices[3 * T(group->triangles[i]).vindices[1] + Z]);
#endif

      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * T(group->triangles[i]).nindices[2]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2*T(group->triangles[i]).tindices[2]]);
      if (model->usePerVertexColors) {
        glColor3ubv( &model->vertexColors[3 * T(group->triangles[i]).vindices[2]] );
      }
      glVertex3fv(&model->vertices[3 * T(group->triangles[i]).vindices[2]]);
#if 0
      printf("%f %f %f\n", 
          model->vertices[3 * T(group->triangles[i]).vindices[2] + X],
          model->vertices[3 * T(group->triangles[i]).vindices[2] + Y],
          model->vertices[3 * T(group->triangles[i]).vindices[2] + Z]);
#endif

    }

    group = group->next;
  }
  glEnd();

  glPopMatrix();
}
#endif

