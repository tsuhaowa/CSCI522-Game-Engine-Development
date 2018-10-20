#include "PhysicsManager.h"
#include <assert.h>

#define PHYSICS_COMPONENT_SIZE 200

std::vector<PhysicsComponent> PhysicsManager::m_phyComponents;
std::vector<PhysicsComponent*> PhysicsManager::m_updatePhyComponents;
float PhysicsManager::m_frameTime;

void PhysicsManager::construct()
{
	m_phyComponents.reserve(PHYSICS_COMPONENT_SIZE);
	m_frameTime = 0.033f;
}

void PhysicsManager::add(SceneNode *t_sceneNode, char *t_meshFileName)
{
	// if over capacity, will cause reallocation and make all iterator invalidated
	// not implement solution yet
	assert(m_phyComponents.size() < m_phyComponents.capacity());  

	m_phyComponents.push_back(PhysicsComponent());
	m_phyComponents.back().construct(t_sceneNode, t_meshFileName);
}

void PhysicsManager::do_PHYSICS_MANAGER_UPDATE(float frameTime)
{
	m_frameTime = frameTime;

	do_PRE_PHYSICS_UPDATE();
	do_SIMULATION();
	do_POST_PHYSICS_UPDATE();
}

void PhysicsManager::do_PRE_PHYSICS_UPDATE()
{
	// import info from SceneNode to PhysicsComponent
	for (PhysicsComponent &phyC : m_phyComponents)
	{
		SceneNode *pMainSN = phyC.m_pMainSN;
		if (pMainSN->m_base == phyC.m_prevMainBase)
			continue;  // cull out unchanged object

		
		m_updatePhyComponents.push_back(&phyC);
		
		if (strcmp(phyC.m_meshFileName, "soldier") == 0)
		{

		}
		else  
		{
			// update static Mesh BoundingBoxAfterTransform if they move or transform
		}
		
		phyC.m_postMainBase = pMainSN->m_base;
	}
}

void PhysicsManager::do_POST_PHYSICS_UPDATE()
{
	// sync info from PhysicsComponent back to SceneNode
	for (PhysicsComponent *phyC : m_updatePhyComponents)
	{
		phyC->m_prevMainBase = phyC->m_postMainBase;
		phyC->m_pMainSN->m_base = phyC->m_postMainBase;
	}

	m_updatePhyComponents.clear();
}

void PhysicsManager::do_SIMULATION()
{
	// check all moving or turning objects
	for (PhysicsComponent *curPhyC : m_updatePhyComponents)
	{
		float speed = 1.4f;
		float allowedDisp = speed * m_frameTime;
		Vector3 dir;
		Vector3 curPos = curPhyC->m_prevMainBase.getPos();

		if (checkCollisionToALL(curPhyC))  
		{
			// has collision at least floor	
			curPhyC->m_fallingTime = 0.0f;

			if (curPhyC->m_collisionCount == 1 && !(curPhyC->m_prevMainBase.getPos().getY() == curPhyC->m_postMainBase.getPos().getY()) )
			{
				// no collision with objects but only the floor plane
				// force soldier not penetrate through the plane if the target pos is at different height
				// rescale the dir
				curPhyC->m_postMainBase.m16[7] = curPhyC->m_curStandPlane.getFloorHeight();  // 7 is y pos 
				dir = curPhyC->m_postMainBase.getPos() - curPhyC->m_prevMainBase.getPos();
				dir.normalize();
				curPhyC->m_postMainBase.setPos(curPhyC->m_prevMainBase.getPos() + dir * allowedDisp);
			}
			else if (curPhyC->m_collisionCount == 1 && curPhyC->m_doDodge)
			{
				// dodge after collision, only collision with floor plane now
				dir = curPhyC->m_moveDodge;
				dir.normalize();
				curPhyC->m_postMainBase.setPos(curPhyC->m_prevMainBase.getPos() + dir * allowedDisp);
				curPhyC->m_doDodge = false;
			}
			else if (curPhyC->m_collisionCount > 1)
			{
				// decide collision dir if collision with objects
				for (int i = 0; i < curPhyC->m_collisionPlane.size(); ++i)
				{
					// when walking, do not care the top or bottom plane collision
					if (curPhyC->m_collisionPlane[i].a == 0 && curPhyC->m_collisionPlane[i].c == 0) continue;

					// get a backward direction from target to current pos
					Vector3 vTargetToCus = curPhyC->m_postMainBase.getPos() -curPhyC->m_prevMainBase.getPos();

					// if the previous collision plane still exist, use it instead of a new collision plane
					bool checkCurCollisionPlaneExist = false;
					for (const Plane &testP : curPhyC->m_collisionPlane)
					{
						if (curPhyC->m_curCollisionPlane == testP)
							checkCurCollisionPlaneExist = true;
					}

					if (curPhyC->m_curCollisionPlane == Plane() || !checkCurCollisionPlaneExist)
					{
						// check which dodge dir can make soldier closer to target
						curPhyC->m_curCollisionPlane = curPhyC->m_collisionPlane[i];
						Vector3 vLeft = curPhyC->m_upVector.crossProduct(curPhyC->m_collisionPlane[i].getOutsideN());  // the direction parallel to collision plane of left way
						Vector3 vRight = curPhyC->m_collisionPlane[i].getOutsideN().crossProduct(curPhyC->m_upVector);  // the direction parallel to collision plane of right way
						curPhyC->m_moveDodge = (vTargetToCus - vLeft).lengthSqr() < (vTargetToCus - vRight).lengthSqr() ?
							vLeft : vRight;
					}
					dir = curPhyC->m_collisionPlane[i].getOutsideN() + curPhyC->m_moveDodge;
					dir.normalize();
					curPhyC->m_postMainBase.setPos(curPhyC->m_prevMainBase.getPos() + dir * allowedDisp);
					curPhyC->m_doDodge = true;
				}
			}
		}
		else if (strcmp(curPhyC->m_meshFileName, "soldier") == 0)
		{
			// no collision but moving => soldier falling
			curPhyC->m_fallingTime += 0.006f;
			float velocityOfFalling = -1 * (curPhyC->m_accelerationOfGravity * curPhyC->m_fallingTime);
			dir = Vector3(0, velocityOfFalling, 0);

			curPhyC->m_postMainBase.setPos(curPos + dir);
		}
	}
}

