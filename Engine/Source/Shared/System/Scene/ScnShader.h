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
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	RsProgram*							getProgram( BcU32 PermutationFlags );
		
private:
	typedef std::map< BcU32, RsShader* > TShaderMap;
	typedef std::map< BcU32, RsProgram* > TProgramMap;
	typedef TShaderMap::iterator TShaderMapIterator;
	typedef TProgramMap::iterator TProgramMapIterator;

	RsShader*							getShader( BcU32 PermutationFlags, TShaderMap& ShaderMap );
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	struct THeader
	{
		BcU32							NoofVertexShaderPermutations_;
		BcU32							NoofFragmentShaderPermutations_;
		BcU32							NoofProgramPermutations_;
	};
	
	struct TShaderHeader
	{
		BcU32							PermutationFlags_;
	};
	
	struct TProgramHeader
	{
		BcU32							ProgramPermutationFlags_;
		BcU32							VertexShaderPermutationFlags_;
		BcU32							FragmentShaderPermutationFlags_;
	};
		
	THeader*							pHeader_;
	TShaderMap							VertexShaderMap_;
	TShaderMap							FragmentShaderMap_;
	TProgramMap							ProgramMap_;
};


#endif
