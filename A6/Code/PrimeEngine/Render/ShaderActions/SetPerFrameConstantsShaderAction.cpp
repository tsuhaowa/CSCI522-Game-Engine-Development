
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/ResourceBufferGPU.h"
#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

#include "PrimeEngine/Scene/CameraManager.h"
#include "CharacterControl/Characters/SoldierNPC.h"


// Sibling/Children includes
#include "SetPerFrameConstantsShaderAction.h"

namespace PE {
using namespace Components;

void SetPerFrameConstantsShaderAction::bindToPipeline(Effect *pCurEffect /* = NULL*/)
{
#if PE_PLAT_IS_PSVITA
	PSVitaRenderer *pPSVitaRenderer = static_cast<PSVitaRenderer *>(m_pContext->getGPUScreen());
	memcpy(s_pBuffer, &m_data, sizeof(m_data));
	SceGxmErrorCode errCode;
	errCode = sceGxmSetVertexUniformBuffer(pPSVitaRenderer->m_context, 0, s_pBuffer); // todo: use buferring
	PEASSERT(errCode == SCE_OK, "Error setting unfiorm buffer");
	errCode = sceGxmSetFragmentUniformBuffer(pPSVitaRenderer->m_context, 0, s_pBuffer); // todo: use buferring
	PEASSERT(errCode == SCE_OK, "Error setting unfiorm buffer");
#	elif APIABSTRACTION_D3D11
	D3D11Renderer *pD3D11Renderer = static_cast<D3D11Renderer *>(m_pContext->getGPUScreen());
	ID3D11Device *pDevice = pD3D11Renderer->m_pD3DDevice;
	ID3D11DeviceContext *pDeviceContext = pD3D11Renderer->m_pD3DContext;

	Effect::setConstantBuffer(pDevice, pDeviceContext, s_pBuffer, 0, &m_data, sizeof(Data));
#	elif APIABSTRACTION_D3D9
	D3D9Renderer *pD3D9Renderer = static_cast<D3D9Renderer *>(m_pContext->getGPUScreen());
	LPDIRECT3DDEVICE9 pDevice = pD3D9Renderer->m_pD3D9Device;
		
	pDevice->SetVertexShaderConstantF(0, (const float *)(&m_data.gGameTimes[0]), 1);
	pDevice->SetPixelShaderConstantF(0, (const float *)(&m_data.gGameTimes[0]), 1);

	setWind(pDevice);
#	endif
}

void SetPerFrameConstantsShaderAction::setWind(LPDIRECT3DDEVICE9& pDevice)
{
	Handle hCam = CameraManager::Instance()->getActiveCameraHandle();
	CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
	m_windSource[0].gWind[0] = pcam->m_base.getPos().getX();
	m_windSource[0].gWind[2] = pcam->m_base.getPos().getZ();

	// get Solider's pos and set to gWind
	using namespace CharacterControl::Components;

	int solIndex = 0;
	for (int i = 1; i < 3; ++i) {
		solIndex = RootSceneNode::Instance()->getFirstComponentIndex<SoldierNPC>(solIndex);
		if (solIndex == -1) break;

		SoldierNPC *soldier = RootSceneNode::Instance()->getComponentByIndex<SoldierNPC>(solIndex);
		SceneNode *pMainSN = soldier->getFirstComponent<SceneNode>();
		assert(pMainSN);

		m_windSource[i].gWind[0] = pMainSN->m_base.getPos().getX();
		m_windSource[i].gWind[2] = pMainSN->m_base.getPos().getZ();
		++solIndex;
	}

	pDevice->SetVertexShaderConstantF(163, (const float *)(&m_windSource), 3);
	pDevice->SetPixelShaderConstantF(163, (const float *)(&m_windSource), 3);
}

void SetPerFrameConstantsShaderAction::unbindFromPipeline(Components::Effect *pCurEffect/* = NULL*/)
{}

void SetPerFrameConstantsShaderAction::releaseData()
{}

}; // namespace PE
