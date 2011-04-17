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
	DECLARE_RESOURCE( ScnMaterial );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	BcBool								createInstance( const std::string& Name, ScnMaterialInstanceRef& Instance, BcU32 PermutationFlags );
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
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
	DECLARE_RESOURCE( ScnMaterialInstance );
	
	void								initialise( ScnMaterialRef Parent, RsProgram* pProgram, const ScnTextureMap& TextureMap );
	
	void								bind();

	virtual BcBool						isReady();

private:
	friend class ScnMaterial;
		
	struct TTextureBinding
	{
		RsProgramParameter*				pParameter_;
		ScnTextureRef					Texture_;
	};
	
	typedef std::list< TTextureBinding > TTextureBindingList;
	typedef TTextureBindingList::iterator TTextureBindingListIterator;
	
	ScnMaterialRef						Parent_;
	RsProgram*							pProgram_;

	TTextureBindingList					TextureBindingList_;
};

#endif
