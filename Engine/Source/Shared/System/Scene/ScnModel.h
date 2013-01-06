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

#include "System/Scene/ScnModelFileData.h"

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
#endif
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	friend class ScnModelComponent;
	
	// Cached pointers for internal use.
	ScnModelHeader*						pHeader_;
	ScnModelNodeTransformData*			pNodeTransformData_;
	ScnModelNodePropertyData*			pNodePropertyData_;
	BcU8*								pVertexBufferData_;
	BcU8*								pIndexBufferData_;
	ScnModelPrimitiveData*				pPrimitiveData_;
	
	ScnModelPrimitiveRuntimeList		PrimitiveRuntimes_;
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

	virtual BcAABB						getAABB() const;

	BcU32								findNodeIndexByName( const BcName& Name ) const;
	void								setNode( BcU32 NodeIdx, const BcMat4d& LocalTransform );
	const BcMat4d&						getNode( BcU32 NodeIdx ) const;
	BcU32								getNoofNodes() const;

	ScnMaterialComponentRef				getMaterialComponent( BcU32 Index );
	ScnMaterialComponentRef				getMaterialComponent( const BcName& MaterialName );
	
public:
	virtual void						postUpdate( BcF32 Tick );

	void								updateNodes( BcMat4d RootMatrix );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	void								render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	
protected:
	ScnModelRef							Parent_;
	ScnModelNodeTransformData*			pNodeTransformData_;
	SysFence							UpdateFence_;

	BcU32								Layer_;

	BcAABB								AABB_;

	struct TMaterialComponentDesc
	{
		ScnMaterialComponentRef MaterialComponentRef_;
	};
	
	typedef std::vector< TMaterialComponentDesc > TMaterialComponentDescList;
	
	TMaterialComponentDescList			MaterialComponentDescList_;

};

#endif
