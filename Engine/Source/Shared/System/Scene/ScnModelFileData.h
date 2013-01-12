/**************************************************************************
*
* File:		ScnModelFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMODELFILEDATA__
#define __SCNMODELFILEDATA__

#include "System/Renderer/RsTypes.h"
#include "System/Scene/ScnMaterial.h" // TODO: Get rid of this reference.

//////////////////////////////////////////////////////////////////////////
// Defines
#define SCN_MODEL_BONE_PALETTE_SIZE		( 24 )

//////////////////////////////////////////////////////////////////////////
// ScnModelHeader
struct ScnModelHeader
{
	BcU32							NoofNodes_;
	BcU32							NoofPrimitives_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnModelNodeTransformData
struct ScnModelNodeTransformData
{
	BcMat4d							RelativeTransform_;
	BcMat4d							AbsoluteTransform_;
	BcMat4d							InverseBindpose_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnModelNodePropertyData
struct ScnModelNodePropertyData
{
	BcU32							ParentIndex_;
	BcName							Name_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnModelPrimitiveData
struct ScnModelPrimitiveData
{
	BcU32							NodeIndex_;
	eRsPrimitiveType				Type_;
	BcU32							VertexFormat_;
	BcU32							ShaderPermutation_;
	BcU32							NoofVertices_;
	BcU32							NoofIndices_;
	BcU32							MaterialRef_;
	BcU32							Padding0_;
	BcU32							Padding1_;
	BcAABB							AABB_;
	BcU32							BonePalette_[ SCN_MODEL_BONE_PALETTE_SIZE ];
};

//////////////////////////////////////////////////////////////////////////
// ScnModelPrimitiveRuntime
struct ScnModelPrimitiveRuntime
{
	BcU32							PrimitiveDataIndex_;
	class RsVertexBuffer*			pVertexBuffer_;
	class RsIndexBuffer*			pIndexBuffer_;
	class RsPrimitive*				pPrimitive_;
	ScnMaterialRef					MaterialRef_; // TODO: Don't use ref, just use pointer.
};

//////////////////////////////////////////////////////////////////////////
// ScnModelPrimitiveRuntimeList: TODO: Just change to a static array in code.
typedef std::vector< ScnModelPrimitiveRuntime > ScnModelPrimitiveRuntimeList;


#endif
