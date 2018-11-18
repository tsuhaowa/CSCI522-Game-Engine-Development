#ifndef _PE_NAZICAR_MOVEMENT_SM_H_
#define _PE_NAZICAR_MOVEMENT_SM_H_


#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/SceneNode.h"

#include "../Events/Events.h"

namespace CharacterControl{
// events that can be sent to this state machine or sent by this state machine (like TARGET_REACHED)
namespace Events
{

// sent by behavior state machine when a NaziCar has to go somewhere
struct NaziCarMovementSM_Event_MOVE_TO : public PE::Events::Event {
	PE_DECLARE_CLASS(NaziCarMovementSM_Event_MOVE_TO);

	NaziCarMovementSM_Event_MOVE_TO(Vector3 targetPos = Vector3());

	Vector3 m_targetPosition;
};

struct NaziCarMovementSM_Event_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(NaziCarMovementSM_Event_STOP);

	NaziCarMovementSM_Event_STOP()
	{}
};

// sent by this state machine to its components. probably to behavior state machine
struct NaziCarMovementSM_Event_TARGET_REACHED : public PE::Events::Event {
	PE_DECLARE_CLASS(NaziCarMovementSM_Event_TARGET_REACHED);

	NaziCarMovementSM_Event_TARGET_REACHED()
	{}
};

};
namespace Components {

// movement state machine talks to associated animation state machine
struct NaziCarMovementSM : public PE::Components::Component
{
	PE_DECLARE_CLASS(NaziCarMovementSM);
	
	enum States
	{
		STANDING,
		RUNNING_TO_TARGET,
		WALKING_TO_TARGET,
	};


	NaziCarMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);
	template<typename T> int sgn(T n);  // return sign of n

	//////////////////////////////////////////////////////////////////////////
	// utility
	//////////////////////////////////////////////////////////////////////////
	PE::Components::SceneNode *getParentsSceneNode();

	//////////////////////////////////////////////////////////////////////////
	// Component API and Event Handlers
	//////////////////////////////////////////////////////////////////////////
	//
	virtual void addDefaultComponents() ;
	//
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE)
	virtual void do_UPDATE(PE::Events::Event *pEvt);

	//////////////////////////////////////////////////////////////////////////
	// Member Variables
	//////////////////////////////////////////////////////////////////////////
	PE::Handle m_hAnimationSM;
	//
	// State
	Vector3 m_targetPostion;
	States m_state;
};

};
};


#endif


