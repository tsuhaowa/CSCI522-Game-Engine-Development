#ifndef _CHARACTER_CONTROL_NAZICAR_
#define _CHARACTER_CONTROL_NAZICAR_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Matrix4x4.h"
#include "PrimeEngine/Events/StandardEvents.h"


namespace CharacterControl {
	namespace Events {
		struct Event_CREATE_NAZICAR : public PE::Events::Event_CREATE_MESH
		{
			PE_DECLARE_CLASS(Event_CREATE_NAZICAR);

			Event_CREATE_NAZICAR(int &threadOwnershipMask) : PE::Events::Event_CREATE_MESH(threadOwnershipMask) {}
			// override SetLuaFunctions() since we are adding custom Lua interface
			static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

			// Lua interface prefixed with l_
			static int l_Construct(lua_State* luaVM);

			Matrix4x4 m_base;

			char m_patrolWayPoint[32];
			int m_carID;
			float m_frictionCoef;
			float m_mass;
			float m_horsepower;
			float m_torque;
			float m_speedMax;
			static int state;

			PEUUID m_peuuid; // unique object id
		};
	}
	namespace Components {

		struct NaziCar : public PE::Components::Component
		{
			PE_DECLARE_CLASS(NaziCar);

			NaziCar(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CREATE_NAZICAR *pEvt);

			virtual void addDefaultComponents();
		};
	}; // namespace Components
}; // namespace CharacterControl
#endif

