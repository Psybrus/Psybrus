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
	
	virtual BcBool						import( const Json::Value& Object );
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	BcBool								createInstance( const std::string& Name, ScnMaterialInstanceRef& Instance, BcU32 PermutationFlags );
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	
	// External resources.
	typedef std::map< BcU32, RsProgram* > TProgramMap;
	typedef TProgramMap::iterator TProgramMapIterator;
	typedef std::vector< ScnTextureRef > TTextureList;
	typedef TTextureList::iterator TTextureListIterator;
	
	ScnShaderRef						VertexShader_;
	ScnShaderRef						FragmentShader_;
	TProgramMap							ProgramMap_;
	TTextureList						TextureList_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialInstance
class ScnMaterialInstance:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnMaterialInstance );

	virtual BcBool						isReady();

private:
	friend class ScnMaterial;

	RsProgram*							pProgram_;
	ScnMaterialRef						Parent_;
};

#endif
