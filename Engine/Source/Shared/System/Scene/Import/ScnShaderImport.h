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
	inline ScnShaderPermutationGroup( ScnShaderPermutationEntry ( &Entries )[ _Size ] ):
		Entries_( Entries ),
		NoofEntries_( _Size )
	{
		
	}

	ScnShaderPermutationEntry*		Entries_;
	BcU32							NoofEntries_;
};

struct ScnShaderPermutation
{
	inline ScnShaderPermutation():
		Flags_( 0 )
	{

	}

	BcU32							Flags_;
	std::map< std::string, std::string > Defines_;
};

struct ScnShaderLevelEntry
{
	std::string						Level_;
	std::string						Entry_;
	eRsShaderType					Type_;
};

struct ScnShaderBuiltData
{
	inline ScnShaderBuiltData():
		Hash_( 0 ),
		ShaderType_( rsST_INVALID ),
		CodeType_( scnSCT_INVALID ) 
	{
	}

	inline ScnShaderBuiltData( ScnShaderBuiltData&& Other )
	{
		Hash_ = std::move( Other.Hash_ );
		ShaderType_ = std::move( Other.ShaderType_ );
		CodeType_ = std::move( Other.CodeType_ );
		Code_ = std::move( Other.Code_ );
	}

	inline BcBool operator == ( const ScnShaderBuiltData& Other ) const
	{
		return Hash_ == Other.Hash_ &&
			   ShaderType_ == Other.ShaderType_ &&
		       CodeType_ == Other.CodeType_ &&
		       Code_ == Other.Code_;
	}

	inline BcBool operator != ( const ScnShaderBuiltData& Other ) const
	{
		return Hash_ != Other.Hash_ ||
			   ShaderType_ != Other.ShaderType_ ||
		       CodeType_ != Other.CodeType_ ||
		       Code_ != Other.Code_;
	}


	BcU32							Hash_;
	eRsShaderType					ShaderType_;
	ScnShaderCodeType				CodeType_;
	BcBinaryData					Code_;
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
						  BcBinaryData& ShaderByteCode,
						  std::vector< std::string >& ErrorMessages );

	void generatePermutations( BcU32 GroupIdx, 
							   BcU32 NoofGroups,
	                           ScnShaderPermutationGroup* PermutationGroups, 
	                           ScnShaderPermutation Permutation );

	BcBool buildPermutation( class CsPackageImporter& Importer, const ScnShaderPermutation& Permutation );

	std::string removeComments( std::string Input );

	eRsVertexChannel semanticToVertexChannel( const std::string& Name, BcU32 Index );

private:
	std::string							Filename_;
	std::vector< ScnShaderPermutation > Permutations_;
	std::list< ScnShaderLevelEntry >	Entries_;
	std::vector< std::string >			IncludePaths_;
	std::vector< std::string >			ErrorMessages_;

	std::map< BcU32, ScnShaderBuiltData > BuiltShaderData_;
	std::vector< ScnShaderProgramHeader > BuiltProgramData_;
	std::vector< std::vector< RsProgramVertexAttribute > > BuiltVertexAttributes_;
};

#endif // PSY_SERVER
#endif / __SCNSHADERIMPORT_H__

