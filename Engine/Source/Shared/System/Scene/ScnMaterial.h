/**************************************************************************
*
* File:		ScnMaterial.h
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

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnTexture.h"
#include "ScnShader.h"

//////////////////////////////////////////////////////////////////////////
// ScnMaterialRef
typedef CsResourceRef< class ScnMaterial > ScnMaterialRef;

//////////////////////////////////////////////////////////////////////////
// ScnMaterialRef
typedef CsResourceRef< class ScnMaterialInstance > ScnMaterialInstanceRef;

//////////////////////////////////////////////////////////////////////////
// ScnMaterial
class ScnMaterial:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnMaterial );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	BcBool								createInstance( const std::string& Name, ScnMaterialInstanceRef& Instance, BcU32 PermutationFlags );
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
private:
	friend class ScnMaterialInstance;
	
	struct THeader
	{
		// TODO: Store IDs, hash keys, or more packed data.
		BcChar							ShaderName_[ 64 ];
		BcU32							NoofTextures_;
	};

	struct TTextureHeader
	{
		BcChar							SamplerName_[ 64 ];
		BcChar							TextureName_[ 64 ];
	};
	
	THeader*							pHeader_;
	
	ScnShaderRef						Shader_;
	ScnTextureMap						TextureMap_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialInstance
class ScnMaterialInstance:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnMaterialInstance );
	
	void								initialise( ScnMaterialRef Parent, RsProgram* pProgram, const ScnTextureMap& TextureMap );
	void								destroy();
	
	BcU32								findParameter( const std::string& ParameterName );	
	void								setParameter( BcU32 Parameter, BcS32 Value );
	void								setParameter( BcU32 Parameter, BcReal Value );
	void								setParameter( BcU32 Parameter, const BcVec2d& Value );
	void								setParameter( BcU32 Parameter, const BcVec3d& Value );
	void								setParameter( BcU32 Parameter, const BcVec4d& Value );
	void								setParameter( BcU32 Parameter, const BcMat3d& Value );
	void								setParameter( BcU32 Parameter, const BcMat4d& Value );
	void								setTexture( BcU32 Parameter, ScnTextureRef Texture );
	
	void								bind( RsFrame* pFrame, RsRenderSort Sort );
	

	virtual BcBool						isReady();

private:
	friend class ScnMaterial;
		
	struct TParameterBinding
	{
		eRsShaderParameterType			Type_;
		BcU32							Offset_;
	};
	
	typedef std::vector< TParameterBinding > TParameterBindingList;
	typedef TParameterBindingList::iterator TParameterBindingListIterator;
	
	struct TTextureBinding
	{
		BcU32							Parameter_;
		ScnTextureRef					Texture_;
	};
	
	typedef std::vector< TTextureBinding > TTextureBindingList;
	typedef TTextureBindingList::iterator TTextureBindingListIterator;
	
	ScnMaterialRef						Parent_;
	RsProgram*							pProgram_;

	TParameterBindingList				ParameterBindingList_;
	TTextureBindingList					TextureBindingList_;

	BcU32								ParameterBufferSize_;
	BcU8*								pParameterBuffer_;
};

#endif
