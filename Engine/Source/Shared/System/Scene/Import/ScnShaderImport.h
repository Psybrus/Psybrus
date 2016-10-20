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

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Scene/Rendering/ScnModelFileData.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Import structures.
struct ScnShaderPermutationEntry
{
	ScnShaderPermutationFlags Flag_;
	std::string Define_;
	std::string Value_;
};

struct ScnShaderPermutationGroup
{
	template < size_t _Size >
	inline ScnShaderPermutationGroup( ScnShaderPermutationEntry ( &Entries )[ _Size ] ):
		Entries_( Entries ),
		NoofEntries_( _Size )
	{
		
	}

	ScnShaderPermutationEntry* Entries_;
	BcU32 NoofEntries_;
};

struct ScnShaderPermutation
{
	inline ScnShaderPermutation():
		Flags_( ScnShaderPermutationFlags::NONE )
	{

	}

	ScnShaderPermutationFlags Flags_;
	std::map< std::string, std::string > Defines_;
};

struct ScnShaderLevelEntry
{
	std::string Level_;
	RsShaderType Type_;
	RsShaderCodeType CodeType_;
};

struct ScnShaderBuiltData
{
	inline ScnShaderBuiltData():
		Hash_( 0 ),
		ShaderType_( RsShaderType::INVALID ),
		CodeType_( RsShaderCodeType::INVALID ) 
	{
	}

	inline ScnShaderBuiltData( ScnShaderBuiltData&& Other )
	{
		Hash_ = std::move( Other.Hash_ );
		ShaderType_ = std::move( Other.ShaderType_ );
		CodeType_ = std::move( Other.CodeType_ );
		Code_ = std::move( Other.Code_ );
	}

	inline ScnShaderBuiltData& operator = ( ScnShaderBuiltData&& Other )
	{
		Hash_ = std::move( Other.Hash_ );
		ShaderType_ = std::move( Other.ShaderType_ );
		CodeType_ = std::move( Other.CodeType_ );
		Code_ = std::move( Other.Code_ );
		return *this;
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


	BcU32 Hash_;
	RsShaderType ShaderType_;
	RsShaderCodeType CodeType_;
	BcBinaryData Code_;
	// Not for comparison.
	ScnShaderComplexity Complexity_;
};

struct ScnShaderPermutationJobParams
{
	RsShaderCodeType InputCodeType_;
	RsShaderCodeType OutputCodeType_;
	std::string ShaderSource_;
	std::string ShaderSourceData_;
	ScnShaderPermutation Permutation_;
	std::vector< ScnShaderLevelEntry > Entries_;

	ScnShaderPermutationJobParams& operator = ( ScnShaderPermutationJobParams& Other )
	{
		InputCodeType_ = Other.InputCodeType_;
		OutputCodeType_ = Other.OutputCodeType_;
		Permutation_ = Other.Permutation_;
		Entries_ = Other.Entries_;
		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderImport
class ScnShaderImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnShaderImport, CsResourceImporter );

public:
	ScnShaderImport();
	ScnShaderImport( ReNoInit );
	virtual ~ScnShaderImport();

	/**
	 * Import.
	 */
	BcBool import() override;

	void addDependency( const BcChar* Dependency );

private:
	void regenerateShaderDataHeader();
	void writeField( std::string& OutString, const ReClass* InClass, const ReField* Field, std::string Indentation, RsShaderBackendType OutputBackend );

	BcBool oldPipeline();
	BcBool newPipeline();
	ScnShaderPermutation getDefaultPermutation();

	BcBool compileShader( 
		const std::string& FileName,
		const std::string& EntryPoint,
		const std::map< std::string, std::string >& Defines, 
		const std::vector< std::string >& IncludePaths,
		const std::string& Target,
		BcBinaryData& ShaderByteCode,
		std::vector< std::string >& ErrorMessages );

	RsProgramVertexAttributeList extractShaderVertexAttributes(
		BcBinaryData& ShaderByteCode );

	void generatePermutations( 
		BcU32 GroupIdx, 
		BcU32 NoofGroups,
		ScnShaderPermutationGroup* PermutationGroups, 
		ScnShaderPermutation Permutation );

	BcBool buildPermutation( ScnShaderPermutationJobParams Params );
	BcBool buildPermutationHLSL( const ScnShaderPermutationJobParams& Params );
	BcBool buildPermutationGLSL( const ScnShaderPermutationJobParams& Params );

	BcU32 generateShaderHash( const ScnShaderBuiltData& Data );

	void logSource( std::string Source );

	RsProgramVertexAttribute semanticToVertexAttribute( BcU32 Channel, const std::string& Name, BcU32 Index );

private:
	// Old HLSL only.
	std::string Source_;
	std::string SourceFileData_;

	std::map< RsShaderType, std::string > Entrypoints_;
	std::vector< ScnShaderPermutationFlags > ExcludePermutations_;
	std::vector< ScnShaderPermutationFlags > IncludePermutations_;
	std::vector< RsShaderCodeType > CodeTypes_;
	std::vector< RsShaderBackendType > BackendTypes_;

	// New any lang.
	std::map< RsShaderCodeType, std::string > Sources_;
	std::map< RsShaderCodeType, std::string > SourcesFileData_;
	std::map< std::string, std::string > Defines_;
	BcBool UsePermutations_;

	//
	std::vector< RsShaderCodeType > OutputCodeTypes_;
	std::vector< ScnShaderPermutation > Permutations_;
	std::vector< std::string > IncludePaths_;
	std::vector< std::string > ErrorMessages_;
	
	std::atomic< BcU32 > ShaderCompileId_;
	std::mutex BuildingMutex_;
	std::map< BcU32, ScnShaderBuiltData > BuiltShaderData_;
	std::vector< ScnShaderProgramHeader > BuiltProgramData_;
	std::vector< RsProgramVertexAttributeList > BuiltVertexAttributes_;
	std::vector< RsProgramParameterList > BuiltParameters_;

	std::atomic< BcU32 > GotErrorBuilding_;
	std::atomic< BcU32 > PendingPermutations_;
	std::string IntermediatePath_;

	std::unordered_map< const ReClass*, std::string > ShaderClassMapping_;
};

#endif // __SCNSHADERIMPORT_H__

