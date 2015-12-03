/**************************************************************************
*
* File:		Rendering/ScnShader.h
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

#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// ScnShader
typedef ReObjectRef< class ScnShader > ScnShaderRef;

//////////////////////////////////////////////////////////////////////////
// ScnShader
class ScnShader:
	public CsResource
{
public:
	static const BcU32 LOAD_FROM_FILE_TAG = 0x01020304;

public:
	REFLECTION_DECLARE_DERIVED( ScnShader, CsResource );

	ScnShader();
	virtual ~ScnShader();
	
	void create() override;
	void destroy() override;
	
	RsProgram* getProgram( ScnShaderPermutationFlags PermutationFlags );
	
private:
	typedef std::map< BcU32, RsShader* > TShaderMap;
	typedef std::map< ScnShaderPermutationFlags, RsProgram* > TProgramMap;
	typedef TShaderMap::iterator TShaderMapIterator;
	typedef TProgramMap::iterator TProgramMapIterator;

	struct ShaderContainer
	{
		TShaderMap Shaders_;
	};

	RsShader* getShader( BcU32 Hash, TShaderMap& ShaderMap );
	
private:
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;
	
private:
	ScnShaderHeader* pHeader_;
	std::array< ShaderContainer, (BcU32)RsShaderType::MAX > ShaderMappings_;
	TProgramMap ProgramMap_;
	RsShaderCodeType TargetCodeType_;
	BcU32 TotalProgramsLoaded_;

};

#endif
