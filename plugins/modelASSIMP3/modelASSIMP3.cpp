////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2001-2014 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "modelASSIMP3.h"
#include "plugins/PluginFactory.h"
#include "Gem/RTE.h"
#include "Gem/Exception.h"
#include "Gem/Properties.h"
#include "Gem/VertexBuffer.h"

#include "Utils/Functions.h"

#include <string>
#include <math.h>

using namespace gem::plugins;

REGISTER_MODELLOADERFACTORY("ASSIMP2", modelASSIMP3);

namespace {

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

// ----------------------------------------------------------------------------
static void get_bounding_box_for_node (const struct aiScene*scene, const struct aiNode* nd,
	aiVector3D* min,
	aiVector3D* max,
	aiMatrix4x4* trafo
){
	aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(scene, nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

// ----------------------------------------------------------------------------

static void get_bounding_box (const aiScene*scene, aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene, scene->mRootNode,min,max,&trafo);
}

// ----------------------------------------------------------------------------

static void color4_to_float4(const aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

// ----------------------------------------------------------------------------

static void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

// ----------------------------------------------------------------------------
static void apply_material(const struct aiMaterial *mtl)
{
	float c[4];

	GLenum fill_mode;
	int ret1, ret2;
	aiColor4D diffuse;
	aiColor4D specular;
	aiColor4D ambient;
	aiColor4D emission;
	float shininess, strength;
	int two_sided;
	int wireframe;
	unsigned int max;

	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);
  //post("diffuse: %g\t%g\t%g\t%g", c[0], c[1], c[2], c[3]);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
  //post("specular: %g\t%g\t%g\t%g", c[0], c[1], c[2], c[3]);

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);
  //post("ambient: %g\t%g\t%g\t%g", c[0], c[1], c[2], c[3]);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);
  //  post("emission: %g\t%g\t%g\t%g", c[0], c[1], c[2], c[3]);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	max = 1;
	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
	if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS)) {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
    //post("shininess: %gx%g=%g\t%g", shininess, strength, shininess*strength);
  }
	else {
    /* JMZ: in modelOBJ the default shininess is 65 */
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max)) {
		fill_mode = wireframe ? GL_LINE : GL_FILL;
  }	else {
		fill_mode = GL_FILL;
  }
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
  }
}

// ----------------------------------------------------------------------------

// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
static void Color4f(const aiColor4D *color)
{
	glColor4f(color->r, color->g, color->b, color->a);
}

// ----------------------------------------------------------------------------
static void recursive_render (const struct aiScene*scene, const struct aiScene *sc, const struct aiNode* nd, const bool use_material,
      std::vector<std::vector<float> >& vertices,  std::vector<std::vector<float> >& normals, std::vector<std::vector<float> >& texcoords, std::vector<std::vector<float> >& colors)
{
	int i;
	unsigned int n = 0, t;
	aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiTransposeMatrix4(&m);
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
    if(use_material)
      apply_material(sc->mMaterials[mesh->mMaterialIndex]);

#if 0 /* handled globally */
		if(mesh->mNormals == NULL) {
      glDisable(GL_LIGHTING);
		} else {
			glEnable(GL_LIGHTING);
		}
#endif

#if 0
		if(mesh->mColors[0] != NULL) {
			glEnable(GL_COLOR_MATERIAL);
		} else {
			glDisable(GL_COLOR_MATERIAL);
		}
#endif
		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch(face->mNumIndices) {
				case 1: face_mode = GL_POINTS; break;
				case 2: face_mode = GL_LINES; break;
				case 3: face_mode = GL_TRIANGLES; break;
				default: face_mode = GL_POLYGON; break;
			}

			//~glBegin(face_mode);

      float* pt;
      std::vector<float> vec;

			for(i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];

				if(use_material && mesh->mColors[0] != NULL){
					//~Color4f(&mesh->mColors[0][index]);
          pt = (float*) &mesh->mColors[0][index];
          vec = std::vector<float>(pt,pt+4);
          colors.push_back(vec);
        }

				if(mesh->mNormals != NULL){
					//~glNormal3fv(&mesh->mNormals[index].x);
          pt = &mesh->mNormals[index].x;
          vec = std::vector<float>(pt,pt+3);
          normals.push_back(vec);
        }

        if(mesh->HasTextureCoords(0)){
          //~glTexCoord2f(mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y);
          vec.clear();
          vec.push_back(mesh->mTextureCoords[0][index].x);
          vec.push_back(mesh->mTextureCoords[0][index].y);
          texcoords.push_back(vec);
        }

				//~glVertex3fv(&mesh->mVertices[index].x);
        pt = &mesh->mVertices[index].x;
        vec = std::vector<float>(pt,pt+3);
        vertices.push_back(vec);
			}

			//~glEnd();
		}
	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		recursive_render(scene, sc, nd->mChildren[n], use_material, vertices, normals, texcoords, colors);
	}

	glPopMatrix();
}

};

modelASSIMP3 :: modelASSIMP3(void) :
  m_rebuild(true),
  m_scene(NULL),
  m_scale(1.f),
  m_useMaterial(false)
{
}

modelASSIMP3 ::~modelASSIMP3(void) {
  destroy();
}

