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
	
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, NaziCarMovementSM::do_UPDATE);
}

void NaziCarMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{

	// see if parent has scene node component
	SceneNode *pSN = getParentsSceneNode();
	if (pSN)
	{
		Vector3 curPos = pSN->m_base.getPos();
		float v = pSN->m_physicsComponent->m_velocity;
		if (v == 0 && pSN->m_physicsComponent->m_forceFB == 0)
		{  // no power and no speed, must remain stop and reset the forceLR
			pSN->m_physicsComponent->m_forceLR = 0;
			return;
		}

		//--- force part begin ---//
		float fLR = pSN->m_physicsComponent->m_forceLR;
		float fFB = fabs(pSN->m_physicsComponent->m_forceFB);
		float f;  // friction

		if (fLR != 0 && fFB != 0)
		{  // only if having power and turning, calculate the component force
			fLR = 3 * sgn<float>(fLR);
			fFB = pow(pSN->m_physicsComponent->m_forceFB, 2.0) - pow(fLR, 2.0);  // straight component force
			assert(fFB > 0);  // total horsepower must greater than torque, so that can count component force
			fFB = sqrt(fFB);
		}
			
		fFB *= sgn<float>(pSN->m_physicsComponent->m_forceFB);

		if (pSN->m_physicsComponent->m_curStandPlane != nullptr)
		{
			if (v != 0)
			{  // let the sign of friction same as the direction of v
				f = pSN->m_physicsComponent->m_curStandPlane->m_frictionCoef * pSN->m_physicsComponent->m_mass * sgn<float>(v);
			}
			else if (fFB != 0)
			{  // let the sign of friction same as the direction of forceFB, if speed is 0
				f = pSN->m_physicsComponent->m_curStandPlane->m_frictionCoef * pSN->m_physicsComponent->m_mass * sgn<float>(fFB);
			}

			if (v == 0 && fabs(fFB) <= fabs(f)) return;  // car is stopping, and the component force is not enough to move the car

			fFB -= f;  // actual force for straight direction
		}
		else 
		{
			fFB = fLR = 0;  // free falling
		}
		//--- force part end ---//

		//--- acceleration part begin ---//
		float aFB = fFB / pSN->m_physicsComponent->m_mass;
		float aLR = fLR / pSN->m_physicsComponent->m_mass;
		//--- acceleration part end ---//

		//--- velocity part begin ---//
		v += aFB;  // pure speed value, positive is forward, negative is backward
		if (v > pSN->m_physicsComponent->m_speedMax)
			v = pSN->m_physicsComponent->m_speedMax;
		else if (v < -pSN->m_physicsComponent->m_speedMax)
			v = -pSN->m_physicsComponent->m_speedMax;
		else if (pSN->m_physicsComponent->m_forceFB == 0 && (sgn<float>(v) * sgn<float>(pSN->m_physicsComponent->m_velocity)) == -1)  // no power, speed should be 0 when crossing 0
			v = 0;

		aLR *= (fabs(v) / pSN->m_physicsComponent->m_speedMax);  // let aLR relating to the current speed

		Vector3 dir;
		if (v != 0) {  // has v, calcualte the direction
			dir = v * pSN->m_physicsComponent->m_carDirAfterTransform[0] + aLR * pSN->m_physicsComponent->m_carDirAfterTransform[3];  // [3] is right
			dir.normalize();
		}
		//--- velocity part end ---//

		// instantaneous turn
		if (!(dir == Vector3()))  // cannot turn 0 degree
		{
			pSN->m_base.turnInDirection( (v<0) ? dir : -dir, 3.14159 );  // do not turn direction when reverse
		}
				
		pSN->m_base.setPos(curPos + dir * fabs(v));

		// setoff force and sync velocity
		pSN->m_physicsComponent->m_forceFB = 0;
		pSN->m_physicsComponent->m_forceLR = 0;
		pSN->m_physicsComponent->m_velocity = v;
	}
}

template<typename T>
int NaziCarMovementSM::sgn(T n)
{
	return (n > T(0)) - (n < T(0));
}

}}



