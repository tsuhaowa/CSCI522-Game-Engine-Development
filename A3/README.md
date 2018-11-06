# Demo Video
[youtube link](https://youtu.be/o8wbik_9D8s)

# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- ccontrollvl0 level for debugging.


# General Description
- Build non-Axis Aligned Bounding Box
- Build frustum planes to cull out objects
- Build 2nd narrower frustum plane to make demo clearer


# Revised Files

#### MeshManager.cpp
	set up AABB matrix from local vertex of each mesh

#### Mesh.h
	add AABB box built by 4 matrices, which can be easily to create line mesh.
	add AABB box built by 8 points, which can be easily to check whether inside the frustum

#### SH_DRAW.cpp::line:210-253
	build world AABB box from local AABB box, if one of the vertices inside the frustum planes, 
	set it as false culledOut and send to DebugRenderer to draw AABB lines
	
#### DebugRenderer.h
	add createAABBLineMesh()

#### DebugRenderer.cpp
	add createAABBLineMesh(), which only produces green lines from matrix

#### Plane.h
	add struct Plane for building 6 frustum planes, and function for checking vertex inside a plane
	
#### CameraSceneNode.h
	add m_frustumPlanes[6] for storing 6 frustum planes
	
#### CameraSceneNode.cpp::line:67-100
	build 6 frustum planes by multipling projected matrix with view matrix
	apply a narrower FOV to a second projected matrix for bounding test. objects disappear within the camera view

#### ClientCharacterControlGame.cpp
	set spawnALotOfMeshes to true to produce 100 Imrods
	

# Reference
1. [LookAt Matrix] (https://www.3dgep.com/understanding-the-view-matrix/#Look_At_Camera)
2. [Fast Extraction of Viewing Frustum Planes from the WorldView-Projection
Matrix] (https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf)

	
# Note
```
drawing lines : DefaulAnimationSM.cpp::line:361-372
drawing lines : SoldierNPCBehaviorSM.cpp::do_PRE_RENDER_needsRC()
```