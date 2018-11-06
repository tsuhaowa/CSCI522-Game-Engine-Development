# Environment
- WIN10
- win32d3d9
- Visual Studio 2015
- Maya 2017
- ccontrollvl0.x level for debugging.


# General Description
- part1
- add 2nd uv which is equal to mesh's y value


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

