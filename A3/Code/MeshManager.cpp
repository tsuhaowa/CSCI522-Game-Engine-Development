// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "MeshManager.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Geometry/SkeletonCPU/SkeletonCPU.h"

#include "PrimeEngine/Scene/RootSceneNode.h"

#include "Light.h"

// Sibling/Children includes

#include "MeshInstance.h"
#include "Skeleton.h"
#include "SceneNode.h"
#include "DrawList.h"
#include "SH_DRAW.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"

namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshManager, Component);
MeshManager::MeshManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_assets(context, arena, 256)
{
}

PE::Handle MeshManager::getAsset(const char *asset, const char *package, int &threadOwnershipMask)
{
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "%s/%s", package, asset);
	
	int index = m_assets.findIndex(key);
	if (index != -1)
	{
		return m_assets.m_pairs[index].m_value;
	}
	Handle h;

	if (StringOps::endswith(asset, "skela"))
	{
		PE::Handle hSkeleton("Skeleton", sizeof(Skeleton));
		Skeleton *pSkeleton = new(hSkeleton) Skeleton(*m_pContext, m_arena, hSkeleton);
		pSkeleton->addDefaultComponents();

		pSkeleton->initFromFiles(asset, package, threadOwnershipMask);
		h = hSkeleton;
	}
	else if (StringOps::endswith(asset, "mesha"))
	{
		MeshCPU mcpu(*m_pContext, m_arena);
		mcpu.ReadMesh(asset, package, "");
		
		PE::Handle hMesh("Mesh", sizeof(Mesh));
		Mesh *pMesh = new(hMesh) Mesh(*m_pContext, m_arena, hMesh);
		pMesh->addDefaultComponents();

		pMesh->loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

#if PE_API_IS_D3D11
		// todo: work out how lods will work
		//scpu.buildLod();
#endif
        // generate collision volume here. or you could generate it in MeshCPU::ReadMesh()
        pMesh->m_performBoundingVolumeCulling = true; // will now perform tests for this mesh

		// find all points in mesh

		PositionBufferCPU *pVB = pMesh->m_hPositionBufferCPU.getObject<PositionBufferCPU>();
		
		float minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN, minZ = INT_MAX, maxZ = INT_MIN;
		for (int i = 0; i < pVB->m_values.m_size / 3; ++i)
		{
			float x = pVB->m_values[i * 3];
			float y = pVB->m_values[i * 3 + 1];
			float z = pVB->m_values[i * 3 + 2];
			
			if (x < minX) minX = x;
			else if (x > maxX) maxX = x;
			if (y < minY) minY = y;
			else if (y > maxY) maxY = y;
			if (z < minZ) minZ = z;
			else if (z > maxZ) maxZ = z;
		}

		// build AABB box by 4 matrices from 4 vertices
		pMesh->m_AABB[0].setPos(Vector3(minX, minY, minZ));
		pMesh->m_AABB[0].setU(Vector3(maxX - minX, 0, 0));
		pMesh->m_AABB[0].setV(Vector3(0, maxY - minY, 0));
		pMesh->m_AABB[0].setN(Vector3(0, 0, maxZ - minZ));

		pMesh->m_AABB[1].setPos(Vector3(maxX, minY, maxZ));
		pMesh->m_AABB[1].setU(Vector3(minX - maxX, 0, 0));
		pMesh->m_AABB[1].setV(Vector3(0, maxY - minY, 0));
		pMesh->m_AABB[1].setN(Vector3(0, 0, minZ - maxZ));

		pMesh->m_AABB[2].setPos(Vector3(maxX, maxY, minZ));
		pMesh->m_AABB[2].setU(Vector3(minX - maxX, 0, 0));
		pMesh->m_AABB[2].setV(Vector3(0, minY - maxY, 0));
		pMesh->m_AABB[2].setN(Vector3(0, 0, maxZ - minZ));

		pMesh->m_AABB[3].setPos(Vector3(minX, maxY, maxZ));
		pMesh->m_AABB[3].setU(Vector3(maxX - minX, 0, 0));
		pMesh->m_AABB[3].setV(Vector3(0, minY - maxY, 0));
		pMesh->m_AABB[3].setN(Vector3(0, 0, minZ - maxZ));

		// build AABB box by 8 points
		// lower vertices
		pMesh->m_AABBPoints[0] = Vector3(minX, minY, minZ);
		pMesh->m_AABBPoints[1] = Vector3(minX, minY, maxZ);
		pMesh->m_AABBPoints[2] = Vector3(maxX, minY, minZ);
		pMesh->m_AABBPoints[3] = Vector3(maxX, minY, maxZ);

		// higher vertices
		pMesh->m_AABBPoints[4] = Vector3(minX, maxY, minZ);
		pMesh->m_AABBPoints[5] = Vector3(minX, maxY, maxZ);
		pMesh->m_AABBPoints[6] = Vector3(maxX, maxY, minZ);
		pMesh->m_AABBPoints[7] = Vector3(maxX, maxY, maxZ);

		char buffer[90];
		snprintf(buffer, 90, "x(%f, %f), y(%f, %f), z(%f, %f) \n", minX, maxX, minY, maxY, minZ, maxZ);
		OutputDebugStringA(buffer);

		h = hMesh;
	}


	PEASSERT(h.isValid(), "Something must need to be loaded here");

	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
	return h;
}

void MeshManager::registerAsset(const PE::Handle &h)
{
	static int uniqueId = 0;
	++uniqueId;
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "__generated_%d", uniqueId);
	
	int index = m_assets.findIndex(key);
	PEASSERT(index == -1, "Generated meshes have to be unique");
	
	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
}

}; // namespace Components
}; // namespace PE
