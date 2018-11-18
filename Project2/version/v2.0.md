> version : 2.0

> author : TsuHao Wang


# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- milestone2 level.


# General Description
- add cameraSwitch to switch currently active sceneNode(skycamera, car1, car2)
- contrl nazicar by up, down, left, right with some physics factors
- we have horsepower, torque, floor friction, mass, acceleration
- remove original code for nazicar script to simplify


# Revised Files

--- important ---

#### CharacterControl\NaziCarMovementSM.cpp
	 CharacterControl\NaziCarMovementSM.h
	use physics factors to move the nazicar
	move nazicar forward, backward, left, and right
	
#### PrimeEngine\GameObjectModel\DefaultGameControls\DefaultGameControls.cpp
	let F1,F2,F3 switch currently active sceneNode
	let up,down,left,right set force to currently active sceneNode's physicsCompoent
	
#### PrimeEngine\PhysicsEngine\PhysicsComponent.cpp
	 PrimeEngine\PhysicsEngine\PhysicsComponent.h
	build and calculate nazicar's four directions from pos
	
#### PrimeEngine\PhysicsEngine\PhysicsManager.cpp
	update pre_physics to correct the transform matrix for bounding box
	update object whose id is not -1 per frame even not move, like car1, car2

#### PrimeEngine\Scene\CameraManager.cpp
	 PrimeEngine\Scene\CameraManager.h
	add static cameraSwitch to store 3 sceneNode : skycamera, car1, car2
	add static cameraIndex to store currently active sceneNode

#### mayafiles
	add torque to nazicar object
	adjust some arguments

--- trivial ---

#### CharacterControl\NaziCar.cpp
	 CharacterControl\NaziCar.h
	read torque

#### CharacterControl\ClientCharacterControlGame.cpp
	adjust floor friction coefficient
	
#### CharacterControl\GlobalRegistry.cpp
	remove NaziCarBehaviorSM and some events


# Todo
- simulate "down" to brake force
- simulate "shift" to shift
- switch camera view to car



