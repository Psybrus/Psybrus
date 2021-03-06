/**************************************************************************
*
* File:		Rendering/ScnMaterial.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMATERIAL_H__
#define __SCNMATERIAL_H__

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"

#include "System/Content/CsResource.h"

#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Scene/Rendering/ScnShader.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/Rendering/ScnMaterialFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnMaterialRef
typedef ReObjectRef< class ScnMaterial > ScnMaterialRef;
typedef std::map< std::string, ScnMaterialRef > ScnMaterialMap;
typedef ScnMaterialMap::iterator ScnMaterialMapIterator;
typedef ScnMaterialMap::const_iterator ScnMaterialConstIterator;
typedef std::vector< ScnMaterialRef > ScnMaterialList;
typedef ScnMaterialList::iterator ScnMaterialListIterator;
typedef ScnMaterialList::const_iterator ScnMaterialListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnMaterialComponentRef
typedef ReObjectRef< class ScnMaterialComponent > ScnMaterialComponentRef;
typedef std::map< std::string, ScnMaterialComponentRef > ScnMaterialComponentMap;
typedef ScnMaterialComponentMap::iterator ScnMaterialComponentMapIterator;
typedef ScnMaterialComponentMap::const_iterator ScnMaterialComponentConstIterator;
typedef std::vector< ScnMaterialComponentRef > ScnMaterialComponentList;
typedef ScnMaterialComponentList::iterator ScnMaterialComponentListIterator;
typedef ScnMaterialComponentList::const_iterator ScnMaterialComponentListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnMaterial
class ScnMaterial:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnMaterial, CsResource );
	
	ScnMaterial();
	virtual ~ScnMaterial();
	
	void create() override;
	void destroy() override;

	/**
	 * Get texture by name.
	 */
	ScnTextureRef getTexture( BcName Name );

	/**
	 * Check if material has permutation.
	 */
	bool hasPermutation( ScnShaderPermutationFlags PermutationFlags );

	/**
	 * Get program.
	 */
	RsProgram* getProgram( ScnShaderPermutationFlags PermutationFlags );

	/**
	 * Get program binding descriptor.
	 * Will return a program binding descriptor for given permutation.
	 */
	RsProgramBindingDesc getProgramBinding( ScnShaderPermutationFlags PermutationFlags );

	/**
	 * Get render state.
	 * Will create a new render state object if it needs to.
	 * @return Render state from this material component.
	 */
	RsRenderState* getRenderState();

	/**
	 * Get all automatic uniforms.
	 */
	std::vector< BcName > getAutomaticUniforms() const;

	/**
	 * Create uniform buffer.
	 * @return Uniform buffer. If nullptr, then material doesn't have uniform buffer.
	 */
	RsBufferUPtr createUniformBuffer( const ReClass* UniformBuffer, const BcChar* DebugName ) const;
	template< typename _Ty >
	RsBufferUPtr createUniformBuffer( const BcChar* DebugName ) const
	{
		return createUniformBuffer( _Ty::StaticGetClass(), DebugName );
	}

private:
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;
	
private:
	friend class ScnMaterialComponent;
		
	ScnMaterialHeader* pHeader_;
	
	ScnShaderRef Shader_;
	ScnTextureMap TextureMap_;
	std::map< BcName, RsSamplerStateUPtr > SamplerStateMap_;
	const RsRenderStateDesc* RenderStateDesc_;
	RsRenderStateUPtr RenderState_;

	// Automatic uniforms.
	std::map< BcName, RsBufferUPtr > AutomaticUniformBuffers_;
	std::map< BcName, BcBinaryData > AutomaticUniformBlocks_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialComponent
class ScnMaterialComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnMaterialComponent, ScnComponent );
	
	ScnMaterialComponent();
	ScnMaterialComponent( ScnMaterialRef Parent, ScnShaderPermutationFlags PermutationFlags );
	ScnMaterialComponent( ScnMaterialComponent* Parent );
	virtual ~ScnMaterialComponent();

	void initialise() override;
	void destroy() override;
	
	BcU32 findTextureSlot( const BcName& TextureName );	
	void setTexture( const BcName& TextureName, ScnTextureRef Texture );
	void setTexture( BcU32 Slot, ScnTextureRef Texture );
	void setSamplerState( const BcName& TextureName, RsSamplerState* Sampler );
	void setSamplerState( BcU32 Slot, RsSamplerState* Sampler );

	BcU32 findUniformBlock( const BcName& UniformBlockName );	
	void setUniformBlock( const BcName& UniformBlockName, RsBuffer* UniformBuffer );
	void setUniformBlock( BcU32 Index, RsBuffer* UniformBuffer );

	// Common uniform blocks.
	void setViewUniformBlock( RsBuffer* UniformBuffer );
	void setBoneUniformBlock( RsBuffer* UniformBuffer );
	void setObjectUniformBlock( RsBuffer* UniformBuffer );

	/**
	 * Get program binding.
	 * Will create a new program binding if it needs to.
	 * @return Program binding that represents this material component.
	 */
	PSY_DEPRECATED( "Use the ScnRenderableComponent::createViewRenderData callback to create program bindings ahead of time." )
	RsProgramBinding* getProgramBinding();
	
	/**
	 * Get render state.
	 * Will create a new render state object if it needs to.
	 * @return Render state from this material component.
	 */
	RsRenderState* getRenderState();
	
	/**
	 * Get bound texture from slot.
	 */
	ScnTextureRef getTexture( BcU32 Slot );

	/**
	 * Get bound texture by name.
	 */
	ScnTextureRef getTexture( const BcName& TextureName );

	/**
	 * Get parent material.
	 */
	ScnMaterialRef getMaterial();
	
public:
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	friend class ScnMaterial;	

	struct TTextureBinding
	{
		REFLECTION_DECLARE_BASIC_NOAUTOREG( TTextureBinding );
		TTextureBinding(){};

		BcU32 Handle_;
		ScnTexture* Texture_;
		RsSamplerState* Sampler_;
	};
	
	struct TUniformBlockBinding
	{
		REFLECTION_DECLARE_BASIC_NOAUTOREG( TUniformBlockBinding );
		TUniformBlockBinding(){};

		BcU32 Index_;
		RsBuffer* UniformBuffer_;
	};
	
	ScnMaterialRef Material_;
	ScnShaderPermutationFlags PermutationFlags_;
	RsProgram* pProgram_;
	RsProgramBindingDesc ProgramBindingDesc_;
	RsProgramBindingUPtr ProgramBinding_;

	// Cache of textures.
	std::unordered_map< BcU32, ScnTextureRef > TextureMap_;
	
	// Common scene parameters.
	BcU32 ViewUniformBlockIndex_;
	BcU32 BoneUniformBlockIndex_;
	BcU32 ObjectUniformBlockIndex_;
};

#endif
