#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Lua/EventGlue/EventDataCreators.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/RootSceneNode.h"

#include "NaziCar.h"
#include "NaziCarMovementSM.h"
#include "NaziCarBehaviorSM.h"

#include "PrimeEngine/PhysicsEngine/PhysicsManager.h"


using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl {
namespace Events {

PE_IMPLEMENT_CLASS1(Event_CREATE_NAZICAR, PE::Events::Event);

void Event_CREATE_NAZICAR::SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM)
{
	static const struct luaL_Reg l_Event_CREATE_NAZICAR[] = {
		{ "Construct", l_Construct },
		{ NULL, NULL } // sentinel
	};

	// register the functions in current lua table which is the table for Event_CreateNaziCar
	luaL_register(luaVM, 0, l_Event_CREATE_NAZICAR);
}

int Event_CREATE_NAZICAR::l_Construct(lua_State* luaVM)
{
	PE::Handle h("EVENT", sizeof(Event_CREATE_NAZICAR));

	// get arguments from stack
	int numArgs, numArgsConst;
	numArgs = numArgsConst = 22;

	PE::GameContext *pContext = (PE::GameContext*)(lua_touserdata(luaVM, -numArgs--));

	Event_CREATE_NAZICAR *pEvt = new(h) Event_CREATE_NAZICAR(pContext->m_gameThreadThreadOwnershipMask);

	const char* name = lua_tostring(luaVM, -numArgs--);
	const char* package = lua_tostring(luaVM, -numArgs--);
	const char* wayPointName = lua_tostring(luaVM, -numArgs--);
	
	pEvt->m_carID = (int)lua_tointeger(luaVM, -numArgs--);
	pEvt->m_frictionCoef = (float)lua_tonumber(luaVM, -numArgs--);
	pEvt->m_mass = (float)lua_tonumber(luaVM, -numArgs--);
	pEvt->m_horsepower = (float)lua_tonumber(luaVM, -numArgs--);
	pEvt->m_speedMax = (float)lua_tonumber(luaVM, -numArgs--);

	float positionFactor = 1.0f / 100.0f;
	Vector3 pos, u, v, n;
	pos.m_x = (float)lua_tonumber(luaVM, -numArgs--) * positionFactor;
	pos.m_y = (float)lua_tonumber(luaVM, -numArgs--) * positionFactor;
	pos.m_z = (float)lua_tonumber(luaVM, -numArgs--) * positionFactor;

	u.m_x = (float)lua_tonumber(luaVM, -numArgs--); u.m_y = (float)lua_tonumber(luaVM, -numArgs--); u.m_z = (float)lua_tonumber(luaVM, -numArgs--);
	v.m_x = (float)lua_tonumber(luaVM, -numArgs--); v.m_y = (float)lua_tonumber(luaVM, -numArgs--); v.m_z = (float)lua_tonumber(luaVM, -numArgs--);
	n.m_x = (float)lua_tonumber(luaVM, -numArgs--); n.m_y = (float)lua_tonumber(luaVM, -numArgs--); n.m_z = (float)lua_tonumber(luaVM, -numArgs--);

	pEvt->m_peuuid = LuaGlue::readPEUUID(luaVM, -numArgs--);

	// set data values before popping memory off stack
	StringOps::writeToString(name, pEvt->m_meshFilename, 255);
	StringOps::writeToString(package, pEvt->m_package, 255);
	StringOps::writeToString(wayPointName, pEvt->m_patrolWayPoint, 32);

	lua_pop(luaVM, numArgsConst); //Second arg is a count of how many to pop

	pEvt->m_base.loadIdentity();
	pEvt->m_base.setPos(pos);
	pEvt->m_base.setU(u);
	pEvt->m_base.setV(v);
	pEvt->m_base.setN(n);
	pEvt->hasCustomOrientation = true;
	pEvt->m_pos = pos;
	pEvt->m_u = u;
	pEvt->m_v = v;
	pEvt->m_n = n;

	LuaGlue::pushTableBuiltFromHandle(luaVM, h);

	return 1;
}
};  // namespace Events

namespace Components {

PE_IMPLEMENT_CLASS1(NaziCar, Component);

// create nazicar form creation event
NaziCar::NaziCar(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CREATE_NAZICAR *pEvt)
	: Component(context, arena, hMyself)
{
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pEvt->m_threadOwnershipMask);

	PE::Handle hMainSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pMainSN = new(hMainSN) SceneNode(*m_pContext, m_arena, hMainSN);
	pMainSN->addDefaultComponents();

	pMainSN->m_base.setPos(pEvt->m_pos);
	pMainSN->m_base.setU(pEvt->m_u);
	pMainSN->m_base.setV(pEvt->m_v);
	pMainSN->m_base.setN(pEvt->m_n);


	RootSceneNode::Instance()->addComponent(hMainSN);

	// add the scene node as component of nazicar without any handlers. this is just data driven way to locate scnenode for nazicar's components
	{
		static int allowedEvts[] = {0};
		addComponent(hMainSN, &allowedEvts[0]);
	}

	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
	pMeshInstance->addDefaultComponents();

	pMeshInstance->initFromFile(pEvt->m_meshFilename, pEvt->m_package, pEvt->m_threadOwnershipMask);

	pMainSN->addComponent(hMeshInstance);

	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pEvt->m_threadOwnershipMask);

#if 1
	// add movement state machine to NaziCar
	PE::Handle hNaziCarMovementSM("NaziCarMovementSM", sizeof(NaziCarMovementSM));
	NaziCarMovementSM *pNaziCarMovementSM = new(hNaziCarMovementSM) NaziCarMovementSM(*m_pContext, m_arena, hNaziCarMovementSM);
	pNaziCarMovementSM->addDefaultComponents();

	// add it to NaziCar
	addComponent(hNaziCarMovementSM);

	// add behavior state machine ot NaziCar
	PE::Handle hNaziCarBehaviorSM("NaziCarBehaviorSM", sizeof(NaziCarBehaviorSM));
	NaziCarBehaviorSM *pNaziCarBehaviorSM = new(hNaziCarBehaviorSM) NaziCarBehaviorSM(*m_pContext, m_arena, hNaziCarBehaviorSM, hNaziCarMovementSM);
	pNaziCarBehaviorSM->addDefaultComponents();

	// add it to NaziCar
	addComponent(hNaziCarBehaviorSM);

	StringOps::writeToString(pEvt->m_patrolWayPoint, pNaziCarBehaviorSM->m_curPatrolWayPoint, 32);
	pNaziCarBehaviorSM->m_havePatrolWayPoint = StringOps::length(pNaziCarBehaviorSM->m_curPatrolWayPoint) > 0;


	PhysicsManager::add(pMainSN, pEvt->m_meshFilename);
	pMainSN->m_physicsComponent = PhysicsManager::getLastPhysicsComponentAddress();

	pMainSN->m_physicsComponent->m_id = pEvt->m_carID;
	pMainSN->m_physicsComponent->m_frictionCoef = pEvt->m_frictionCoef;
	pMainSN->m_physicsComponent->m_mass = pEvt->m_mass;
	pMainSN->m_physicsComponent->m_horsepower = pEvt->m_horsepower;
	pMainSN->m_physicsComponent->m_speedMax = pEvt->m_speedMax;

	pNaziCarBehaviorSM->start();

#endif
}

void NaziCar::addDefaultComponents()
{
	Component::addDefaultComponents();

	// custom methods of this component
}

}; // namespace Components
}; // namespace CharacterControl
