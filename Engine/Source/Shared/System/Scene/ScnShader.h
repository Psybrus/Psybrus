/**************************************************************************
*
* File:		ScnShader.h
* Author:	Neil Richardson 
* Ver/Date:	11/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSHADER_H__
#define __SCNSHADER_H__

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnTexture.h"

//////////////////////////////////////////////////////////////////////////
// eScnShaderPermutationFlags
enum eScnShaderPermutationFlags
{
	scnSPF_DEFAULT					= 0x00000000,		// Default minimal shader.
	scnSPF_WORLD_TRANSFORM			= 0x00000001,		// Transforms vertices into clip space.
	scnSPF_CLIP_SPACE_TRANSFORM		= 0x00000002,		// Transform vertices into world space.
};

//////////////////////////////////////////////////////////////////////////
// ScnShader
typedef CsResourceRef< class ScnShader > ScnShaderRef;

//////////////////////////////////////////////////////////////////////////
// ScnShaderProgram
typedef CsResourceRef< class ScnShaderProgram > ScnShaderProgramRef;

//////////////////////////////////////////////////////////////////////////
// ScnShader
class ScnShader:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnShader );
	
	virtual BcBool						import( const Json::Value& Object );
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	RsShader*							getShader( BcU32 PermutationFlags );
		
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	struct THeader
	{
		eRsShaderType					Type_;
		BcU32							NoofPermutations_;
	};
	
	struct TShaderHeader
	{
		BcU32							PermutationFlags_;
	};
	
	typedef std::map< BcU32, RsShader* > TShaderMap;
	typedef TShaderMap::iterator TShaderMapIterator;
	
	THeader*							pHeader_;
	TShaderMap							ShaderMap_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderProgram
class ScnShaderProgram:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnShaderProgram );
	
	void								initialise( ScnShaderRef VertexShader, ScnShaderRef FragmentShader );
	virtual void						create();
	virtual void						destroy();
	
	virtual BcBool						isReady();
	
private:
	friend class ScnShader;

private:
	BcU32								PermutatationFlags_;
	RsProgram*							pProgram_;
	ScnShaderRef						VertexShader_;
	ScnShaderRef						FragmentShader_;
};



#endif