std::vector<std::vector<float> > modelASSIMP3 :: getVector(std::string vectorName){
  if ( vectorName == "vertices" ) return m_vertices;
  if ( vectorName == "normals" ) return m_normals;
  if ( vectorName == "texcoords" ) return m_texcoords;
  if ( vectorName == "colors" ) return m_colors;
  error("there is no \"%s\" vector !",vectorName.c_str());
  return std::vector<std::vector<float> >();
}

std::vector<modelloader::VBOarray> modelASSIMP3 :: getVBOarray(){
  return m_VBOarray;
}

void modelASSIMP3 :: unsetRefresh(){ m_refresh = false; }
bool modelASSIMP3 :: needRefresh(){ return m_refresh; }

bool modelASSIMP3 :: open(const std::string&name, const gem::Properties&requestprops) {
  destroy();

	m_scene = aiImportFile(name.c_str(), aiProcessPreset_TargetRealtime_Quality);
  if(!m_scene)return false;

  get_bounding_box(m_scene, &m_min,&m_max);
  m_center.x=(m_min.x+m_max.x)/2.f;
  m_center.y=(m_min.y+m_max.y)/2.f;
  m_center.z=(m_min.z+m_max.z)/2.f;

  /* default is to rescale the object */
  float tmp;
  tmp = m_max.x-m_min.x;
  tmp = aisgl_max(m_max.y - m_min.y,tmp);
  tmp = aisgl_max(m_max.z - m_min.z,tmp);
  m_scale = 2.f / tmp;

  m_offset.x=-m_center.x;
  m_offset.y=-m_center.y;
  m_offset.z=-m_center.z;

  gem::Properties props=requestprops;
  setProperties(props);

  m_rebuild=true;
  compile();
  return true;
}

bool modelASSIMP3 :: render(void) {
  bool res=true;
  if(m_rebuild){
    res = compile();
  }

    //~glPushMatrix();
//~
    //~// scale the model
    //~glScalef(m_scale, m_scale, m_scale);
    //~// center the model
    //~glTranslatef( m_offset.x, m_offset.y, m_offset.z );
//~
    //~glPopMatrix();

  return res;
}
void modelASSIMP3 :: close(void)  {
  destroy();
}

bool modelASSIMP3 :: enumProperties(gem::Properties&readable,
                                gem::Properties&writeable) {
  readable.clear();
  writeable.clear();
  return false;
}

void modelASSIMP3 :: setProperties(gem::Properties&props) {
  double d;


#if 0
  std::vector<std::string>keys=props.keys();
  unsigned int i;
  for(i=0; i<keys.size(); i++) {
    post("key[%d]=%s ... %d", i, keys[i].c_str(), props.type(keys[i]));
  }
#endif

  if(props.get("rescale", d)) {
    bool b=(bool)d;
    if(b) {
      float tmp;
      tmp = m_max.x-m_min.x;
      tmp = aisgl_max(m_max.y - m_min.y,tmp);
      tmp = aisgl_max(m_max.z - m_min.z,tmp);
      m_scale = 2.f / tmp;

      m_offset.x=-m_center.x;
      m_offset.y=-m_center.y;
      m_offset.z=-m_center.z;
    } else {
      m_scale=1.;
      m_offset.x=m_offset.y=m_offset.z=0.f;
    }
  }
  if(props.get("usematerials", d)) {
    bool useMaterial=d;
    if(useMaterial!=m_useMaterial)
      m_rebuild=true;
    m_useMaterial=useMaterial;
  }

}
void modelASSIMP3 :: getProperties(gem::Properties&props) {
}

void modelASSIMP3 :: fillVBOarray(){
  m_VBOarray.clear();
  VBOarray vboarray;

  vboarray.data = &m_vertices;
  vboarray.type = VertexBuffer::GEM_VBO_VERTICES;
  m_VBOarray.push_back(vboarray);

  vboarray.data = &m_normals;
  vboarray.type = VertexBuffer::GEM_VBO_NORMALS;
  m_VBOarray.push_back(vboarray);

  vboarray.data = &m_texcoords;
  vboarray.type = VertexBuffer::GEM_VBO_TEXCOORDS;
  m_VBOarray.push_back(vboarray);

  vboarray.data = &m_colors;
  vboarray.type = VertexBuffer::GEM_VBO_COLORS;
  m_VBOarray.push_back(vboarray);
}

bool modelASSIMP3 :: compile(void)  {
  if(!m_scene) return false;

  GLboolean useColorMaterial=GL_FALSE;
  glGetBooleanv(GL_COLOR_MATERIAL, &useColorMaterial);

  glDisable(GL_COLOR_MATERIAL);

  // now begin at the root node of the imported data and traverse
  // the scenegraph by multiplying subsequent local transforms
  // together on GL's matrix stack.
  m_vertices.clear();
  m_normals.clear();
  m_texcoords.clear();
  m_colors.clear();
  recursive_render(m_scene, m_scene, m_scene->mRootNode, m_useMaterial, m_vertices, m_normals, m_texcoords, m_colors);
  fillVBOarray();
  if(useColorMaterial)
    glEnable(GL_COLOR_MATERIAL);

  bool res = !(m_vertices.empty() && m_normals.empty() && m_texcoords.empty() && m_colors.empty());
  if(res) {
    m_rebuild=false;
    m_refresh=true;
  }
  return res;
}
void modelASSIMP3 :: destroy(void)  {
  if(m_scene)
    aiReleaseImport(m_scene);
  m_scene=NULL;
}
