/********************************************************************
	created:	2010/04/13
	file name:	scene.cpp
	author:		maxint lnychina@gmail.com
*********************************************************************/

#include "scene.h"
#include "raytracer.h"
#include "material.h"
#include "primitive.h"
#include "AccessObj.h"

#include <sstream>

namespace RayTracer {

using std::min;
using std::max;
using std::abs;

// ------------------------------------------------------------------------------
// Scene class implementation
// ------------------------------------------------------------------------------

Scene::Scene()
: mObjLoader(0)
{}

Scene::~Scene()
{
	destroy();
	destroyLights();
}

void Scene::destroy()
{
	// release primitives
	PrimListItor it = mPrimitives.begin();
	PrimListItor it_end = mPrimitives.end();
	for (; it != it_end; ++it)
	{
		SAFE_DELETE(*it);
	}
	mPrimitives.clear();
	mLights.clear();

	// remove regular grid
	removeGrid();

	// release vertex buffer
	VertexItor vit = mVerticesPool.begin();
	VertexItor vit_end = mVerticesPool.end();
	for (; vit!=vit_end; ++vit)
	{
		SAFE_DELETE(*vit);
	}
	mVerticesPool.clear();
}

void Scene::destroyLights()
{
	// release lights
	LightItor lit = mLights.begin();
	LightItor lit_end = mLights.end();
	for (; lit!=lit_end; ++lit)
	{
		SAFE_DELETE(*lit);
	}
	mLights.clear();
}

void Scene::setupMaterials()
{
	// add textures to texture manager
	TextureManager::getInstance().createFromFile("textures/marble.png", "marble");
	TextureManager::getInstance().createFromFile("textures/wood.png", "wood");
	
	// add materials
	Material *mat = MaterialManager::getInstance().createManual("woodMat");
	mat->setReflection(0);
	mat->setRefraction(0);
	mat->setDiffuse(0.5f*Vec3::ONE);
	mat->setUVScale(0.1f, 0.1f);
	mat->setAmbient(Vec3::ZERO);
	mat->setTexture("wood");

	mat = MaterialManager::getInstance().createManual("marbleMat");
	mat->setReflection(0.0f);
	mat->setRefraction(0.0f);
	mat->setRefrIndex(1.3f);
	mat->setDiffuse(Vec3::ONE);
	mat->setSpecular(0.3f, 0.3f, 0.3f);
	mat->setShininess(20.0f);
	//mat->setAmbient(Vec3::ZERO);
	mat->setTexture("marble");

	mat = MaterialManager::getInstance().createManual("marbleMat2");
	mat->setReflection(0.0f);
	mat->setRefraction(0.0f);
	mat->setSpecular(0.3f, 0.3f, 0.3f);
	mat->setShininess(20.0f);
	mat->setDiffuse(0.4f, 0.5f, 0.5f);
	mat->setUVScale(0.15f, 0.15f);
	mat->setTexture("marble");

	mat = MaterialManager::getInstance().createManual("reflectMat");
	mat->setReflection(1.0f);
	mat->setRefraction(0.0f);
	mat->setDiffuse(Vec3::ZERO);
	mat->setAmbient(Vec3::ZERO);

	mat = MaterialManager::getInstance().createManual("refraMat");
	mat->setReflection(0.0f);
	mat->setRefraction(1.0f);
	mat->setRefrIndex(1.2f);
	mat->setSpecular(0.5f * Vec3::ONE);
	mat->setShininess(30.0f);
	mat->setDiffuse(Vec3::ZERO);
	mat->setAmbient(Vec3::ZERO);

	mat = MaterialManager::getInstance().createManual("cellingMat");
	mat->setReflection(0.5f);
	mat->setDiffuse(0.4f, 0.3f, 0.3f);

	mat = MaterialManager::getInstance().createManual("whiteLightMat");
	mat->setDiffuse(1,1,1);

	mat = MaterialManager::getInstance().createManual("blueMat");
	mat->setDiffuse(0.2f, 0.2f, 3.0f);
	mat->setDiffuseRefl(0.2f);
	mat->setSpecular(0.3f * Vec3::ONE);
	mat->setShininess(20.0f);
	mat->setAmbient(Vec3::ZERO);

	mat = MaterialManager::getInstance().createManual("greenSphereMat");
	mat->setReflection(0);
	mat->setRefraction(0);
	mat->setSpecular(0.6f * Vec3::ONE);
	mat->setShininess(20.0f);
	mat->setDiffuse(0.3f, 1.0f, 0.4f);
}

void Scene::setupLights()
{
	Light *lit;

	// area lights
	//lit = new Light;
	//lit->mType = Light::LT_AREA;
	//lit->mAABB.setMin(Vec3(-3, 5, -1));
	//lit->mAABB.setMax(Vec3(-1, 5.1f, 1));
	//lit->setSpecular(0.2f*Vec3::ONE);
	//lit->setDiffuse(0.2f*Vec3::ONE);
	//lit->setAmbient(Vec3::ZERO);
	//mLights.push_back(lit);

	lit = new Light;
	lit->mType = Light::LT_AREA;
	lit->mAABB.setMin(Vec3(1, 5, -1));
	lit->mAABB.setMax(Vec3(3, 5.1f, 1));
	lit->setSpecular(0.2f*Vec3::ONE);
	lit->setDiffuse(0.2f*Vec3::ONE);
	mLights.push_back(lit);


	// point lights
	lit = new Light;
	lit->mType = Light::LT_POINT;
	lit->mPosition = Vec3(0,0,0);
	lit->setSpecular(0.3f*Vec3::ONE);
	lit->setDiffuse(0.5f*Vec3::ONE);
	mLights.push_back(lit);

	lit = new Light;
	lit->mType = Light::LT_POINT;
	lit->mPosition = Vec3(0, 1, 0);
	lit->setSpecular(0.3f*Vec3::ONE);
	lit->setDiffuse(0.5f*Vec3::ONE);
	mLights.push_back(lit);

}

void Scene::initScene()
{
	setupMaterials();
	setupLights();

	Primitive *prim;

#if 0
	// area light
	prim = new Box(AABB(Vec3(-3, 5, -1), Vec3(-1, 5.1f, 1)));
	prim->setName("area light 1");
	prim->setLight(true);
	prim->setMaterial("whiteLightMat");
	mPrimitives.push_back(prim);

	// area light
	prim = new Box(AABB(Vec3(1, 5, -1), Vec3(3, 5.1f, 1)));
	prim->setName("area light 2");
	prim->setLight(true);
	prim->setMaterial("whiteLightMat");
	mPrimitives.push_back(prim);

	// light source 1
	prim = new Sphere( Vec3(0, 0, 10), 0.2f);
	prim->setLight(true);
	prim->setMaterial("whiteLightMat");
	mPrimitives.push_back(prim);

	// light source 2
	prim = new Sphere( Vec3(-3, 5, 1), 0.1f);
	prim->setLight(true);
	prim->setMaterial("whiteLightMat");
	mPrimitives.push_back(prim);
#endif

#if 1
	// middle sphere
	prim = new Sphere(Vec3(0.0f, -2.0f, 0.0f), 1.0f);
	prim->setName("middle sphere");
	prim->setMaterial("reflectMat");
	//prim->setMaterial("refraMat");
	mPrimitives.push_back(prim);
#endif

#if 0
	Vertex *vert1 = new Vertex(Vec3(-1, 1, 1), -Vec3::UNIT_Z, 0, 0);
	Vertex *vert2 = new Vertex(Vec3(1, 1, 1), -Vec3::UNIT_Z, 0, 1);
	Vertex *vert3 = new Vertex(Vec3(-2, -2, 1), -Vec3::UNIT_Z, 1, 0);
	mVerticesPool.push_back(vert1);
	mVerticesPool.push_back(vert2);
	mVerticesPool.push_back(vert3);
	prim = new TrianglePrim(vert1, vert2, vert3);
	prim->setName("a triangle");
	prim->setMaterial("marbleMat");
	mPrimitives.push_back(prim);
#endif

#if 0
	// left sphere
	prim = new Sphere(Vec3(-5.0f, -0.8f, 0.0f), 2.0f);
	prim->setName("left sphere");
	prim->setMaterial("blueMat");
	mPrimitives.push_back(prim);

	// right sphere
	prim = new Sphere(Vec3(5.0f, -0.8f, 0.0f), 2.0f);
	prim->setName("right sphere");
	prim->setMaterial("marbleMat");
	mPrimitives.push_back(prim);

#endif

#if 0
	// bottom box
	prim = new Box(AABB(Vec3(-1.5f, -4.3f, 5.5f), Vec3(1.5f, -3.0f, 8.5f)));
	prim->setName("middle box");
	prim->getMaterial()->setDiffuse(0.5f);
	prim->getMaterial()->setSpecular(0.0f);
	prim->getMaterial()->setReflection(0.0f);
	prim->getMaterial()->setRefraction(0.8f);
	prim->getMaterial()->setRefrIndex(1.3f);
	prim->getMaterial()->setColor(0.8f, 0.8f, 0.8f);
	mPrimitives.push_back(prim);
#endif

#if 0
	// ground plane
	prim = new PlanePrim(Vec3::UNIT_Y, 4.4f);
	prim->setName("plane");
	prim->setMaterial("woodMat");
	mPrimitives.push_back(prim);

	// black plane
	prim = new PlanePrim(-Vec3::UNIT_Z, 5.0f);
	prim->setName("back plane");
	prim->setMaterial("marbleMat2");
	mPrimitives.push_back(prim);

#endif
#if 0
	// right plane
	prim = new PlanePrim(Vec3(-1,0,0), 8.0f);
	prim->setName("right wall");
	prim->setMaterial("marbleMat2");
	mPrimitives.push_back(prim);

	// ceiling plane
	prim = new PlanePrim(Vec3(0,-1,0), 5.2f);
	prim->setName("celling plane");
	prim->setMaterial("cellingMat");
	mPrimitives.push_back(prim);

	// left plane
	//prim = new PlanePrim(Vec3(1,0,0), 8.0f);
	//prim->setName("left wall");
	//prim->setMaterial("reflectMat");
	//mPrimitives.push_back(prim);
#endif

	// build the regular grid
	buildGrid();
}

void Scene::removeGrid()
{
	GridMapItor git = mGird.begin();
	GridMapItor git_end = mGird.end();
	for (; git!=git_end; ++git)
	{
		SAFE_DELETE(*git);
	}
	mGird.clear();
}

void Scene::buildGrid()
{
	// initialize regular grid
	removeGrid();
	mGird.resize(RT_GRIDSIZE * RT_GRIDSIZE * RT_GRIDSIZE, 0);

	updateExtends();
	Vec3 dv = mExtends.getDim() / RT_GRIDSIZE;
	Vec3 rdv = 1.0f / dv;
	Vec3 rMin, rMax;
	AABB cell;

	// store primitives in the grid cells
	PrimListItor it = mPrimitives.begin();
	PrimListItor it_end = mPrimitives.end();
	Primitive *prim;
	int count = 0;
	for (; it!=it_end; ++it)
	{
		prim = *it;

		// find out which cells could contain the primitive ( based on aabb)
		rMin = (prim->getAABB().getMin() - mExtends.getMin()) * rdv;
		rMax = (prim->getAABB().getMax() - mExtends.getMin()) * rdv + 1.0f;
		rMin.Max(Vec3::ZERO);
		rMax.Min(Vec3::ONE * (RT_GRIDSIZE-1));
		
		// loop over candidate cells
		for (int z=static_cast<int>(rMin.z); z<static_cast<int>(rMax.z); ++z)
			for (int y=static_cast<int>(rMin.y); y<static_cast<int>(rMax.y); ++y)
				for (int x=static_cast<int>(rMin.x); x<static_cast<int>(rMax.x); ++x)
		{
			// construct aabb for current cell
			int idx = x + y * RT_GRIDSIZE + z * RT_GRIDSIZE * RT_GRIDSIZE;
			Vec3 pos(mExtends.getMin() + Vec3(static_cast<Real>(x),
				static_cast<Real>(y), static_cast<Real>(z) ) * dv);
			cell.setMin(pos);
			cell.setMax(pos + dv);
			// do an accurate aabb / primitive intersection test
			if (prim->intersetBox(cell))
			{
				if (mGird[idx] == 0)
					mGird[idx] = new ObjectList;
				mGird[idx]->push_back(prim);
				//std::cout << count << ": " << idx << std::endl;
				++count;
			}
		} // end for cells
	}// end for primitives
	count = count;
}

void Scene::updateExtends()
{
	//Vec3 tMin(10000 * Vec3::ONE);
	//Vec3 tMax(-10000 * Vec3::ONE);
	Vec3 tMin(-3, -3, -6), tMax( 14, 8, 30 );

	if (mObjLoader)
	{
		tMin.x = mObjLoader->m_vMin.x;
		tMin.y = mObjLoader->m_vMin.y;
		tMin.z = mObjLoader->m_vMin.z;
		tMax.x = mObjLoader->m_vMax.x;
		tMax.y = mObjLoader->m_vMax.y;
		tMax.z = mObjLoader->m_vMax.z;
	}

	//PrimListItor it = mPrimitives.begin();
	//PrimListItor it_end = mPrimitives.end();
	//for (; it!=it_end; ++it)
	//{
	//	if ((*it)->getType() != Primitive::PT_PLANE)
	//	{
	//		tMin.Min((*it)->getAABB().getMin());
	//		tMax.Max((*it)->getAABB().getMax());
	//	}
	//}
	//Vec3 tCentre = 0.5 * (tMin + tMax);
	//Vec3 tOffset = tMax - tCentre;
	//tMin -= 2.0f * tOffset;
	//tMax += 2.0f * tOffset;
	mExtends.setMin(tMin);
	mExtends.setMax(tMax);
}

void Scene::loadObjModel(const trimeshVec::CAccessObj* accessObj)
{
	//destroy();
	mObjLoader = accessObj;

	unsigned int i, k, count = 0;
	trimeshVec::CPoint3D *vpVertices = accessObj->m_pModel->vpVertices;
	trimeshVec::CPoint3D *vpNormals = accessObj->m_pModel->vpNormals;
	trimeshVec::CPoint3D *vpTexCoords = accessObj->m_pModel->vpTexCoords;
	trimeshVec::COBJmaterial *pMaterials = accessObj->m_pModel->pMaterials;
	for (i=0; i<accessObj->m_pModel->nMaterials; ++i)
	{
		trimeshVec::COBJmaterial &mat = accessObj->m_pModel->pMaterials[i];
		Material *newmat = MaterialManager::getInstance().createManual(mat.name);
		Texture *tex = NULL;
		if (mat.sTexture[0] != '\0')
		{
			tex = TextureManager::getInstance().createFromFile(mat.sTexture);
		}
		newmat->setTexture(tex);
		newmat->setAmbient(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
		newmat->setDiffuse(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
		newmat->setSpecular(mat.specular[0], mat.specular[1], mat.specular[2]);
		newmat->setShininess(mat.shininess[0]);
		newmat->setEmission(mat.emissive[0], mat.emissive[1], mat.emissive[2]);
	}

	TrianglePrim *prim;
	for (i=0; i<accessObj->m_pModel->nTriangles; ++i)
	{
		trimeshVec::COBJtriangle &tri = accessObj->m_pModel->pTriangles[i];
		Vertex *v[3];
		for (k=0; k<3; ++k)
		{
			trimeshVec::CPoint3D &pos = vpVertices[tri.vindices[k]];
			if (vpNormals)
			{
				trimeshVec::CPoint3D &norm = vpNormals[tri.nindices[k]];
				if (vpTexCoords)
				{
					trimeshVec::CPoint3D &texcoord = vpTexCoords[tri.tindices[k]];
					v[k] = new Vertex(Vec3(pos.x, pos.y, pos.z), Vec3(norm.x, norm.y, norm.z),
						texcoord.x, texcoord.y);
				}
				else
				{
					v[k] = new Vertex(Vec3(pos.x, pos.y, pos.z), Vec3(norm.x, norm.y, norm.z));
				}
			}
			else
				v[k] = new Vertex(Vec3(pos.x, pos.y, pos.z));
			mVerticesPool.push_back(v[k]);
		}
		prim = new TrianglePrim(v[0], v[1], v[2], vpNormals==NULL);
		std::ostringstream oss;
		oss << "_Triangle" << ++count;
		prim->setName(oss.str());
		if (pMaterials)
		{
			trimeshVec::COBJmaterial &mat =  pMaterials[tri.mindex];
			prim->setMaterial(mat.name);
		}
		mPrimitives.push_back(prim);
	}

	buildGrid();
}

}; // namespace RayTracer