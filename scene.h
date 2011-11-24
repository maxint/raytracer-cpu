/********************************************************************
	created:	2010/04/13
	file name:	scene.h
	author:		maxint lnychina@gmail.com
*********************************************************************/

#ifndef _RT_SCENE_H_
#define _RT_SCENE_H_

#include "common.h"
#include <vector>
#include <list>


namespace trimeshVec{
	class CAccessObj;
}

namespace RayTracer {

class Primitive;
class Vertex;
class Light;

// ------------------------------------------------------------------------------
// Scene class definition
// ------------------------------------------------------------------------------

class Scene
{
public:
	Scene();
	~Scene();

	/**	Initialization
		Create some primitives
	 */
	void initScene();

	int getNumOfPrimitives() const
	{
		return static_cast<int>(mPrimitives.size());
	}
	int getNumOfLights() const
	{
		return static_cast<int>(mLights.size());
	}
	const AABB& getExtends() const
	{
		return mExtends;
	}

	/**	Load obj model file
	 */
	void loadObjModel(const trimeshVec::CAccessObj* accessObj);

	friend class Engine;

private:
	void setupMaterials();

	/**	Destroy the primitives and release the resources.
	 */
	void destroy();

	/**	Setup lights
	 */
	void setupLights();

	/**	Destroy lights
	 */
	void destroyLights();

	/**	Find extends of the primitives
	 */
	void updateExtends();

	/**	Build the grid of the primitives
	 */
	void buildGrid();

	void removeGrid();

private:
	typedef std::list<Primitive*>		PrimitiveList;
	typedef PrimitiveList::iterator		PrimListItor;
	typedef std::list<Primitive*>		ObjectList;
	typedef ObjectList::iterator		ObjectItor;
	typedef std::vector<ObjectList*>	GridMap;
	typedef GridMap::iterator			GridMapItor;
	typedef std::list<Vertex*>			VertexList;
	typedef VertexList::iterator		VertexItor;
	typedef std::list<Light*>			LightList;
	typedef LightList::iterator			LightItor;
private:
	PrimitiveList mPrimitives;
	LightList mLights;
	GridMap mGird;
	AABB mExtends;

	/// obj model loader
	const trimeshVec::CAccessObj* mObjLoader;
	VertexList mVerticesPool;
};

}; // namespace RayTracer

#endif // _RT_SCENE_H_