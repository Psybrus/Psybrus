#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <d3dcompiler.h>

const IID IID_ID3D11ShaderReflection = { 0x0a233719, 0x3960, 0x4578, { 0x9d, 0x7c, 0x20, 0x3b, 0x8b, 0x1d, 0x9c, 0xc1 } };

namespace
{
	class ShaderIncludeHandler : public ID3DInclude 
	{
	public:
		ShaderIncludeHandler( 
				const std::vector< std::string >& IncludePaths ):
			IncludePaths_( IncludePaths )
		{

		}

		HRESULT __stdcall Open( D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
		{
			for( auto& IncludePath : IncludePaths_ )
			{
				std::string IncludeFileName = IncludePath + pFileName;
				try
				{
					std::ifstream InputFile( IncludeFileName.c_str() );
					if( InputFile.is_open() )
					{
						std::string InputString( ( std::istreambuf_iterator< char >( InputFile ) ), std::istreambuf_iterator< char >() );
						CachedIncludes_[ IncludeFileName ] = InputString;
						*ppData = InputString.c_str();
						*pBytes = InputString.size();
						return S_OK;
					}
				}
				catch( ... )
				{

				}
			}

			return E_FAIL;
		}

		HRESULT __stdcall Close(LPCVOID pData)
		{
			delete [] (char*)pData;
			return S_OK;
		}

	private:
		const std::vector< std::string >& IncludePaths_;
		std::map< std::string, std::string > CachedIncludes_;
	};
}

void writeDocumentation()
{
	std::cout << "Psybrus Shader Compiler." << std::endl;
	std::cout << "Args:" << std::endl;
	std::cout << "-i<filename> = Input file. I.e. -idefault.hlsl" << std::endl;
	std::cout << "-e<filename> = Error file. I.e. -edefault.log" << std::endl;
	std::cout << "-o<filename> = Output file. I.e. -odefault.o" << std::endl;
	std::cout << "-T<entry> = Target shader. I.e. -Tvs_4_0, -Tps_5_0, etc.. See D3D10+11/FXC documentation." << std::endl;
	std::cout << "-E<target> = Shader entrypoint. I.e. -EvertexMain, -EhullMain, etc.." << std::endl;
	std::cout << "-I<includepath> = Include path. I.e. -IZ:\\home\\builder\\includes\\" << std::endl;
	std::cout << "-D<define>[=<value>] = Define. I.e. -DENABLE_LIGHTING, -DPASS_COUNT=5, etc.." << std::endl;
}

int main( int argc, char* argv[] )
{
	bool RetVal = false;

	// Input parameters.
	std::string InputFileName;
	std::string ErrorFileName;
	std::string OutputFileName;
	std::string EntryPoint;
	std::string Target;
	std::vector< std::string > IncludePaths;
	std::map< std::string, std::string > Defines; 

	IncludePaths.push_back( "." );

	// TODO: Parse command line.
	for( int ArgIdx = 0; ArgIdx < argc; ++ArgIdx )
	{
		std::string Arg = argv[ ArgIdx ];
		std::string Cmd = Arg.substr( 0, 2 );
		if( Cmd == "-i" )
		{
			InputFileName = Arg.substr( 2, Arg.size() - Cmd.size() );
		}
		else if( Cmd == "-e" )
		{
			ErrorFileName = Arg.substr( 2, Arg.size() - Cmd.size() );
		}
		else if( Cmd == "-o" )
		{
			OutputFileName = Arg.substr( 2, Arg.size() - Cmd.size() );
		}
		else if( Cmd == "-E" )
		{
			EntryPoint = Arg.substr( 2, Arg.size() - Cmd.size() );
		}
		else if( Cmd == "-T" )
		{
			Target = Arg.substr( 2, Arg.size() - Cmd.size() );
		}
		else if( Cmd == "-I" )
		{
			auto IncludePath = Arg.substr( 2, Arg.size() - Cmd.size() );
			IncludePaths.push_back( IncludePath );
		}
		else if( Cmd == "-D" )
		{
			auto FullDefine = Arg.substr( 2, Arg.size() - Cmd.size() );
			auto DefineAssign = FullDefine.find( "=" );
			if( DefineAssign == std::string::npos )
			{
				std::cout << "-D" << FullDefine << std::endl;
				Defines[ FullDefine ] = "1";
			}
			else
			{
				auto Define = FullDefine.substr( 0, DefineAssign );
				auto Value = FullDefine.substr( DefineAssign + 1, FullDefine.size() - DefineAssign + 1 );
				Defines[ Define ] = Value;
				std::cout << "-D" << Define << "=" << Value << std::endl;
			}
			auto IncludePath = Arg.substr( 2, Arg.size() - Cmd.size() );
			IncludePaths.push_back( IncludePath );
		}
	}
	//
	if( InputFileName.size() == 0 ||
		ErrorFileName.size() == 0 ||
		OutputFileName.size() == 0 ||
		Target.size() == 0 ||
		EntryPoint.size() == 0 )
	{
		writeDocumentation();
		return 1;
	}

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
	ShaderIncludeHandler IncludeHandler( IncludePaths );

	std::ifstream InputFile( InputFileName.c_str() );
	std::string InputString( ( std::istreambuf_iterator< char >( InputFile ) ), std::istreambuf_iterator< char >() );

	D3DCompile( InputString.c_str(), InputString.size(), InputFileName.c_str(), &Macros[ 0 ], &IncludeHandler, EntryPoint.c_str(), Target.c_str(), 0, 0, &OutByteCode, &OutErrorMessages );

	// Extract error messages if we have any.
	if( OutErrorMessages != nullptr )
	{
		size_t BufferSize = OutErrorMessages->GetBufferSize();
		LPVOID BufferData = OutErrorMessages->GetBufferPointer();

		std::ofstream OutFile ( ErrorFileName );
		OutFile.write( (const char*)BufferData, BufferSize );
		OutFile.close();

		// TODO: Split up into lines.
		std::string Error = (const char*)BufferData;
		std::cout << Error << std::endl;
		OutErrorMessages->Release();
	}

	// Extract byte code if we have it.
	if( OutByteCode != nullptr )
	{
		unsigned int BlockSize = 0;
		std::ofstream OutFile( OutputFileName, std::ofstream::binary );

		size_t BufferSize = OutByteCode->GetBufferSize();
		LPVOID BufferData = OutByteCode->GetBufferPointer();

		// Write out the size of the byte code and the byte code.
		BlockSize = BufferSize;
		OutFile.write( (const char*)&BlockSize, sizeof( BlockSize ) );
		OutFile.write( (const char*)BufferData, BufferSize );

		// Parse out reflection data.
		// NOTE: It should be in the byte code, so look into parsing it out later on from there instead
		//       of writing it out manually.
		ID3D11ShaderReflection* ShaderReflection = nullptr;
		HRESULT Result = D3DReflect( BufferData, BufferSize, IID_ID3D11ShaderReflection, (void**)&ShaderReflection );

		struct VertexAttribute
		{
			char SemanticName_[ 32 ];
			unsigned int SemanticIndex_;
			unsigned int ChannelIdx_;
		};

		std::vector< VertexAttribute > VertexAttributes;

		if( SUCCEEDED( Result ) )
		{
			int ChannelIdx = 0;
			for( int Idx = 0; Idx < 16; ++Idx )
			{
				D3D11_SIGNATURE_PARAMETER_DESC Desc;
				if( SUCCEEDED( ShaderReflection->GetInputParameterDesc( Idx, &Desc ) ) )
				{
					VertexAttribute VertexAttribute;
					strcpy_s( VertexAttribute.SemanticName_, Desc.SemanticName );
					VertexAttribute.SemanticIndex_ = Desc.SemanticIndex;
					VertexAttribute.ChannelIdx_ = ChannelIdx++;
					VertexAttributes.push_back( VertexAttribute );
				}
			}

			BlockSize = VertexAttributes.size() * sizeof( VertexAttribute );
			OutFile.write( (const char*)&BlockSize, sizeof( BlockSize ) );
			OutFile.write( (const char*)&VertexAttributes[ 0 ], BlockSize );

			RetVal = true;
		}
		else
		{
			std::cout << "Error creating D3D11ShaderReflection: " << Result << std::endl;
		}

		OutByteCode->Release();
	}


	return RetVal ? 0 : 1;
}
