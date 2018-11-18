#ifndef __PHYENGINE_PHYSICS_COMPONENT_H__
#define __PHYENGINE_PHYSICS_COMPONENT_H__

#include "Math/Plane.h"

#include "PrimeEngine/Scene/Mesh.h"
#include "PrimeEngine/Scene/MeshInstance.h"

#include <vector>

#define SOLDIER_MESH_NAME "soldier.x_soldiermesh_mesh.mesha"
#define NAZICAR_MESH_NAME "nazicar.x_carmesh_mesh.mesha"
#define COBBLEPLANE_MESH_NAME "cobbleplane.x_pplaneshape1_mesh.mesha"

namespace PE {
namespace Components {
	struct SceneNode;
	struct CameraManager;
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
	void addToCameraList();
	void buildCollisionSkipList_Soldier();
	void buildBoundingVolume(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	void buildBoundingVolumeAfterTransform(const Matrix4x4 &worldMatrix);
	static void setExtremeValue(const float &x, const float &y, const float &z, float &minX, float &maxX,
		float &minY, float &maxY, float &minZ, float &maxZ);

	Matrix4x4 m_prevMainBase;
	Matrix4x4 m_postMainBase;
	int m_id;
	int m_movingState;
	float m_velocity;  // positive for forward, negative for backward
	float m_mass;
	float m_accelerationOfGravity;
	float m_gravity;
	float m_fallingTime;
	float m_movingTime;  // debugging, for maintaing highest speed
	float m_frictionCoef;
	float m_horsepower;  // const power for forward
	float m_torque;  // const power for left and right 
	float m_forceFB;  // positive for forward, negative for backward
	float m_forceLR;  // positive for rightward, negative for leftward
	Vector3 m_dir;
	float m_speedMax;
	bool m_collisionCheck;
	int m_collisionCount;
	bool m_doDodge;
	bool m_stuckCheck;

	Vector3 m_carDir[4];  // 0: front, 1:back, 2:left, 3:right
	Vector3 m_carDirAfterTransform[4];  // 0: front, 1:back, 2:left, 3:right
	Vector3 m_upVector;  // up vector
	Vector3 m_moveDodge;
	Plane m_curCollisionPlane;
	PhysicsComponent *m_curStandPlane;
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