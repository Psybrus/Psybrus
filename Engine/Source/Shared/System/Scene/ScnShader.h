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

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTexture.h"
#include "System/Scene/ScnShaderFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnShader
typedef ReObjectRef< class ScnShader > ScnShaderRef;

//////////////////////////////////////////////////////////////////////////
// ScnShader
class ScnShader:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnShader );
	
#ifdef PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	
	RsProgram*							getProgram( BcU32 PermutationFlags );
	
private:
	typedef std::map< BcU32, RsShader* > TShaderMap;
	typedef std::map< BcU32, RsProgram* > TProgramMap;
	typedef TShaderMap::iterator TShaderMapIterator;
	typedef TProgramMap::iterator TProgramMapIterator;

	struct ShaderContainer
	{
		TShaderMap Shaders_;
	};

	RsShader*							getShader( BcU32 PermutationFlags, TShaderMap& ShaderMap );
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
private:
	ScnShaderHeader*					pHeader_;
	RsProgramVertexAttribute*			pVertexAttributes_;
	std::array< ShaderContainer, (BcU32)RsShaderType::MAX > ShaderMappings_;
	TProgramMap							ProgramMap_;

};

#endif
