#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "SoldierNPCMovementSM.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPC.h"

#include <PrimeEngine/Scene/SkeletonInstance.h>
#include "PrimeEngine/Scene/MeshInstance.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a soldier should move
namespace Events{

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_MOVE_TO, Event);

SoldierNPCMovementSM_Event_MOVE_TO::SoldierNPCMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos)
{ }

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_STOP, Event);

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_TARGET_REACHED, Event);
}

namespace Components{

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM, Component);


SoldierNPCMovementSM::SoldierNPCMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) 
: Component(context, arena, hMyself)
, m_state(STANDING)
{}

SceneNode *SoldierNPCMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
		
	}
	return NULL;
}

void SoldierNPCMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_MOVE_TO, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_STOP, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP);
	
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierNPCMovementSM::do_UPDATE);
}

void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	SoldierNPCMovementSM_Event_MOVE_TO *pRealEvt = (SoldierNPCMovementSM_Event_MOVE_TO *)(pEvt);
	
	// change state of this state machine
	m_state = WALKING_TO_TARGET;
	m_targetPostion = pRealEvt->m_targetPosition;

	// make sure the animations are playing
	
	PE::Handle h("SoldierNPCAnimSM_Event_WALK", sizeof(SoldierNPCAnimSM_Event_WALK));
	Events::SoldierNPCAnimSM_Event_WALK *pOutEvt = new(h) SoldierNPCAnimSM_Event_WALK();
	
	SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

	// release memory now that event is processed
	h.release();
}

void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	Events::SoldierNPCAnimSM_Event_STOP Evt;

	SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
}

void SoldierNPCMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_state == WALKING_TO_TARGET)
	{
		// see if parent has scene node component
		SceneNode *pSN = getParentsSceneNode();
		if (pSN)
		{
			SceneNode *ptempSN = pSN->getFirstComponent<SceneNode>();
			SceneNode *pRotateSN = ptempSN->getFirstComponent<SceneNode>();
			SkeletonInstance *pSI = pRotateSN->getFirstComponent<SkeletonInstance>();
			PhysicsManager *pPM = pSI->getFirstComponent<PhysicsManager>();

			Vector3 dir;
			Vector3 curPos = pSN->m_base.getPos();

			if (!pPM->m_backward && pPM->m_collisionCount > 1)
			{
				// usually have a collision with floor plane
				// collision with object, get the collision plane and change the direction
				for (int i = 0; i < pPM->m_collisionPlane.m_size; ++i)
				{
					// when walking, do not care the top or bottom plane collision
					if (pPM->m_collisionPlane[i].a == 0 && pPM->m_collisionPlane[i].c == 0) continue;

					Vector3 vTargetToCus = m_targetPostion - pSN->m_base.getPos();
					int checkCurCollisionPlaneExist = pPM->m_collisionPlane.indexOf(pPM->m_curCollisionPlane);

					if (pPM->m_curCollisionPlane == Plane() || checkCurCollisionPlaneExist == PrimitiveTypes::Constants::c_MaxUInt32)
					{
						// check which can make soldier closer to target
						pPM->m_curCollisionPlane = pPM->m_collisionPlane[i];
						Vector3 vLeft = pPM->m_normalVector.crossProduct(pPM->m_collisionPlane[i].getOutsideN());  // the direction parallel to collision plane of left way
						Vector3 vRight = pPM->m_collisionPlane[i].getOutsideN().crossProduct(pPM->m_normalVector);  // the direction parallel to collision plane of right way
						pPM->m_moveDodge = (vTargetToCus - vLeft).lengthSqr() < (vTargetToCus - vRight).lengthSqr() ?
							vLeft : vRight;
					}
							
					dir = pPM->m_moveDodge;

					if (pPM->m_moveAfterBackWard == Vector3() || 
						(vTargetToCus - dir).lengthSqr() < (vTargetToCus - pPM->m_moveAfterBackWard).lengthSqr())
					{
						pPM->m_moveAfterBackWard = dir;
						pPM->m_moveBackWard = pPM->m_collisionPlane[i].getOutsideN() + dir;
						//pPM->m_moveBackWard = curPos - m_targetPostion;
					}
					else
					{
						dir = pPM->m_moveAfterBackWard;
					}
					pPM->m_backward = true;
				}
				
			}

			if (pPM->m_collisionCount == 0)  // falling
			{
				pPM->m_fallingTime += 0.006f;
				float velocityOfFalling = -1 * (pPM->m_accelerationOfGravity * pPM->m_fallingTime);
				dir = Vector3(0, velocityOfFalling, 0);

				pSN->m_base.setPos(curPos + dir);
			} 
			else  // has floor
			{
				pPM->m_fallingTime = 0;
				float dsqr = (m_targetPostion - curPos).lengthSqr();

				bool reached = true;
				if (dsqr > 0.01f)
				{
					// not at the spot yet
					Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
					static float speed = 1.4f;
					float allowedDisp = speed * pRealEvt->m_frameTime;

					if (pPM->m_backward)  // collision happened
					{
						if (pPM->m_collisionCount <= 1)  // if no more collision and no need to move backward
						{
							dir = pPM->m_moveAfterBackWard;
							pPM->m_moveAfterBackWard = Vector3();
							pPM->m_moveBackWard = Vector3();
							pPM->m_backward = false;
						}
						else {
							dir = pPM->m_moveBackWard;  // if collision happened and dir is set, means go back a little
						}
					}
					else
					{
						dir = (m_targetPostion - curPos);
						dir.m_y = 0;
						curPos = Vector3(curPos.getX(), pPM->m_curStandPlane.getFloorHeight(), curPos.getZ());
					}

					dir.normalize();
					float dist = sqrt(dsqr);
					if (dist > allowedDisp)
					{
						dist = allowedDisp; // can move up to allowedDisp
						reached = false; // not reaching destination yet
					}

					// instantaneous turn
					pSN->m_base.turnInDirection(m_targetPostion - curPos, 3.1415f);
					pSN->m_base.setPos(curPos + dir * dist);
				}

				if (reached)
				{
					m_state = STANDING;

					// target has been reached. need to notify all same level state machines (components of parent)
					{
						PE::Handle h("SoldierNPCMovementSM_Event_TARGET_REACHED", sizeof(SoldierNPCMovementSM_Event_TARGET_REACHED));
						Events::SoldierNPCMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) SoldierNPCMovementSM_Event_TARGET_REACHED();

						PE::Handle hParent = getFirstParentByType<Component>();
						if (hParent.isValid())
						{
							hParent.getObject<Component>()->handleEvent(pOutEvt);
						}

						// release memory now that event is processed
						h.release();
					}

					if (m_state == STANDING)
					{
						// no one has modified our state based on TARGET_REACHED callback
						// this means we are not going anywhere right now
						// so can send event to animation state machine to stop
						{
							Events::SoldierNPCAnimSM_Event_STOP evt;

							SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
							pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&evt);
						}
					}
				}
				else {  // not reached

				}
			}
		}
	}
}

}}




