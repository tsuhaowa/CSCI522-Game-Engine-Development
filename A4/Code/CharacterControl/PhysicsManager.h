#ifndef _CHARACTER_CONTROL_PHYSICS_MANAGER_
#define _CHARACTER_CONTROL_PHYSICS_MANAGER_

// Inter-Engine includes
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Math/Plane.h"
#include "Events/Events.h"

//#define USE_DRAW_COMPONENT

namespace PE {
	
namespace Components {

struct PhysicsManager : public Component
{
	PE_DECLARE_CLASS(PhysicsManager);

	// Constructor -------------------------------------------------------------
	PhysicsManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

	PhysicsManager getInstance() { return *this; }

	// Component ------------------------------------------------------------
	virtual void addDefaultComponents();

	bool collisionDetectionAll();  // test collision detection toward all MeshInstances
	bool checkCollisionPhysicsManager(PhysicsManager *pPM);
	bool checkCollisionPlane(Plane &p);
	bool checkStuck(PhysicsManager *pPM);
	bool checkSegmentIntersect(float lminP, float lmaxP, float rminP, float rmaxP);
	void addCollisionPlane(PhysicsManager *pPM);
	void setExtremeAxisPos();
	void buildBoundingVolume(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	void buildBoundingVolumeAfterTransform(const Matrix4x4 &worldMatrix);
	void setBoundingBoxCenterAndHalfLength();
	void buildCollisionSkipList();
	static void setExtremeValue(const float &x, const float &y, const float &z, float &minX, float &maxX, 
						float &minY, float &maxY, float &minZ, float &maxZ);
	void operator=(const PhysicsManager &rhs);

	float m_minX;
	float m_maxX;
	float m_minY;
	float m_maxY;
	float m_minZ;
	float m_maxZ;
	float m_velocity;
	float m_mess;
	float m_acceleration;
	float m_accelerationOfGravity;
	float m_gravity;
	float m_fallingTime;
	bool m_collisionCheck;
	int m_collisionCount;
	bool m_backward;
	bool m_stuckCheck;
	Vector3 m_normalVector;  // normal vector
	Vector3 m_moveAfterBackWard;
	Vector3 m_moveBackWard;
	Vector3 m_moveDodge;
	Plane m_curCollisionPlane;
	Plane m_curStandPlane;
	PEStaticVector<Matrix4x4, 4> m_boundingBox;  // four vertices with vectors for building the AABB box
	PEStaticVector<Matrix4x4, 4> m_boundingBoxAfterTransform;  // four real world vertices with vectors for building the AABB box
	PEStaticVector<Vector3, 8> m_boundingBoxVertex;  // eight bounding box vertices
	PEStaticVector<Vector3, 8> m_boundingBoxVertexAfterTransform;  // eight real world bounding box vertices
	Array<Plane> m_boundingBoxPlanes; 
	Vector3 m_boundingBoxCenter;
	Vector3 m_boundingBoxHalfLength;
	Array<MeshInstance*, 1> m_collisionMeshInstance;
	Array<Plane, 1> m_collisionPlane;
	Array<PrimitiveTypes::Char*, 1> m_collisionSkipList;

}; // class PhysicsManager
}; // namespace Components
}; // namespace PE
#endif
