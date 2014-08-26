/**************************************************************************
*
* File:		ScnRenderTarget.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnRenderTarget.h"

#include "System/Content/CsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnRenderTarget::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	BcU32 Width = Object[ "width" ].asUInt();
	BcU32 Height = Object[ "height" ].asUInt();

	// Streams.
	BcStream HeaderStream;

	// Write header.
	ScnTextureHeader Header =
	{
		Width,
		Height,
		1,
		1, 
		RsTextureType::TEX2D, 
		RsTextureFormat::R8G8B8A8
	};
	HeaderStream << Header;

	Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );

	return BcTrue;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderTarget );

void ScnRenderTarget::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "pRenderTarget_",			&ScnRenderTarget::pRenderTarget_ ),
	};
		
	ReRegisterClass< ScnRenderTarget, Super >( Fields );
	*/
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRenderTarget::initialise( BcU32 Width, BcU32 Height )
{
	ScnTexture::initialise();
	
	Header_.Width_ = Width;
	Header_.Height_ = Height;
	Header_.Depth_ = 1;
	Header_.Levels_ = 1;
	Header_.Type_ = RsTextureType::TEX2D;
	Header_.Format_ = RsTextureFormat::R8G8B8A8;

	markCreate();
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRenderTarget::create()
{
	BcBreakpoint;
	
	// Ready to go.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRenderTarget::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// fileReady
//virtual
void ScnRenderTarget::fileReady()
{
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void ScnRenderTarget::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		markCreate();
	}
}

//////////////////////////////////////////////////////////////////////////
// bind
void ScnRenderTarget::bind( RsFrame* pFrame )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// unbind
void ScnRenderTarget::unbind( RsFrame* pFrame )
{
	BcBreakpoint;
}

