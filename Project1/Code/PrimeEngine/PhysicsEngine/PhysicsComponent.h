#ifndef __PHYENGINE_PHYSICS_COMPONENT_H__
#define __PHYENGINE_PHYSICS_COMPONENT_H__

#include "Math/Plane.h"

#include "PrimeEngine/Scene/Mesh.h"
#include "PrimeEngine/Scene/MeshInstance.h"

#include <vector>

namespace PE {
namespace Components {
	struct SceneNode;
};
};

using namespace PE::Components;

struct PhysicsComponent
{
	PhysicsComponent();
	void construct(SceneNode *t_sceneNode, char *t_meshFileName);
	void init();
	void init_Soldier();
	void init_StaticMesh();
	void buildCollisionSkipList_Soldier();
	void buildBoundingVolume(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	void buildBoundingVolumeAfterTransform(const Matrix4x4 &worldMatrix);
	static void setExtremeValue(const float &x, const float &y, const float &z, float &minX, float &maxX,
		float &minY, float &maxY, float &minZ, float &maxZ);

	Matrix4x4 m_prevMainBase;
	Matrix4x4 m_postMainBase;
	float m_velocity;
	float m_mass;
	float m_acceleration;
	float m_accelerationOfGravity;
	float m_gravity;
	float m_fallingTime;
	bool m_collisionCheck;
	int m_collisionCount;
	bool m_doDodge;
	bool m_stuckCheck;

	Vector3 m_upVector;  // up vector
	Vector3 m_moveDodge;
	Plane m_curCollisionPlane;
	Plane m_curStandPlane;
	Matrix4x4 m_boundingBox[4];  // four vertices with vectors for building the AABB box
	Matrix4x4 m_boundingBoxAfterTransform[4];  // four real world vertices with vectors for building the AABB box
	Vector3 m_boundingBoxVertex[8];  // eight bounding box vertices
	Vector3 m_boundingBoxVertexAfterTransform[8];  // eight real world bounding box vertices
	Plane m_boundingBoxPlanes[6];
	Vector3 m_boundingBoxCenter;
	std::vector<PhysicsComponent*> m_collisionPhyC;
	std::vector<Plane> m_collisionPlane;
	std::vector<char*> m_collisionSkipList;
	char *m_meshFileName;
	SceneNode *m_pMainSN;

};

#endif