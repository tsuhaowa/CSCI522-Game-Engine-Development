#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "NaziCarMovementSM.h"
#include "NaziCar.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a NaziCar should move
namespace Events{

PE_IMPLEMENT_CLASS1(NaziCarMovementSM_Event_MOVE_TO, Event);

NaziCarMovementSM_Event_MOVE_TO::NaziCarMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos)
{ }

PE_IMPLEMENT_CLASS1(NaziCarMovementSM_Event_STOP, Event);

PE_IMPLEMENT_CLASS1(NaziCarMovementSM_Event_TARGET_REACHED, Event);
}

namespace Components{

PE_IMPLEMENT_CLASS1(NaziCarMovementSM, Component);


NaziCarMovementSM::NaziCarMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) 
: Component(context, arena, hMyself)
, m_state(STANDING)
{}

SceneNode *NaziCarMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
		
	}
	return NULL;
}

void NaziCarMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(NaziCarMovementSM_Event_MOVE_TO, NaziCarMovementSM::do_NaziCarMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(NaziCarMovementSM_Event_STOP, NaziCarMovementSM::do_NaziCarMovementSM_Event_STOP);
	
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, NaziCarMovementSM::do_UPDATE);
}

void NaziCarMovementSM::do_NaziCarMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	NaziCarMovementSM_Event_MOVE_TO *pRealEvt = (NaziCarMovementSM_Event_MOVE_TO *)(pEvt);
	
	// change state of this state machine
	m_state = WALKING_TO_TARGET;
	m_targetPostion = pRealEvt->m_targetPosition;

	// make sure the animations are playing
	
	//PE::Handle h("NaziCarAnimSM_Event_WALK", sizeof(NaziCarAnimSM_Event_WALK));
	//Events::NaziCarAnimSM_Event_WALK *pOutEvt = new(h) NaziCarAnimSM_Event_WALK();
	//
	//NaziCar *pSol = getFirstParentByTypePtr<NaziCar>();
	//pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

	//// release memory now that event is processed
	//h.release();
}

void NaziCarMovementSM::do_NaziCarMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	/*Events::NaziCarAnimSM_Event_STOP Evt;

	NaziCar *pSol = getFirstParentByTypePtr<NaziCar>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);*/
}

void NaziCarMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_state == WALKING_TO_TARGET)
	{
		// see if parent has scene node component
		SceneNode *pSN = getParentsSceneNode();
		if (pSN)
		{
			Vector3 curPos = pSN->m_base.getPos();
			float dsqr = (m_targetPostion - curPos).lengthSqr();

			bool reached = true;
			if (dsqr > 0.01f)
			{
				// not at the spot yet
				Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
				static float speed = 1.4f;
				float allowedDisp = speed * pRealEvt->m_frameTime;

				Vector3 dir = (m_targetPostion - curPos);
				dir.normalize();
				float dist = sqrt(dsqr);
				if (dist > allowedDisp)
				{
					dist = allowedDisp; // can move up to allowedDisp
					reached = false; // not reaching destination yet
				}

				// instantaneous turn
				pSN->m_base.turnInDirection(-dir, 3.1415f);
				pSN->m_base.setPos(curPos + dir * dist);
			}

			if (reached)
			{
				m_state = STANDING;
				
				// target has been reached. need to notify all same level state machines (components of parent)
				{
					PE::Handle h("NaziCarMovementSM_Event_TARGET_REACHED", sizeof(NaziCarMovementSM_Event_TARGET_REACHED));
					Events::NaziCarMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) NaziCarMovementSM_Event_TARGET_REACHED();

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
						/*events::nazicaranimsm_event_stop evt;
						
						nazicar *psol = getfirstparentbytypeptr<nazicar>();
						psol->getfirstcomponent<pe::components::scenenode>()->handleevent(&evt);*/
					}
				}
			}
		}
	}
}

}}




