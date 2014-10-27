/**************************************************************************
*
* File:		ScnShaderImportCompile.cpp
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnShaderImport.h"

#include "Base/BcFile.h"

#include <boost/format.hpp>

#if PLATFORM_WINDOWS
#include "Base/BcComRef.h"

#include <D3DCompiler.h>
#include <D3DCompiler.inl>

#pragma comment( lib, "D3DCompiler.lib" )
#pragma comment (lib, "dxguid.lib")

namespace
{
	class ScnShaderIncludeHandler : public ID3DInclude 
	{
	public:
		ScnShaderIncludeHandler( 
			class ScnShaderImport& Importer,
			const std::vector< std::string >& IncludePaths ):
			Importer_( Importer ),
			IncludePaths_( IncludePaths )
		{

		}

		HRESULT __stdcall Open( D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
		{
			BcFile IncludeFile;

			for( auto& IncludePath : IncludePaths_ )
			{
				std::string IncludeFileName = IncludePath + pFileName;

				if( IncludeFile.open( IncludeFileName.c_str(), bcFM_READ ) )
				{
					Importer_.addDependency( IncludeFileName.c_str() );
					*ppData = IncludeFile.readAllBytes();
					*pBytes = IncludeFile.size();
					return S_OK;
				}
			}

			return E_FAIL;
		}

		HRESULT __stdcall Close(LPCVOID pData)
		{
			BcMemFree( (void*)pData );
			return S_OK;
		}

	private:
		class ScnShaderImport& Importer_;
		const std::vector< std::string >& IncludePaths_;
	};
}
#endif // PLATFORM_WINDOWS

#if PLATFORM_LINUX
#include <cstdlib>
#endif // PLATFORM_LINUX

BcBool ScnShaderImport::compileShader( 
	const std::string& FileName,
	const std::string& EntryPoint,
	const std::map< std::string, std::string >& Defines, 
	const std::vector< std::string >& IncludePaths,
	const std::string& Target,
	BcBinaryData& ShaderByteCode,
	std::vector< std::string >& ErrorMessages )
{
	BcBool RetVal = BcFalse;
#if PLATFORM_WINDOWS
	CsResourceImporter::addDependency( FileName.c_str() );

	std::wstring WFileName( FileName.begin(), FileName.end() );
	// Create macros.
	std::vector< D3D_SHADER_MACRO > Macros;
	Macros.reserve( Defines.size() + 1 );
	for( auto& DefineEntry : Defines )
	{
		D3D_SHADER_MACRO Macro = { DefineEntry.first.c_str(), DefineEntry.second.c_str() };
		Macros.push_back( Macro );
	}
	D3D_SHADER_MACRO EmptyMacro = { nullptr, nullptr };
	Macros.push_back( EmptyMacro );

	ID3D10Blob* OutByteCode;
	ID3D10Blob* OutErrorMessages;
	ScnShaderIncludeHandler IncludeHandler( *this, IncludePaths );
	D3DCompileFromFile( WFileName.c_str(), &Macros[ 0 ], &IncludeHandler, EntryPoint.c_str(), Target.c_str(), 0, 0, &OutByteCode, &OutErrorMessages );

	// Extract byte code if we have it.
	if( OutByteCode != nullptr )
	{
		ShaderByteCode = std::move( BcBinaryData( OutByteCode->GetBufferPointer(), OutByteCode->GetBufferSize(), BcTrue ) );
		OutByteCode->Release();

		RetVal = BcTrue;
	}

	// Extract error messages if we have any.
	if( OutErrorMessages != nullptr )
	{
		size_t BufferSize = OutErrorMessages->GetBufferSize();
		LPVOID BufferData = OutErrorMessages->GetBufferPointer();

		// TODO: Split up into lines.
		std::string Error = (const char*)BufferData;
		ErrorMessages.push_back( Error );
		OutErrorMessages->Release();
	}
#endif // PLATFORM_WINDOWS

#if PLATFORM_LINUX
	// LINUX TODO: Use env path or config file.
	auto PsybrusSDKRoot = "../../Psybrus";

	// Generate some unique ids.
	BcU32 ShaderCompileId = ++ShaderCompileId_;
	std::string BytecodeFilename = getIntermediatePath() + 
		boost::str( boost::format( "/built_shader_%1%.bytecode" ) % ShaderCompileId );
	std::string LogFilename = getIntermediatePath() + 
		boost::str( boost::format( "/built_shader_%1%.bytecode" ) % ShaderCompileId );

	std::string CommandLine = std::string( "wine " ) + PsybrusSDKRoot + "/Tools/ShaderCompiler/ShaderCompiler.exe";
	CommandLine += std::string( " -i" ) + FileName;
	CommandLine += std::string( " -e" ) + LogFilename;
	CommandLine += std::string( " -o" ) + BytecodeFilename;
	CommandLine += std::string( " -T" ) + Target;
	CommandLine += std::string( " -E" ) + EntryPoint;
	for( auto Define : Defines )
	{
		CommandLine += " -D" + Define.first + "=" + Define.second;
	}
	for( auto IncludePath : IncludePaths )
	{
		CommandLine += " -I" + IncludePath;
	}
	int RetCode = std::system( CommandLine.c_str() );

	// If successful, load in output file.
	if( RetCode == 0 )
	{
		BcFile ByteCodeFile;
		if( ByteCodeFile.open( ( BytecodeFilename ).c_str(), bcFM_READ ) )
		{
			auto ByteCode = ByteCodeFile.readAllBytes();
			ShaderByteCode = std::move( BcBinaryData( ByteCode, ByteCodeFile.size(), BcTrue ) );
			BcMemFree( ByteCode );
			RetVal = BcTrue;
		}
	}

#endif // PLATFORM_LINUX

	return RetVal;
}

RsProgramVertexAttributeList ScnShaderImport::extractShaderVertexAttributes(
	BcBinaryData& ShaderByteCode )
{
	RsProgramVertexAttributeList VertexAttributeList;
#if PLATFORM_WINDOWS
	BcComRef< ID3D11ShaderReflection > ShaderReflection;
	D3D11Reflect( ShaderByteCode.getData< const BcU8 >( 0 ), ShaderByteCode.getDataSize(), &ShaderReflection );

	BcU32 ChannelIdx = 0;
	for( BcU32 Idx = 0; Idx < 16; ++Idx )
	{
		D3D11_SIGNATURE_PARAMETER_DESC Desc;
		if( SUCCEEDED( ShaderReflection->GetInputParameterDesc( Idx, &Desc ) ) )
		{
			auto VertexAttribute = semanticToVertexAttribute( ChannelIdx++, Desc.SemanticName, Desc.SemanticIndex ); 
			VertexAttributeList.push_back( VertexAttribute );
		}
	}
#endif // PLATFORM_WINDOWS


#if PLATFORM_LINUX
	struct VertexAttribute
	{
		char SemanticName_[ 32 ];
		unsigned int SemanticIndex_;
		unsigned int ChannelIdx_;
	};

	BcU32 SizeOfVertexAttrs = *ShaderByteCode.getData< BcU32 >( ShaderByteCode.getDataSize() - sizeof( BcU32 ) );
	BcU32 NoofVertexAttrs = SizeOfVertexAttrs / sizeof( VertexAttribute );
	VertexAttribute* VertexAttrs = ShaderByteCode.getData< VertexAttribute >( ShaderByteCode.getDataSize() - ( SizeOfVertexAttrs + sizeof( BcU32 ) ) );
	for( BcU32 Idx = 0; Idx < NoofVertexAttrs; ++Idx )
	{
		auto VertexAttribute = semanticToVertexAttribute( VertexAttrs[ Idx ].ChannelIdx_, VertexAttrs[ Idx ].SemanticName_, VertexAttrs[ Idx ].SemanticIndex_ ); 
		VertexAttributeList.push_back( VertexAttribute );
	}

#endif // PLATFORM_LINUX

	return VertexAttributeList;
}
