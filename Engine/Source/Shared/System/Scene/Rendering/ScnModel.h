/**************************************************************************
*
* File:		Rendering/ScnModel.h
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

#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"

#include "System/Scene/Rendering/ScnModelFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelRef
typedef ReObjectRef< class ScnModel > ScnModelRef;

//////////////////////////////////////////////////////////////////////////
// ScnModelComponentRef
typedef ReObjectRef< class ScnModelComponent > ScnModelComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnModel
class ScnModel:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnModel, CsResource );
		
	ScnModel();
	virtual ~ScnModel();

	void create() override;
	void destroy() override;
	
private:
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;
	
protected:
	friend class ScnModelComponent;
	
	// Cached pointers for internal use.
	ScnModelHeader* pHeader_;
	ScnModelNodeTransformData* pNodeTransformData_;
	ScnModelNodePropertyData* pNodePropertyData_;
	BcU8* pVertexBufferData_;
	BcU8* pIndexBufferData_;
	RsVertexElement* pVertexElements_;
	ScnModelMeshData* pMeshData_;
	
	ScnModelMeshRuntimeList MeshRuntimes_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelComponent
class ScnModelComponent:
	public ScnRenderableComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnModelComponent, ScnRenderableComponent );

	ScnModelComponent();
	virtual ~ScnModelComponent();

	void initialise() override;

	MaAABB getAABB() const override;

	BcU32 findNodeIndexByName( const BcName& Name ) const;
	const BcName& findNodeNameByIndex( BcU32 NodeIdx ) const;
	void setNode( BcU32 NodeIdx, const MaMat4d& LocalTransform );
	const MaMat4d& getNode( BcU32 NodeIdx ) const;
	BcU32 getNoofNodes() const;

	ScnMaterialComponentRef getMaterialComponent( BcU32 Index );
	ScnMaterialComponentRef getMaterialComponent( const BcName& MaterialName );
	ScnMaterialComponentList getMaterialComponents( const BcName& MaterialName );

	void setBaseTransform( const MaVec3d& Position, const MaVec3d& Scale, const MaVec3d& Rotation );
	
public:
	static void updateModels( const ScnComponentList& Components );
	void updateModel( BcF32 Tick );
	void updateNodes( MaMat4d RootMatrix );
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	void render( ScnRenderContext & RenderContext ) override;
	
protected:
	ScnModelRef Model_;
	BcU32 Layer_;
	BcU32 Pass_;

	MaVec3d Position_;
	MaVec3d Scale_;
	MaVec3d Rotation_;
	MaMat4d BaseTransform_;

	ScnModelNodeTransformData* pNodeTransformData_;
	SysFence UploadFence_;
	SysFence UpdateFence_;
	SysFence RenderFence_;

	MaAABB AABB_;

	struct TPerComponentMeshData
	{
		ScnMaterialComponentRef MaterialComponentRef_;
		RsBuffer* UniformBuffer_;
	};
	
	typedef std::vector< TPerComponentMeshData > TPerComponentMeshDataList;
	
	TPerComponentMeshDataList PerComponentMeshDataList_;
};

#endif
