#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/Scene/Skeleton.h"
#include "DefaultAnimationSM.h"
#include "Light.h"

#include "PrimeEngine/GameObjectModel/Camera.h"

// Sibling/Children includes
#include "MeshInstance.h"
#include "MeshManager.h"
#include "SceneNode.h"
#include "CameraManager.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"

#include "CharacterControl/PhysicsManager.h"

namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshInstance, Component);

MeshInstance::MeshInstance(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
: Component(context, arena, hMyself)
, m_culledOut(false)
{
	
}

void MeshInstance::addDefaultComponents()
{
	Component::addDefaultComponents();
}

void MeshInstance::initFromFile(const char *assetName, const char *assetPackage,
		int &threadOwnershipMask)
{
	// for debugging
	snprintf(m_meshFileName, 127, "%s", assetName);

	Handle h = m_pContext->getMeshManager()->getAsset(assetName, assetPackage, threadOwnershipMask);

	initFromRegisteredAsset(h);
	createPhysicsManager(h);
}

bool MeshInstance::hasSkinWeights()
{
	Mesh *pMesh = m_hAsset.getObject<Mesh>();
	return pMesh->m_hSkinWeightsCPU.isValid();
}

void MeshInstance::initFromRegisteredAsset(const PE::Handle &h)
{
	m_hAsset = h;
	//add this instance as child to Mesh so that skin knows what to draw
	static int allowedEvts[] = {0};
	m_hAsset.getObject<Component>()->addComponent(m_hMyself, &allowedEvts[0]);
}

void MeshInstance::createPhysicsManager(PE::Handle &h)
{
	PE::Handle hPhyManager("PhysicsManager", sizeof(PhysicsManager));
	PhysicsManager *pPhyManager = new(hPhyManager) PhysicsManager(*m_pContext, m_arena, hPhyManager);
	pPhyManager->addDefaultComponents();

	Mesh *pMesh = h.getObject<Mesh>();
	PhysicsManager *rhsPhyManager = pMesh->getFirstComponent<PhysicsManager>();
	*pPhyManager = *rhsPhyManager;

	// add PhysicsManager to MeshInstance
	addComponent(hPhyManager);
}

}; // namespace Components
}; // namespace PE