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

	ScnTextureRef getTexture( BcName Name );

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
	
	ScnTextureRef getTexture( BcU32 Idx );
	ScnTextureRef getTexture( const BcName& TextureName );
	ScnMaterialRef getMaterial();
	
	void bind( class RsFrame* pFrame, class RsRenderSort& Sort );

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

	typedef std::vector< TTextureBinding > TTextureBindingList;
	typedef TTextureBindingList::iterator TTextureBindingListIterator;

	typedef std::vector< TUniformBlockBinding > TUniformBlockBindingList;
	typedef TUniformBlockBindingList::iterator TUniformBlockBindingListIterator;


	ScnMaterialRef Material_;
	ScnShaderPermutationFlags PermutationFlags_;
	RsProgram* pProgram_;

	TTextureBindingList TextureBindingList_;
	TUniformBlockBindingList UniformBlockBindingList_;
	
	// Common scene parameters.
	BcU32 ViewUniformBlockIndex_;
	BcU32 BoneUniformBlockIndex_;
	BcU32 ObjectUniformBlockIndex_;
};

#endif
