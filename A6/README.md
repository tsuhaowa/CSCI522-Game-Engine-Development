# Demo Video
[youtube link](https://youtu.be/kgz0SkrrJTU)


# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- ccontrollvl0.x level for debugging.


# General Description
- add 2nd uv which is equal to mesh's y value
- pass 2nd uv to Vertex Shader and Pixel Shader
- add 3 wind source, one from camera pos, two from soldier pos
- set wind sources followed by gGameTimes in same file
- can adjust wind proportion in Vertex Shader


# Revised Files

#### PrimeEngine\APIAbstraction\GPUBuffers\VertexBufferGPU.cpp
	add 2nd uv in createGPUBufferFromSource_DetailedMesh, the u2 and v2 are both equal to y
	
#### PrimeEngine\APIAbstraction\GPUBuffers\VertexBufferGPUManager.cpp
	add 2nd uv in layout, set the order to 7 to match TEXCOORD7
	
#### PrimeEngine\APIAbstraction\GPUBuffers\BufferInfo.cpp
	add one if statement in elementSemanticToApiSemantic() to match TEXCOORD7
	
#### AssetsOut\Default\GPUPrograms\DetailedMesh_Structs.fx
	add 2nd uv in DETAILED_MESH_VS_IN and DETAILED_MESH_SHADOWED_PS_IN
	the 2nd uv uses TEXCOORD7
	
#### AssetsOut\Default\GPUPrograms\DetailedMesh_Shadowed_VS.cgvs
	assign vIn.iTexCoord2 to vOut.iTexCoord2

#### AssetsOut\Default\GPUPrograms\DetailedMesh_Shadowed_A_Glow_PS.cgps
	return color based on uv2, and do not revise soldier's skin
	
#### AssetsOut\Default\GPUPrograms\DetailedSkin_Shadowed_VS.cgvs
	the output is DETAILED_MESH_SHADOWED_PS_IN, must give a value to iTexCoord2 to initialize
	set vOut.iTexCoord2 = float2(-1.0f, -1.0f) to prevent changing soldier's skin
	
#### PrimeEngine\APIAbstraction\GPUBuffers\BufferInfo.h
	change the binding name

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