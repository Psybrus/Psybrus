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
// Import structures.
struct ScnShaderPermutationEntry
{
	BcU32							Flag_;
	std::string						Define_;
	std::string						Value_;
};

struct ScnShaderPermutationGroup
{
	template < size_t _Size >
	ScnShaderPermutationGroup( ScnShaderPermutationEntry ( &Entries )[ _Size ] ):
		Entries_( Entries ),
		NoofEntries_( _Size )
	{
		
	}

	ScnShaderPermutationEntry*		Entries_;
	BcU32							NoofEntries_;
};

struct ScnShaderPermutation
{
	BcU32							Flags_;
	std::map< std::string, std::string > Defines_;
};

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

	void generatePermutations( BcU32 GroupIdx, 
	                           ScnShaderPermutationGroup* PermutationGroups, 
	                           ScnShaderPermutation Permutation );

	std::string removeComments( std::string Input );

	eRsVertexChannel semanticToVertexChannel( const std::string& Name, BcU32 Index );

private:
	std::vector< ScnShaderPermutation > Permutations_;
};

#endif // PSY_SERVER
#endif / __SCNSHADERIMPORT_H__

