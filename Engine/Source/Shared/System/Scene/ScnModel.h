/**************************************************************************
*
* File:		ScnModel.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMODEL_H__
#define __SCNMODEL_H__

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// ScnMeshRef
typedef CsResourceRef< class ScnModel > ScnModelRef;

//////////////////////////////////////////////////////////////////////////
// ScnModel
class ScnModel:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnModel );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
	void								recursiveSerialiseNodes( class BcStream& TransformStream,
																 class BcStream& PropertyStream,
																 class BcStream& VertexStream,
																 class BcStream& IndexStream,
																 class BcStream& PrimitiveStream,
																 class MdlNode* pNode,
																 BcU32 ParentIndex,
																 BcU32& NodeIndex,
																 BcU32& PrimitiveIndex );
#endif
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
private:
	void								setup();
	void								updateNodes();
	void								renderPrimitives();
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	// Header.
	struct THeader
	{
		BcU32 NoofNodes_;
		BcU32 NoofPrimitives_;
	};
	
	// Node transform data.
	struct TNodeTransformData
	{
		BcMat4d							RelativeTransform_;
		BcMat4d							AbsoluteTransform_;
		BcMat4d							InverseBindpose_;
	};
	
	// Node property data.
	struct TNodePropertyData
	{
		BcU32							ParentIndex_;
	};
	
	// Primitive data.
	struct TPrimitiveData
	{
		BcU32							NodeIndex_;
		eRsPrimitiveType				Type_;
		BcU32							VertexFormat_;
		BcU32							NoofVertices_;
		BcU32							NoofIndices_;
		BcChar							MaterialName_[ 256 ];		// NOTE: Not optimal...look at packing into a string table.
	};
	
	// Cached pointers for internal use.
	THeader*							pHeader_;
	TNodeTransformData*					pNodeTransformData_;
	TNodePropertyData*					pNodePropertyData_;
	BcU8*								pVertexBufferData_;
	BcU8*								pIndexBufferData_;
	TPrimitiveData*						pPrimitiveData_;
	
	// Runtime structures.
	struct TPrimitiveRuntime
	{
		TNodeTransformData*				pNodeTransformData_;
		TPrimitiveData*					pPrimitiveData_;
		RsVertexBuffer*					pVertexBuffer_;
		RsIndexBuffer*					pIndexBuffer_;
		RsPrimitive*					pPrimitive_;
		ScnMaterialRef					MaterialRef_;
	};
	
	std::vector< TPrimitiveRuntime >	PrimitiveRuntimes_;
};

#endif
