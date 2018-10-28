#include "PhysicsComponent.h"
#include "PrimeEngine/Scene/SceneNode.h"

#include <string.h>

PhysicsComponent::PhysicsComponent()
{
	m_id = 0;
	m_velocity = 0.0f;
	m_mass = 0.0f;
	m_acceleration = 0.0f;
	m_accelerationOfGravity = 0.8f;
	m_gravity = 0.0f;
	m_fallingTime = 0.0f;
	m_frictionCoef = 0.0f;
	m_horsepower = 0.0f;
	m_speedMax = 0.0f;
	m_collisionCheck = false;
	m_collisionCount = 0;
	m_doDodge = false;
	m_stuckCheck = false;
}

void PhysicsComponent::construct(SceneNode *t_sceneNode, char *t_meshFileName)
{
	m_meshFileName = t_meshFileName;
	m_pMainSN = t_sceneNode;

	init();
}

void PhysicsComponent::init()
{
	m_prevMainBase = m_postMainBase = m_pMainSN->m_base;

	if (strcmp(m_meshFileName, "soldier") == 0)
		init_Soldier();
	else
		init_StaticMesh();
}

void PhysicsComponent::init_Soldier()
{
	// init skip collision list
	buildCollisionSkipList_Soldier();

	// set soldier bounding box in DefaultAnimationSM.cpp
}

void PhysicsComponent::init_StaticMesh()
{
	// init bounding box
	Mesh *pMesh = m_pMainSN->getFirstComponent<MeshInstance>()->getFirstParentByTypePtr<Mesh>();
	PositionBufferCPU *pVB = pMesh->m_hPositionBufferCPU.getObject<PositionBufferCPU>();

	float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX, minZ = FLT_MAX, maxZ = -FLT_MAX;
	for (int i = 0; i < pVB->m_values.m_size / 3; ++i)
	{
		float x = pVB->m_values[i * 3];
		float y = pVB->m_values[i * 3 + 1];
		float z = pVB->m_values[i * 3 + 2];

		PhysicsComponent::setExtremeValue(x, y, z, minX, maxX, minY, maxY, minZ, maxZ);
	}

	buildBoundingVolume(minX, maxX, minY, maxY, minZ, maxZ);
	buildBoundingVolumeAfterTransform(m_prevMainBase);
}

void PhysicsComponent::buildBoundingVolume(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
	m_boundingBox[0].setPos(Vector3(minX, minY, minZ));
	m_boundingBox[0].setU(Vector3(maxX - minX, 0, 0));
	m_boundingBox[0].setV(Vector3(0, maxY - minY, 0));
	m_boundingBox[0].setN(Vector3(0, 0, maxZ - minZ));

	m_boundingBox[1].setPos(Vector3(maxX, minY, maxZ));
	m_boundingBox[1].setU(Vector3(minX - maxX, 0, 0));
	m_boundingBox[1].setV(Vector3(0, maxY - minY, 0));
	m_boundingBox[1].setN(Vector3(0, 0, minZ - maxZ));

	m_boundingBox[2].setPos(Vector3(maxX, maxY, minZ));
	m_boundingBox[2].setU(Vector3(minX - maxX, 0, 0));
	m_boundingBox[2].setV(Vector3(0, minY - maxY, 0));
	m_boundingBox[2].setN(Vector3(0, 0, maxZ - minZ));

	m_boundingBox[3].setPos(Vector3(minX, maxY, maxZ));
	m_boundingBox[3].setU(Vector3(maxX - minX, 0, 0));
	m_boundingBox[3].setV(Vector3(0, minY - maxY, 0));
	m_boundingBox[3].setN(Vector3(0, 0, minZ - maxZ));

	// build AABB box by 8 points
	// lower vertices
	m_boundingBoxVertex[0] = Vector3(minX, minY, minZ);
	m_boundingBoxVertex[1] = Vector3(minX, minY, maxZ);
	m_boundingBoxVertex[2] = Vector3(maxX, minY, minZ);
	m_boundingBoxVertex[3] = Vector3(maxX, minY, maxZ);

	// higher vertices
	m_boundingBoxVertex[4] = Vector3(minX, maxY, minZ);
	m_boundingBoxVertex[5] = Vector3(minX, maxY, maxZ);
	m_boundingBoxVertex[6] = Vector3(maxX, maxY, minZ);
	m_boundingBoxVertex[7] = Vector3(maxX, maxY, maxZ);
}

void PhysicsComponent::buildBoundingVolumeAfterTransform(const Matrix4x4 &worldMatrix)
{
	for (int i = 0; i < 4; ++i)
		m_boundingBoxAfterTransform[i] = worldMatrix * m_boundingBox[i];
	for (int i = 0; i < 8; ++i)
		m_boundingBoxVertexAfterTransform[i] = worldMatrix * m_boundingBoxVertex[i];

	m_upVector = m_boundingBoxVertexAfterTransform[4] - m_boundingBoxVertexAfterTransform[0];

	// all normal vectors point to the inside of the box
	m_boundingBoxPlanes[0].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[0], m_boundingBoxVertexAfterTransform[1], m_boundingBoxVertexAfterTransform[2]);  // bottom
	m_boundingBoxPlanes[1].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[4], m_boundingBoxVertexAfterTransform[0], m_boundingBoxVertexAfterTransform[6]);  // match to [4]
	m_boundingBoxPlanes[2].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[5], m_boundingBoxVertexAfterTransform[1], m_boundingBoxVertexAfterTransform[4]);
	m_boundingBoxPlanes[3].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[4], m_boundingBoxVertexAfterTransform[6], m_boundingBoxVertexAfterTransform[5]);  // top
	m_boundingBoxPlanes[4].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[7], m_boundingBoxVertexAfterTransform[3], m_boundingBoxVertexAfterTransform[5]);
	m_boundingBoxPlanes[5].buildPlaneByPoints(m_boundingBoxVertexAfterTransform[6], m_boundingBoxVertexAfterTransform[2], m_boundingBoxVertexAfterTransform[7]);

	m_boundingBoxCenter = (m_boundingBoxVertexAfterTransform[0] + m_boundingBoxVertexAfterTransform[7]) / 2.0f;
}

void PhysicsComponent::buildCollisionSkipList_Soldier()
{
	char *skipBuffer = NULL;

	skipBuffer = "SoldierTransform.mesha";  // mesha
	m_collisionSkipList.push_back(skipBuffer);

	skipBuffer = "m98.x_m98main_mesh.mesha";  // mesha
	m_collisionSkipList.push_back(skipBuffer);
}

void PhysicsComponent::setExtremeValue(const float &x, const float &y, const float &z, float &minX, float &maxX,
	float &minY, float &maxY, float &minZ, float &maxZ)
{
	if (x < minX) minX = x;
	if (x > maxX) maxX = x;
	if (y < minY) minY = y;
	if (y > maxY) maxY = y;
	if (z < minZ) minZ = z;
	if (z > maxZ) maxZ = z;
}