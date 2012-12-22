/**************************************************************************
*
* File:		ScnShaderFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSHADERFILEDATA__
#define __SCNSHADERFILEDATA__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationFlags
enum ScnShaderPermutationFlags
{
	// Exclusive base implementations.
	scnSPF_2D						= 0x00000001,		// 2D.
	scnSPF_3D						= 0x00000002,		// 3D.
	scnSPF_PARTICLE_3D				= 0x00000004,		// Particle 3D.
	scnSPF_EXCLUSIVE_BASE = scnSPF_2D | scnSPF_3D | scnSPF_PARTICLE_3D,
	
	// Vertex processing.
	scnSPF_STATIC					= 0x00000010,		// Static mesh.
	scnSPF_SKINNED					= 0x00000020,		// Skinned mesh.
	scnSPF_MORPH					= 0x00000040,		// Morph mesh.
	
	scnSPF_EXCLUSIVE_VERTEX = scnSPF_STATIC | scnSPF_SKINNED | scnSPF_MORPH
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationBootstrap
struct ScnShaderPermutationBootstrap
{
	BcU32							PermutationFlags_;
	const BcChar*					SourceGeometryShaderName_;
	const BcChar*					SourceVertexShaderName_;
	const BcChar*					SourceFragmentShaderName_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderHeader
struct ScnShaderHeader
{
	BcU32							NoofVertexShaderPermutations_;
	BcU32							NoofFragmentShaderPermutations_;
	BcU32							NoofProgramPermutations_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderUnitHeader
struct ScnShaderUnitHeader
{
	BcU32							PermutationFlags_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderProgramHeader
struct ScnShaderProgramHeader
{
	BcU32							ProgramPermutationFlags_;
	BcU32							VertexShaderPermutationFlags_;
	BcU32							FragmentShaderPermutationFlags_;
};

#endif
