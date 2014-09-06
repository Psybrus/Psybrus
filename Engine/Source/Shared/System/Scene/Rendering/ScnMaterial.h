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
	DECLARE_RESOURCE( ScnMaterial, CsResource );
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();

	ScnTextureRef						getTexture( BcName Name );

private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
private:
	friend class ScnMaterialComponent;
		
	ScnMaterialHeader*					pHeader_;
	
	ScnShaderRef						Shader_;
	ScnTextureMap						TextureMap_;

	// TODO: Should be handled by the state block.
	BcU32*								pStateBuffer_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialComponent
class ScnMaterialComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnMaterialComponent, ScnComponent );
	
	void								initialise( ScnMaterialRef Parent, ScnShaderPermutationFlags PermutationFlags );
	void								initialise( const Json::Value& Object );
	void								destroy();
	
	BcU32								findTextureSlot( const BcName& TextureName );	
	void								setTexture( BcU32 Slot, ScnTextureRef Texture );

	BcU32								findUniformBlock( const BcName& UniformBlockName );	
	void								setUniformBlock( BcU32 Index, RsBuffer* UniformBuffer );

	// Common uniform blocks.
	void								setViewUniformBlock( RsBuffer* UniformBuffer );
	void								setBoneUniformBlock( RsBuffer* UniformBuffer );
	void								setObjectUniformBlock( RsBuffer* UniformBuffer );


	void								setState( RsRenderStateType State, BcU32 Value );
	
	ScnTextureRef						getTexture( BcU32 Idx );
	ScnMaterialRef						getMaterial();
	
	void								bind( class RsFrame* pFrame, class RsRenderSort& Sort );

public:
	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );


private:
	friend class ScnMaterial;	

	struct TTextureBinding
	{
		REFLECTION_DECLARE_BASIC_NOAUTOREG( TTextureBinding );
		TTextureBinding(){};

		BcU32 Handle_;
		ScnTexture* Texture_;
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


	ScnMaterialRef						Parent_;
	ScnShaderPermutationFlags			PermutationFlags_;
	RsProgram*							pProgram_;

	TTextureBindingList					TextureBindingList_;
	TUniformBlockBindingList			UniformBlockBindingList_;
	
	std::vector< BcU32 >				StateBuffer_;

	// Common scene parameters.
	BcU32								ViewUniformBlockIndex_;
	BcU32								BoneUniformBlockIndex_;
	BcU32								ObjectUniformBlockIndex_;

public:
	SysFence							UpdateFence_;
};

#endif
