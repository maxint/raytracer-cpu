// AccessObj.h: interface for the CAccessObj class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MY_ACCESS_OBJ_
#define _MY_ACCESS_OBJ_

#include "Point3D.h"
#include <cstdio>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete (p); (p)=0; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] (p); (p)=0; }
#endif


#define OBJ_NONE (0)			// render with only vertices 
#define OBJ_FLAT (1 << 0)		// render with facet normals 
#define OBJ_SMOOTH (1 << 1)		// render with vertex normals 
#define OBJ_TEXTURE (1 << 2)	// render with texture coords 
#define OBJ_COLOR (1 << 3)		// render with colors 
#define OBJ_MATERIAL (1 << 4)	// render with materials 

namespace trimeshVec
{
	// --------------------------------------------------------------------
	// COBJmaterial: defines a material in a model. 
	class COBJmaterial
	{
	public:
		char name[256];		// name of material
		float diffuse[4];	// diffuse component 
		float ambient[4];	// ambient component 
		float specular[4];	// specular component 
		float emissive[4];	// emissive component 
		float shininess[1];	// specular exponent

		// Texture Data
		bool bTextured;
		char sTexture[256];	// name of texture file

		char* MaterialName() { return (name);}

		float* DiffuseComponent() {return (diffuse);}
		float* AmbientComponent() {return (ambient);}
		float* SpecularComponent() {return (specular);}
		float* EmissiveComponent() {return (emissive);}
		float* ShininessComponent() {return (shininess);}

		COBJmaterial()
		{
			sprintf_s (name, "default");
			sTexture[0] = '\0';
			diffuse[0] = diffuse[1] = diffuse[2] = diffuse[3] = 1.0f;
			ambient[0] = ambient[1] = ambient[2] = ambient[3] = 0.1f;
			specular[0] = specular[1] = specular[2] = specular[3] = 0.0f;
			emissive[0] = emissive[1] = emissive[2] = emissive[3] = 0.0f;
			shininess[0] = 10;

			bTextured = false;
		}
	};

	// --------------------------------------------------------------------
	// COBJtriangle: defines a triangle in a model.
	class COBJtriangle
	{
	public:
		unsigned int vindices[3];	// array of triangle vertex indices 
		unsigned int nindices[3];	// array of triangle normal indices 
		unsigned int tindices[3];	// array of triangle texcoord indices
		unsigned int findex;		// idxInTri of triangle facet normal 
		unsigned int mindex;		// idxInTri of material
		COBJtriangle()
		{
			vindices[0] = vindices[1] = vindices[2] = 
				nindices[0] = nindices[1] = nindices[2] = 
				tindices[0] = tindices[1] = tindices[2] = 0;
		}
		~COBJtriangle()	{}
	};// ------------------------------------------------------------------

	// --------------------------------------------------------------------
	// COBJgroup: defines a group in a model.
	class COBJgroup
	{
	public:
		char name[256];			// name of this group 
		unsigned int nTriangles;	// number of triangles in this group 
		unsigned int* pTriangles;	// array of triangle indices 
		unsigned int material;		// idxInTri to material for group 
		char texturename[256];
		bool m_bTexture;
		int m_iTextureType;
		float m_fTran_X;
		float m_fTran_Y;
		float m_fTran_Z;
		float m_fScale_X;
		float m_fScale_Y;
		float m_fScale_Z;
		float m_fRotate_X;
		float m_fRotate_Y;
		float m_fRotate_Z;
		class COBJgroup* next;		// pointer to next group in model 

		COBJgroup()
		{
			nTriangles = 0;
			pTriangles = NULL;
			next = NULL;
		}

		~COBJgroup()
		{
			SAFE_DELETE_ARRAY(pTriangles)
			nTriangles = 0;
		}
	};// ------------------------------------------------------------------

