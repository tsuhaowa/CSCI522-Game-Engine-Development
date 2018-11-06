# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- ccontrollvl0.x level for debugging.


# General Description
- part2
- add 3 wind source, one from camera pos, two from soldier pos
- set wind sources with gGameTimes in same file
- can adjust wind proportion in Vertex Shader


# Revised Files

#### AssetsOut\Default\GPUPrograms\DetailedMesh_Shadowed_VS.cgvs
	use 3 wind sources to perform proportional revision to mesh

#### AssetsOut\Default\GPUPrograms\StandardConstants.fx
	add 3 wind sources, using register 163-165
	update them with gGameTimes

#### PrimeEngine\Render\ShaderActions\SetPerFrameConstantsShaderAction.h
	 PrimeEngine\Render\ShaderActions\SetPerFrameConstantsShaderAction.cpp
	set three wind sources and pass to register
	
#### CharacterControl\Characters\SoldierNPC.cpp
	add SoldierNPC to RootSceneNode