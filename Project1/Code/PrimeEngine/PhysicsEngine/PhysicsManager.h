#ifndef __PHYENGINE_PHYSICS_MANAGER_H__
#define __PHYENGINE_PHYSICS_MANAGER_H__

#include "PhysicsComponent.h"

#include "PrimeEngine/Scene/SceneNode.h"

#include <vector>

using namespace PE::Components;

struct PhysicsManager
{
	static std::vector<PhysicsComponent> m_phyComponents;
	static std::vector<PhysicsComponent*> m_updatePhyComponents;
	static float m_frameTime;

	static void do_PHYSICS_MANAGER_UPDATE(float frameTime);
	static void do_PRE_PHYSICS_UPDATE();
	static void do_SIMULATION();
	static void do_POST_PHYSICS_UPDATE();

	static void construct();
	static void add(SceneNode *t_sceneNode, char *t_meshFileName);
	static int getSize() { return m_phyComponents.size(); }
	static PhysicsComponent* getLastPhysicsComponentAddress() { return &(m_phyComponents.back()); }
	static bool checkCollisionToALL(PhysicsComponent *curPhyC);
	static bool checkCollisionTwoPhysicsComponent(PhysicsComponent *curPhyC, PhysicsComponent *t_phyC);
	static bool checkSegmentIntersect(float lminP, float lmaxP, float rminP, float rmaxP);
	static void addCollisionPlane(PhysicsComponent *curPhyC, PhysicsComponent *t_phyC);
	static bool checkCollisionPlane(PhysicsComponent *curPhyC, Plane &p);
};

#endif