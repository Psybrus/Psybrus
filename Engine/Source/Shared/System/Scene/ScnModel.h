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
#include "ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelRef
typedef CsResourceRef< class ScnModel > ScnModelRef;

//////////////////////////////////////////////////////////////////////////
// ScnModelComponentRef
typedef CsResourceRef< class ScnModelComponent > ScnModelComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnModel
class ScnModel:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnModel );
	
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
	
	BcBool								createComponent( const BcName& Name, ScnModelComponentRef& Handle );
	
private:
	void								setup();
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:
	friend class ScnModelComponent;
	
	// Header.
	struct THeader
	{
		BcU32							NoofNodes_;
		BcU32							NoofPrimitives_;
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
		BcChar							MaterialName_[ 64 ];		// NOTE: Not optimal...look at packing into a string table.
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
		BcU32							PrimitiveDataIndex_;
		RsVertexBuffer*					pVertexBuffer_;
		RsIndexBuffer*					pIndexBuffer_;
		RsPrimitive*					pPrimitive_;
		ScnMaterialRef					MaterialRef_;
	};
	
	typedef std::vector< TPrimitiveRuntime > TPrimitiveRuntimeList;
	TPrimitiveRuntimeList				PrimitiveRuntimes_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelComponent
class ScnModelComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnModelComponent );

	virtual void						initialise( ScnModelRef Parent );
	virtual void						destroy();
	virtual BcBool						isReady();

	void								setTransform( BcU32 NodeIdx, const BcMat4d& LocalTransform );
	
public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	void								render( RsFrame* pFrame, RsRenderSort Sort );
	
protected:
	ScnModelRef							Parent_;
	ScnModel::TNodeTransformData*		pNodeTransformData_;

	struct TMaterialComponentDesc
	{
		ScnMaterialComponentRef MaterialComponentRef_;
		BcU32 WorldMatrixIdx_;
	};
	
	typedef std::vector< TMaterialComponentDesc > TMaterialComponentDescList;
	
	TMaterialComponentDescList			MaterialComponentDescList_;
};

#endif