	// --------------------------------------------------------------------
	// COBJmodel: defines a model.
	class COBJmodel
	{
	public:
		char pathname[256];			// path to this model 
		char mtllibname[256];		// name of the material library 
		unsigned int nVertices;		// number of vertices in model 
		CPoint3D* vpVertices;		// array of vertices 
		unsigned int nNormals;		// number of normals in model 
		CPoint3D* vpNormals;		// array of normals 
		unsigned int nTexCoords;	// number of texcoords in model 
		CPoint3D* vpTexCoords;		// array of texture coordinates 
		unsigned int nFacetnorms;	// number of facet normals in model 
		CPoint3D* vpFacetNorms;		// array of facet normals 
		unsigned int nTriangles;	// number of triangles in model 
		COBJtriangle* pTriangles;	// array of triangles 
		unsigned int nMaterials;	// number of materials in model 
		COBJmaterial* pMaterials;	// array of materials 
		unsigned int nGroups;		// number of groups in model 
		COBJgroup* pGroups;			// linked list of groups 
		CPoint3D position;			// position of the model 

		// construction
		COBJmodel()
		{
			nVertices   = 0;
			vpVertices  = NULL;
			nNormals    = 0;
			vpNormals   = NULL;
			nTexCoords  = 0;
			vpTexCoords = NULL;
			nFacetnorms = 0;
			vpFacetNorms= NULL;
			nTriangles  = 0;
			pTriangles  = NULL;
			nMaterials  = 0;
			pMaterials  = NULL;
			nGroups     = 0;
			pGroups     = NULL;
			position    = CPoint3D(0, 0, 0);
		}

		// free all memory
		void Destory()
		{
			COBJgroup *group;

			SAFE_DELETE_ARRAY(vpVertices)
			SAFE_DELETE_ARRAY(vpNormals)
			SAFE_DELETE_ARRAY(vpTexCoords)
			SAFE_DELETE_ARRAY(vpFacetNorms)
			SAFE_DELETE_ARRAY(pTriangles)
			SAFE_DELETE_ARRAY(pMaterials)

			while(pGroups)
			{
				group = pGroups;
				pGroups = pGroups->next;
				SAFE_DELETE(group);
			}

			nVertices    = 0;
			nNormals     = 0;
			nTexCoords   = 0;
			nFacetnorms  = 0;
			nTriangles   = 0;
			nMaterials   = 0;
			nGroups      = 0;
			position     = CPoint3D(0, 0, 0);
		}

		// destruction
		~COBJmodel()
		{
			Destory();
		}
	};// ------------------------------------------------------------------


	///////////////////////////////////////////////////////////////////////////////
	// Definition of the OBJ R/W class 
	///////////////////////////////////////////////////////////////////////////////
	class CAccessObj
	{
	public:
		/// A temporal class
		class OBJnode
		{
		public:
			unsigned int triIdx;
			unsigned int idxInTri; // idxInTri in triangle
			OBJnode() :triIdx(0), idxInTri(0)	{}
		};

	public:
		CAccessObj();
		~CAccessObj();

		COBJmodel *m_pModel;
		CPoint3D m_vMax, m_vMin;

	protected:

		void CalcBoundingBox();
		bool Equal(CPoint3D * u, CPoint3D * v, float epsilon);

		COBJgroup* FindGroup(char* name);
		COBJgroup* AddGroup(char* name);

		char* DirName(char* path);
		unsigned int FindMaterial(char* name);
		void ReadMTL(char* name);
		void WriteMTL(char* modelpath, char* mtllibname);

		bool FirstPass(FILE* file);
		void SecondPass(FILE* file);

		void Dimensions(float* dimensions);
		void Scale(float scale);
		void ReverseWinding();
		void FacetNormals();
		void VertexNormals(float angle);

		void LinearTexture();
		void SpheremapTexture();
		void WriteOBJ(char* filename, unsigned int mode);

	public:
		void Destory();
		void Boundingbox(CPoint3D &vMax, CPoint3D &vMin);
		bool LoadOBJ(const char* filename);
		void UnifiedModel();
	};

} //namespace trimeshVec

#endif