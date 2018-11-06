#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"

#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);

CameraSceneNode::CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
{
	m_near = 0.05f;
	m_far = 2000.0f;
}
void CameraSceneNode::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
}

void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt)
{
	Handle hParentSN = getFirstParentByType<SceneNode>();
	if (hParentSN.isValid())
	{
		Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
		m_worldTransform = parentTransform * m_base;
	}
	
	Matrix4x4 &mref_worldTransform = m_worldTransform;

	Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]);
	Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]);
	Vector3 target = pos + n;
	Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

	m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

	m_worldTransform2 = mref_worldTransform;

	m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

	Vector3 pos2 = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
	Vector3 n2 = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
	Vector3 target2 = pos2 + n2;
	Vector3 up2 = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

	m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);
    
    PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
    
    PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
    if (aspect < 1.0f)
    {
        //ios portrait view
        static PrimitiveTypes::Float32 factor = 0.5f;
        verticalFov *= factor;
    }

	m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, 
		aspect,
		m_near, m_far);

	SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);
	
	// create 6 frustum planes

	// *** the second matrix is made by narrower FOV, so that objects would disappear within the camera view
	PrimitiveTypes::Float32 verticalFov2 = 0.318f * PrimitiveTypes::Constants::c_Pi_F32;
	Matrix4x4 m_viewToProjectedTransform2 = CameraOps::CreateProjectionMatrix(verticalFov2,
		aspect,
		m_near, m_far);
	// ***

	// Matrix4x4 mvp = m_viewToProjectedTransform * m_worldToViewTransform;  // this is normal version
	Matrix4x4 mvp = m_viewToProjectedTransform2 * m_worldToViewTransform;  // objects disappear within camera view
	mvp = mvp.transpose();
	Vector3 m1j = mvp.getU();  // m11, m12, m13
	Vector3 m2j = mvp.getV();  // m21, m22, m23
	Vector3 m3j = mvp.getN();  // m31, m32, m33
	Vector3 m4j = mvp.getPos();  // m41, m42, m43

	// Left clipping plane
	m_frustumPlanes[0].buildPlaneByPlus(m4j, m1j, mvp.m[3][3], mvp.m[3][0]);

	// Right clipping plane
	m_frustumPlanes[1].buildPlaneByMinus(m4j, m1j, mvp.m[3][3], mvp.m[3][0]);

	// Top clipping plane
	m_frustumPlanes[2].buildPlaneByMinus(m4j, m2j, mvp.m[3][3], mvp.m[3][1]);

	// Bottom clipping plane
	m_frustumPlanes[3].buildPlaneByPlus(m4j, m2j, mvp.m[3][3], mvp.m[3][1]);

	// Near clipping plane
	m_frustumPlanes[4].buildPlaneByPlus(m4j, m3j, mvp.m[3][3], mvp.m[3][2]);

	// Far clipping plane
	m_frustumPlanes[5].buildPlaneByMinus(m4j, m3j, mvp.m[3][3], mvp.m[3][2]);
	
}

}; // namespace Components
}; // namespace PE
