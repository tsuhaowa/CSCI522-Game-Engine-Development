#include "PhysicsManager.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Scene/MeshInstance.h"

namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(PhysicsManager, Component);

// Constructor -------------------------------------------------------------
PhysicsManager::PhysicsManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) 
	: Component(context, arena, hMyself), 
	m_boundingBoxPlanes(context, arena, 6, Plane()), m_collisionMeshInstance(context, arena, 1), 
	m_collisionSkipList(context, arena, 1), m_collisionPlane(context, arena, 1), 
	m_velocity(0), m_acceleration(0), m_gravity(0), m_mess(0), m_fallingTime(0), m_accelerationOfGravity(0.8), 
	m_collisionCheck(0), m_collisionCount(0), m_backward(0), m_stuckCheck(0)
{
	buildCollisionSkipList();
	Matrix4x4 m;
	Vector3 v;

	for (int i = 0; i < 4; ++i)
	{
		m_boundingBox.add(m);
		m_boundingBoxAfterTransform.add(m);
	}
	for (int i = 0; i < 8; ++i)
	{
		m_boundingBoxVertex.add(v);
		m_boundingBoxVertexAfterTransform.add(v);
	}

}

bool PhysicsManager::collisionDetectionAll()
{
	m_stuckCheck = false;
	m_collisionCount = 0;
	m_collisionMeshInstance.clear();
	m_collisionPlane.clear();

	RootSceneNode *rSN = RootSceneNode::Instance();

	for (int i = rSN->getFirstComponentIndex<Mesh>(); i != -1; i = rSN->getFirstComponentIndex<Mesh>(i + 1))
	{
		Mesh *pMesh = rSN->getComponentByIndex<Mesh>(i);
		if ( pMesh->isEnabled() )
		{
			bool checkSkipMesh = false;

			for (int i = 0; i < m_collisionSkipList.m_size; ++i) 
			{
				if(StringOps::startsswith(pMesh->m_meshFileName, m_collisionSkipList[i]))
				{
					checkSkipMesh = true;
					break;
				}
			}
			if (checkSkipMesh) continue;

			for (PrimitiveTypes::Int32 j = 0; j < pMesh->m_instances.m_size; ++j)
			{
				MeshInstance *pInst = pMesh->m_instances[j].getObject<MeshInstance>();
				PhysicsManager *pPM = pInst->getFirstComponent<PhysicsManager>();
				
				// do collision test here toward each MeshInstance

				if ( pPM && checkCollisionPhysicsManager(pPM))
				{
					++m_collisionCount;

					if (checkStuck(pPM))
						m_stuckCheck = true;
					if (StringOps::startsswith(pMesh->m_meshFileName, "cobbleplane.x_pplaneshape1_mesh"))
					{
						m_curStandPlane = pPM->m_boundingBoxPlanes[0];
						continue;
					}
					addCollisionPlane(pPM);
					m_collisionMeshInstance.add(pInst);
				}
			}
		}
	}
	
	m_collisionCheck = m_collisionCount > 0;
	if (m_collisionCount <= 1)
		m_stuckCheck = false;
	return m_collisionCheck;
}

bool PhysicsManager::checkCollisionPhysicsManager(PhysicsManager *pPM)
{
	// Non-Axis Aligned Bounding Box Collision
	// Separating Axis Theorem
	// project 2 * 8 vectors to 3 * 2 normal vectors, for each combination check line segment intersecting
	// if there is one no overlapping => no collision
	Vector3 separatingAxes[6] = { 
		m_boundingBoxPlanes[0].getN(), m_boundingBoxPlanes[1].getN(), m_boundingBoxPlanes[2].getN(),
		pPM->m_boundingBoxPlanes[0].getN(), pPM->m_boundingBoxPlanes[1].getN(), pPM->m_boundingBoxPlanes[2].getN() };
	
	for (int i = 0; i < 6; ++i)
	{
		float lminDot = FLT_MAX, lmaxDot = -FLT_MAX, rminDot = FLT_MAX, rmaxDot = -FLT_MAX;
		
		for (int j = 0; j < 8; ++j)
		{
			float tempDot = separatingAxes[i].dotProduct(m_boundingBoxVertexAfterTransform[j]);
			lminDot = min(lminDot, tempDot);
			lmaxDot = max(lmaxDot, tempDot);
		}

		for (int j = 0; j < 8; ++j)
		{
			float tempDot = separatingAxes[i].dotProduct(pPM->m_boundingBoxVertexAfterTransform[j]);
			rminDot = min(rminDot, tempDot);
			rmaxDot = max(rmaxDot, tempDot);
		}

		if ( !checkSegmentIntersect(lminDot, lmaxDot, rminDot, rmaxDot) ) 
		{
			return false;
		}
	}
	return true;
}

