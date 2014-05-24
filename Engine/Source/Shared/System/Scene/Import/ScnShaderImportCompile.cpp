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

#ifdef PSY_SERVER

#include "Base/BcFile.h"

#include <D3DCompiler.h>
#pragma comment( lib, "D3DCompiler.lib" )

namespace
{
	class ScnShaderIncludeHandler : public ID3DInclude 
	{
	public:
		ScnShaderIncludeHandler( const std::vector< std::string >& IncludePaths ):
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
		const std::vector< std::string >& IncludePaths_;
	};
}


BcBool ScnShaderImport::compileShader( const std::string& FileName,
	                                   const std::string& EntryPoint,
	                                   const std::map< std::string, std::string >& Defines, 
	                                   const std::vector< std::string >& IncludePaths,
	                                   const std::string& Target,
	                                   BcBinaryData& ShaderByteCode,
	                                   std::vector< std::string >& ErrorMessages )
{
	BcBool RetVal = BcFalse;
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
	ScnShaderIncludeHandler IncludeHandler( IncludePaths );
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
		ErrorMessages.push_back( (const char*)BufferData );
		OutErrorMessages->Release();
	}

	return RetVal;
}


#endif

