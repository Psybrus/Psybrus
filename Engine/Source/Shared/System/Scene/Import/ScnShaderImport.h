/**************************************************************************
*
* File:		ScnShaderImport.h
* Author:	Neil Richardson 
* Ver/Date:
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSHADERIMPORT_H__
#define __SCNSHADERIMPORT_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Scene/ScnModelFileData.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnShaderImport
class ScnShaderImport
{
public:
	ScnShaderImport();

	/**
	 * Import.
	 */
	BcBool import( class CsPackageImporter& Importer, const Json::Value& Object );

private:
	BcBool legacyImport( class CsPackageImporter& Importer, const Json::Value& Object );

	BcBool compileShader( const std::string& FileName,
	                      const std::string& EntryPoint,
	                      const std::map< std::string, std::string >& Defines, 
						  const std::vector< std::string >& IncludePaths,
						  const std::string& Target,
						  BcStream& ShaderByteCode,
						  std::vector< std::string >& ErrorMessages );

	std::string removeComments( std::string Input );

	eRsVertexChannel semanticToVertexChannel( const std::string& Name, BcU32 Index );
};

#endif // PSY_SERVER
#endif / __SCNSHADERIMPORT_H__