bool PhysicsManager::checkSegmentIntersect(float lminP, float lmaxP, float rminP, float rmaxP)
{
	return (lmaxP < rminP || rmaxP < lminP) ? false : true;
}

void PhysicsManager::addCollisionPlane(PhysicsManager *pPM)
{
	bool checkPlane[6];

	for (int i = 0; i < 6; ++i)
	{
		checkPlane[i] = checkCollisionPlane(pPM->m_boundingBoxPlanes[i]);
	}

	for (int i = 0; i < 3; ++i)
	{
		if (checkPlane[i] ^ checkPlane[i + 3])  // check the paired plane, ex: 0 and 4 is top and bottom
		{
			checkPlane[i] ? m_collisionPlane.add(pPM->m_boundingBoxPlanes[i]) :
							m_collisionPlane.add(pPM->m_boundingBoxPlanes[i + 3]);
		}
	}
}

bool PhysicsManager::checkCollisionPlane(Plane &p)
{
	for (int i = 0; i < 8; ++i) {
		Vector3 extent = m_boundingBoxVertexAfterTransform[i] - m_boundingBoxCenter;

		float d_projection = abs(p.getN().dotProduct(extent));  // distance of the rear vertex projecting on p's n vector
		float d_centerToP = abs(m_boundingBoxCenter.dotProduct(p.getN()) + p.getD());

		//return (d_centerToP <= d_projection) && !(p == Plane());
		if ((d_centerToP <= d_projection) && (p != Plane())) return true;
	}
	return false;
}

bool PhysicsManager::checkStuck(PhysicsManager *pPM)
{
	int insideCounter = 0;
	for (int i=0; i<pPM->m_boundingBoxPlanes.m_size; ++i)
	{
		Plane plane = pPM->m_boundingBoxPlanes[i];
		if (plane.isInsidePlane(m_boundingBoxCenter))
		{
			++insideCounter;
		}
		else break;
	}

	return insideCounter == 6;
}

void PhysicsManager::buildBoundingVolume(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
	m_boundingBox[0].setPos(Vector3(minX, minY, minZ));
	m_boundingBox[0].setU(Vector3(maxX - minX, 0, 0));
	m_boundingBox[0].setV(Vector3(0, maxY - minY, 0));
	m_boundingBox[0].setN(Vector3(0, 0, maxZ - minZ));

	m_boundingBox[1].setPos(Vector3(maxX, minY, maxZ));
	m_boundingBox[1].setU(Vector3(minX - maxX, 0, 0));
	m_boundingBox[1].setV(Vector3(0, maxY - minY, 0));
	m_boundingBox[1].setN(Vector3(0, 0, minZ - maxZ));

	m_boundingBox[2].setPos(Vector3(maxX, maxY, minZ));
	m_boundingBox[2].setU(Vector3(minX - maxX, 0, 0));
	m_boundingBox[2].setV(Vector3(0, minY - maxY, 0));
	m_boundingBox[2].setN(Vector3(0, 0, maxZ - minZ));

	m_boundingBox[3].setPos(Vector3(minX, maxY, maxZ));
	m_boundingBox[3].setU(Vector3(maxX - minX, 0, 0));
	m_boundingBox[3].setV(Vector3(0, minY - maxY, 0));
	m_boundingBox[3].setN(Vector3(0, 0, minZ - maxZ));

	// build AABB box by 8 points
	// lower vertices
	m_boundingBoxVertex[0] = Vector3(minX, minY, minZ);
	m_boundingBoxVertex[1] = Vector3(minX, minY, maxZ);
	m_boundingBoxVertex[2] = Vector3(maxX, minY, minZ);
	m_boundingBoxVertex[3] = Vector3(maxX, minY, maxZ);

	// higher vertices
	m_boundingBoxVertex[4] = Vector3(minX, maxY, minZ);
	m_boundingBoxVertex[5] = Vector3(minX, maxY, maxZ);
	m_boundingBoxVertex[6] = Vector3(maxX, maxY, minZ);
	m_boundingBoxVertex[7] = Vector3(maxX, maxY, maxZ);
}

