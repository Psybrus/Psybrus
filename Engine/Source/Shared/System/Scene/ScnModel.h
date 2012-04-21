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

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResourceRef.h"

#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnRenderableComponent.h"

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
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
	void								recursiveSerialiseNodes( class CsPackageImporter& Importer,
										                         class BcStream& TransformStream,
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
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
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
		BcU32							MaterialName_;
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
	public ScnRenderableComponent
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, ScnModelComponent );

	virtual void						initialise( ScnModelRef Parent );
	virtual void						initialise( const Json::Value& Object );
	virtual void						destroy();
	virtual BcBool						isReady();

	void								setTransform( BcU32 NodeIdx, const BcMat4d& LocalTransform );
	
public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	void								render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	
protected:
	ScnModelRef							Parent_;
	ScnModel::TNodeTransformData*		pNodeTransformData_;

	struct TMaterialComponentDesc
	{
		ScnMaterialComponentRef MaterialComponentRef_;
	};
	
	typedef std::vector< TMaterialComponentDesc > TMaterialComponentDescList;
	
	TMaterialComponentDescList			MaterialComponentDescList_;

};

#endif
