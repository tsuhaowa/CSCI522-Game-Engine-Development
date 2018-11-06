# Demo Video
[youtube link](https://www.youtube.com/watch?v=wT1wRR34Pyk)

# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- A4 level for debugging.


# General Description
- Update the bounding box and checked soldier collision toward each MeshInstance.
- Do collision check in PhysicsManager.cpp.
- Apply Separating Axis Theorem(SAT) and Line Intersect Check for non-AABB collision detection.
- Acquire intersect plane by comparison between the distance from center to the target plane and 
									  the projected distance from center to farest vertex onto the normal vector of target plane.


# Features
- Build Skeleton non-AABB by joints
- Add physics attribute and manager to each SkeletonInstance and MeshInstance
- Implement non-AABB collision detection
- Add function to get collision plane
- Add acceleration of gravity for falling.
- Make stuck solution


# Revised Files

#### /CharacterControl/Characters/SoldierNPC.h
	include PhysicsManager.h

#### /CharacterControl/Characters/SoldierMovementSM.cpp::101-201
	add a lot of judgements here, including deciding the direction after collision,
	also falling action is set here.
	make soldier always facing to the target waypoint.
	do different procedures in two frames(f):
		f=1, collision detection true -> falling or move m_moveBackWard, which is an addition of collision plane's normal and dodge vector.
					false -> move to target
		f=2, if no collision -> move m_moveAfterBackWard, which normally is a perpendicular vector of collision plane's normal,
									one of the two perpendicular vectors that makes soldier closer to target.
		     still collision -> keep moving m_moveBackWard

#### /CharacterControl/PhysicsManager.h
	A Component.
	include infos about physics data
	put bounding box to PhysicsManager

#### /CharacterControl/PhysicsManager.cpp
	collisionDetectionAll() : do collision check to all MeshInstance, add the collision MeshInstance to m_collisionMeshInstance,
				set current standing plane to m_curStandPlane, set MeshInstance collision count to m_collisionCount
	checkCollisionPhysicsManager() : implement SAT algorithm for non-AABB collision check
	checkSegmentIntersect() : check whether two line segments is intersect
	addCollisionPlane() : if a plane is collision, but its corresponding plane does not collide, consider it as a effective collision plane.
	checkCollisionPlane() : check collision by comparison two distances, see details in References(1).
	checkStuck() : Not used.  check whether a center is inside a box.
	buildBoundingVolume() : initialize model space bounding box's 8 vertices, and 4 vertices with vectors. The latter is for quickly drawing box. 
	buildBoundingVolumeAfterTransform() : set bounding box after transformed attributes that are in real world.
	setBoundingBoxCenterAndHalfLength() : set bounding box cetner. The half length is not used.
	setExtremeAxisPos() : Not used. set the min and max x,y,z of AABB box, but I'm using non-AABB box.
	buildCollisionSkipList() : build skip list for gun Mesh and SoldierTransform Mesh, that should not consider collision.
	
#### /CharacterControl/ClientCharacterControlGame.cpp
	take way the TigerTank and Imrod to have a better debugging process
	
#### /CharacterControl/GlobalRegistry.cpp
	register PhysicsManager component	
	
#### /PrimeEngine/Scene/Mesh.h
	include PhysicsManager and add m_physicsManager
	add m_meshFileName
	
#### /PrimeEngine/Scene/MeshManager.cpp
	create bounding box for each Mesh, add a PhysicsManager to Mesh
	
#### /PrimeEngine/Scene/SH_DRAW.cpp
	change the variables in drawing bounding box
	update real world bounding box for each PhysicsManager of MeshInstance
	
#### /PrimeEngine/Scene/DefaultAnimationSM.cpp::362-401
	update and draw the bounding box of soldiers' joints
	
#### /PrimeEngine/Scene/MeshInstance.h
#### /PrimeEngine/Scene/MeshInstance.cpp
	add PhysicsManager Component to each MeshInstance, get information from the return handle of Mesh
	
#### /PrimeEngine/Scene/SkeletonInstance.h
#### /PrimeEngine/Scene/SkeletonInstance.cpp
	add PhysicsManager Component for each SkeletonInstance
	add m_meshFileName

#### /PrimeEngine/Scene/CameraSceneNode.cpp
	take away the narrow bounding demo
		
#### /PrimeEngine/Math/Plane.h
	add buildPlaneByPoints() and getN(),
	add func to check whether two Plane intersect by checking normal vector


# References
1. [Acquire Intersect Plane](https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html)
2. [Separating Axis Theorem(SAT)](https://gamedevelopment.tutsplus.com/tutorials/collision-detection-using-the-separating-axis-theorem--gamedev-169)
3. [Hyperplane separation theorem](https://en.wikipedia.org/wiki/Hyperplane_separation_theorem)

# Note
```
In SoldierNPC, can get a hierachy:
RootSceneNode::Instance()->addComponent(hSN)
pMainScene->addComponent(hSN)  // the innder hSN
pSN->addComponent(hRotateSN)
pRotateSN->addComponent(hSkeletonInstance)
pSkelInst->addComponent(hMeshInstance)

and
(pSkelInst->addComponent(PhysicsManager) in SkeletonInstance.cpp)

Furthermore:
SoldierNPC.addComponent(SoldierMovementSM);
SoldierNPC.addComponent(SoldierBehaviorSM);
```