void PhysicsManager::buildBoundingVolumeAfterTransform(const Matrix4x4 &worldMatrix)
{
	for (int i = 0; i < 4; ++i)
		m_boundingBoxAfterTransform[i] = worldMatrix * m_boundingBox[i];
	for (int i = 0; i < 8; ++i)
		m_boundingBoxVertexAfterTransform[i] = worldMatrix * m_boundingBoxVertex[i];
	
	m_normalVector = m_boundingBoxVertexAfterTransform[4] - m_boundingBoxVertexAfterTransform[0];

	// all normal vectors point to the inside of the box
	m_boundingBoxPlanes[0].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[0], m_boundingBoxVertexAfterTransform[1], m_boundingBoxVertexAfterTransform[2]);  // bottom
	m_boundingBoxPlanes[1].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[4], m_boundingBoxVertexAfterTransform[0], m_boundingBoxVertexAfterTransform[6]);  // match to [4]
	m_boundingBoxPlanes[2].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[5], m_boundingBoxVertexAfterTransform[1], m_boundingBoxVertexAfterTransform[4]);
	m_boundingBoxPlanes[3].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[4], m_boundingBoxVertexAfterTransform[6], m_boundingBoxVertexAfterTransform[5]);  // top
	m_boundingBoxPlanes[4].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[7], m_boundingBoxVertexAfterTransform[3], m_boundingBoxVertexAfterTransform[5]);
	m_boundingBoxPlanes[5].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[6], m_boundingBoxVertexAfterTransform[2], m_boundingBoxVertexAfterTransform[7]);

	setBoundingBoxCenterAndHalfLength();
	setExtremeAxisPos();
}

void PhysicsManager::setBoundingBoxCenterAndHalfLength()
{
	m_boundingBoxCenter = (m_boundingBoxVertexAfterTransform[0] + m_boundingBoxVertexAfterTransform[7]) / 2.0f;

	float lengthX = abs(m_boundingBoxVertexAfterTransform[2].getX() - m_boundingBoxVertexAfterTransform[0].getX());
	float lengthY = abs(m_boundingBoxVertexAfterTransform[4].getY() - m_boundingBoxVertexAfterTransform[0].getY());
	float lengthZ = abs(m_boundingBoxVertexAfterTransform[1].getZ() - m_boundingBoxVertexAfterTransform[0].getZ());
	m_boundingBoxHalfLength = Vector3(lengthX*0.5f, lengthY*0.5f, lengthZ*0.5f);
}

void PhysicsManager::setExtremeAxisPos()
{
	float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX, minZ = FLT_MAX, maxZ = -FLT_MAX;

	for (int i = 0; i < 8; ++i)
	{
		Vector3 v = m_boundingBoxVertexAfterTransform[i];
		setExtremeValue(v.getX(), v.getY(), v.getZ(), minX, maxX, minY, maxY, minZ, maxZ);
	}

	m_minX = minX;
	m_maxX = maxX;
	m_minY = minY;
	m_maxY = maxY;
	m_minZ = minZ;
	m_maxZ = maxZ;
}

void PhysicsManager::setExtremeValue(const float &x, const float &y, const float &z, float &minX, float &maxX,
									float &minY, float &maxY, float &minZ, float &maxZ)
{
	if (x < minX) minX = x;
	if (x > maxX) maxX = x;
	if (y < minY) minY = y;
	if (y > maxY) maxY = y;
	if (z < minZ) minZ = z;
	if (z > maxZ) maxZ = z;
}

void PhysicsManager::operator=(const PhysicsManager &rhs)
{
	this->m_velocity = rhs.m_velocity;
	this->m_gravity = rhs.m_gravity;
	this->m_boundingBox = rhs.m_boundingBox;
	this->m_boundingBoxAfterTransform = rhs.m_boundingBoxAfterTransform; 
	this->m_boundingBoxVertex = rhs.m_boundingBoxVertex;
	this->m_boundingBoxVertexAfterTransform = rhs.m_boundingBoxVertexAfterTransform;
}

void PhysicsManager::buildCollisionSkipList()
{
	PrimitiveTypes::Char *skipBuffer = NULL;

	skipBuffer = "SoldierTransform";
	m_collisionSkipList.add(skipBuffer);

	skipBuffer = "m98.x_m98main_mesh";
	m_collisionSkipList.add(skipBuffer);
}

void PhysicsManager::addDefaultComponents()
{
	Component::addDefaultComponents();

	// custom methods of this component
}

};  // namespace Components
};  // namespace PE