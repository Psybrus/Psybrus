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
// ScnModelUniforms
struct ScnModelUniforms
{
	REFLECTION_DECLARE_BASIC( ScnModelUniforms );
	ScnModelUniforms();

	std::string Name_;
	BcBinaryData Data_;
	RsBufferUPtr Buffer_;
};

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

	RsBufferUPtr VertexBuffer_;
	RsBufferUPtr IndexBuffer_;
	std::vector< RsVertexDeclarationUPtr > VertexDeclarations_;
	std::vector< RsGeometryBindingUPtr > GeometryBindings_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelComponent
class ScnModelComponent:
	public ScnRenderableComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnModelComponent, ScnRenderableComponent );

	ScnModelComponent();
	ScnModelComponent( ScnModelRef Model );
	virtual ~ScnModelComponent();

	void initialise() override;

	MaAABB getAABB() const override;

	BcU32 findNodeIndexByName( const BcName& Name ) const;
	const BcName& findNodeNameByIndex( BcU32 NodeIdx ) const;
	void setNode( BcU32 NodeIdx, const MaMat4d& LocalTransform );
	const MaMat4d& getNode( BcU32 NodeIdx ) const;
	BcU32 getNoofNodes() const;

	/**
	 * Set uniforms.
	 * @param UniformClass Uniform reflection class.
	 * @param UniformData Uniform data.
	 */
	void setUniforms( const ReClass* UniformClass, const void* UniformData );

	/**
	 * Templated version of @a setUniforms.
	 */
	template< typename _Ty >
	void setUniforms( const _Ty& UniformData )
	{
		setUniforms( _Ty::StaticGetClass(), &UniformData );
	}

#if 0
	ScnMaterialComponentRef getMaterialComponent( BcU32 Index );
	ScnMaterialComponentRef getMaterialComponent( const BcName& MaterialName );
	ScnMaterialComponentList getMaterialComponents( const BcName& MaterialName );
#endif

	void setBaseTransform( const MaVec3d& Position, const MaVec3d& Scale, const MaVec3d& Rotation );
	
public:
	static BcU32 recursiveModelUpdate( const ScnComponentList& Components, BcU32 StartIdx, BcU32 EndIdx, BcU32 MaxNodesPerJob, SysFence* Fence );
	static void updateModels( const ScnComponentList& Components );
	void updateModel( BcF32 Tick, SysFence* Fence );
	void updateNodes( MaMat4d RootMatrix );
	class ScnViewRenderData* createViewRenderData( class ScnViewComponent* View ) override;
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	void render( ScnRenderContext & RenderContext ) override;
	
protected:
	ScnModelRef Model_;
	BcU32 Layer_;

	MaVec3d Position_;
	MaVec3d Scale_;
	MaVec3d Rotation_;
	MaMat4d BaseTransform_;

	typedef std::vector< ScnModelUniforms > TMaterialUniforms;
	TMaterialUniforms Uniforms_;

	ScnModelNodeTransformData* pNodeTransformData_;
	SysFence UploadFence_;

	MaAABB AABB_;

	struct TPerComponentMeshData
	{
		RsBufferUPtr ObjectUniformBuffer_;
		RsBufferUPtr LightingUniformBuffer_;
	};
	
	typedef std::vector< TPerComponentMeshData > TPerComponentMeshDataList;	
	TPerComponentMeshDataList PerComponentMeshDataList_;
};

#endif