bool PhysicsManager::checkCollisionToALL(PhysicsComponent *curPhyC)
{
	curPhyC->m_stuckCheck = false;
	curPhyC->m_collisionCount = 0;
	curPhyC->m_collisionPhyC.clear();
	curPhyC->m_collisionPlane.clear();

	for (int i=0; i<m_phyComponents.size(); ++i)
	{
		PhysicsComponent *t_phyC = &m_phyComponents[i];
		if (curPhyC == t_phyC)  // should check collision with itself
			continue;

		bool checkSkipMesh = false;
		for (char *skipMeshName : curPhyC->m_collisionSkipList)
		{
			if (strcmp(t_phyC->m_meshFileName, skipMeshName) == 0)
			{
				checkSkipMesh = true;
				break;
			}
		}
		if (checkSkipMesh) continue;

		if (checkCollisionTwoPhysicsComponent(curPhyC, t_phyC))
		{
			++curPhyC->m_collisionCount;

			if (strcmp(t_phyC->m_meshFileName, "cobbleplane.x_pplaneshape1_mesh.mesha") == 0)
			{
				curPhyC->m_curStandPlane = t_phyC->m_boundingBoxPlanes[3];  // top plane of standing floor
				continue;
			}

			addCollisionPlane(curPhyC, t_phyC);
			curPhyC->m_collisionPhyC.push_back(t_phyC);
		}
	}

	curPhyC->m_collisionCheck = curPhyC->m_collisionCount > 0;
	return curPhyC->m_collisionCheck;
}

bool PhysicsManager::checkCollisionTwoPhysicsComponent(PhysicsComponent *curPhyC, PhysicsComponent *t_phyC)
{
	// Non-Axis Aligned Bounding Box Collision
	// Separating Axis Theorem
	// project 2 * 8 vectors to 3 * 2 normal vectors, for each combination check line segment intersecting
	// if there is one no overlapping => no collision
	Vector3 separatingAxes[6] = {
		curPhyC->m_boundingBoxPlanes[0].getN(), curPhyC->m_boundingBoxPlanes[1].getN(), curPhyC->m_boundingBoxPlanes[2].getN(),
		t_phyC->m_boundingBoxPlanes[0].getN(), t_phyC->m_boundingBoxPlanes[1].getN(), t_phyC->m_boundingBoxPlanes[2].getN() };

	for (int i = 0; i < 6; ++i)
	{
		float lminDot = FLT_MAX, lmaxDot = -FLT_MAX, rminDot = FLT_MAX, rmaxDot = -FLT_MAX;

		for (int j = 0; j < 8; ++j)
		{
			float tempDot = separatingAxes[i].dotProduct(curPhyC->m_boundingBoxVertexAfterTransform[j]);
			lminDot = min(lminDot, tempDot);
			lmaxDot = max(lmaxDot, tempDot);
		}

		for (int j = 0; j < 8; ++j)
		{
			float tempDot = separatingAxes[i].dotProduct(t_phyC->m_boundingBoxVertexAfterTransform[j]);
			rminDot = min(rminDot, tempDot);
			rmaxDot = max(rmaxDot, tempDot);
		}

		if (!checkSegmentIntersect(lminDot, lmaxDot, rminDot, rmaxDot))
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

void PhysicsManager::addCollisionPlane(PhysicsComponent *curPhyC, PhysicsComponent *t_phyC)
{
	bool checkPlane[6];

	for (int i = 0; i < 6; ++i)
	{
		checkPlane[i] = checkCollisionPlane(curPhyC, t_phyC->m_boundingBoxPlanes[i]);
	}

	for (int i = 0; i < 3; ++i)
	{
		if (checkPlane[i] ^ checkPlane[i + 3])  // check the paired plane, ex: 0 and 4 is top and bottom
		{
			checkPlane[i] ? curPhyC->m_collisionPlane.push_back(t_phyC->m_boundingBoxPlanes[i]) :
							curPhyC->m_collisionPlane.push_back(t_phyC->m_boundingBoxPlanes[i + 3]);
		}
	}
}

bool PhysicsManager::checkCollisionPlane(PhysicsComponent *curPhyC, Plane &p)
{
	for (int i = 0; i < 8; ++i) {
		Vector3 extent = curPhyC->m_boundingBoxVertexAfterTransform[i] - curPhyC->m_boundingBoxCenter;

		float d_projection = abs(p.getN().dotProduct(extent));  // distance of the rear vertex projecting on p's n vector
		float d_centerToP = abs(curPhyC->m_boundingBoxCenter.dotProduct(p.getN()) + p.getD());

		if ((d_centerToP <= d_projection) && (p != Plane())) return true;
	}
	return false;
}