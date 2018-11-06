> version : 1.2
> author : TsuHao Wang


# Demo Video
[youtube link] (https://youtu.be/yr5fRU57Mxc)


# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- milestone1 level.


# General Description
- add friction coefficient to each static mesh PhysicsComponent
- set camera to higher place that can watch the cars clearly


# Revised Files

--- important ---

#### PrimeEngine/GameObjectModel/GameObjectManager.cpp::334
	set frictionCoef from Event_CREATE_MESH to PhysicsComponent
	
#### PrimeEngine/Scene/CameraManager.cpp::23
	set default camera pos to a higher place

	
--- trivial ---

#### PrimeEngine/Events/StandardEvents.h
	 PrimeEngine/Events/StandardEvents.cpp
	add frictionCoef to struct Event_CREATE_MESH

#### AssetsOut\Default\GameObjectMetaScripts\SMBrokenPlaneMeta.py
	 AssetsOut\Default\GameObjectMetaScripts\SMCobblePlane.py
	 AssetsOut\Default\GameObjectMetaScripts\SMImrod.py
	 AssetsOut\Default\GameObjectMetaScripts\SMKingTigerMeta.py
	 AssetsOut\Default\GameObjectMetaScripts\SMNaziCarMeta.py
	 AssetsOut\City\GameObjectMetaScripts\SMBrickSpire.py
	 AssetsOut\City\GameObjectMetaScripts\SMStreetLight.py
	 AssetsOut\Default\Scripts\StaticMesh.lua
	add friction coefficient to lua and default value in py
	 

# Todo
- change velocity in simulation
- change PhysicsComponent to factory mode


