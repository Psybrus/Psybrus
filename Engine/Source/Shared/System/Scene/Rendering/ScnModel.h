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

	const ReClass* Class_;
	BcBinaryData Data_;
	RsBufferUPtr Buffer_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelProcessor
class ScnModelProcessor : 
	public BcGlobal< ScnModelProcessor >,
	public ScnComponentProcessor,
	public ScnViewRenderInterface
{
public:
	ScnModelProcessor();
	virtual ~ScnModelProcessor();

	/// ScnComponentProcessor
	void initialise() override;
	void shutdown() override;

	/// ScnViewRenderInterface
	class ScnViewRenderData* createViewRenderData( class ScnComponent* Component, class ScnViewComponent* View ) override;
	void destroyViewRenderData( class ScnComponent* Component, ScnViewRenderData* ViewRenderData ) override;
	void render( const ScnViewComponentRenderData* ComponentRenderDatas, BcU32 NoofComponents, class ScnRenderContext & RenderContext ) override;
	void getAABB( MaAABB* OutAABBs, class ScnComponent** Components, BcU32 NoofComponents ) override;
	void getRenderMask( BcU32* OutRenderMasks, class ScnComponent** Components, BcU32 NoofComponents ) override;

	///
	size_t recursiveModelUpdate( const ScnComponentList& Components, size_t StartIdx, size_t EndIdx, size_t MaxNodesPerJob, SysFence* Fence );
	void updateModels( const ScnComponentList& Components );

private:
	std::vector< ScnViewComponentRenderData > ComponentRenderDatas_;

	struct InstancingData
	{
		BcU32 RefCount_ = 0;
		std::unordered_map< const ReClass*, RsBuffer* > UniformBuffers_;
		std::vector< RsProgramBinding* > ProgramBindings_; 
	};

	std::unordered_map< ScnModel*, InstancingData > InstancingData_;

	
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
	friend class ScnModelProcessor;
	
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
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnModelComponent, ScnComponent );

	ScnModelComponent();
	ScnModelComponent( ScnModelRef Model );
	virtual ~ScnModelComponent();

	void initialise() override;

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

	/**
	 * Is this model component lit?
	 */
	bool isLit() const { return IsLit_; }

	/**
	 * Set if model is lit.
	 */
	void setLit( bool Lit ) { IsLit_ = Lit; }

	void setBaseTransform( const MaVec3d& Position, const MaVec3d& Scale, const MaVec3d& Rotation );
	
public:
	void updateModel( BcF32 Tick, SysFence* Fence );
	void updateNodes( const MaMat4d& RootMatrix );
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	class ScnViewRenderData* createViewRenderData( class ScnViewComponent* View );
	void render( ScnRenderContext & RenderContext );
	MaAABB getAABB() const;

protected:
	friend class ScnModelProcessor;

	ScnModelRef Model_;
	BcU32 Layer_;

	MaVec3d Position_;
	MaVec3d Scale_;
	MaVec3d Rotation_;
	MaMat4d BaseTransform_;

	/// Used to specify what kind of object it is for selectively rendering with certain views.
	BcU32 RenderMask_;
	/// Does this need to be lit?
	bool IsLit_;
	/// Render permutation flags that this renderable supports.
	ScnShaderPermutationFlags RenderPermutations_;
	/// Sort pass flags that this renderable supports.
	RsRenderSortPassFlags Passes_;

	typedef std::vector< ScnModelUniforms > TMaterialUniforms;
	TMaterialUniforms Uniforms_;

	ScnModelNodeTransformData* pNodeTransformData_;
	SysFence UploadFence_;

	MaAABB AABB_;
	struct TPerComponentMeshData
	{
		RsBufferUPtr ObjectUniformBuffer_;
		RsBufferUPtr LightingUniformBuffer_;
		MaAABB AABB_;
	};
	
	typedef std::vector< TPerComponentMeshData > TPerComponentMeshDataList;	
	TPerComponentMeshDataList PerComponentMeshDataList_;
};

#endif
