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

#include "System/Scene/ScnRenderTarget.h"

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
		rsTT_2D, 
		rsTF_RGBA8
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
	static const ReField Fields[] = 
	{
		ReField( "pRenderTarget_",			&ScnRenderTarget::pRenderTarget_ ),
	};
		
	ReRegisterClass< ScnRenderTarget, Super >( Fields );
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
	Header_.Type_ = rsTT_2D;
	Header_.Format_ = rsTF_RGBA8;

	//
	pRenderTarget_ = NULL;

	markCreate();
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRenderTarget::create()
{
	// Create render target.
	pRenderTarget_ = RsCore::pImpl()->createRenderTarget( 
		RsRenderTargetDesc( Header_.Width_, Header_.Height_ )
			.renderSurface( 0, rsCF_A8R8G8B8 )
			.depthStencilSurface( rsDSF_D24S8 )
		);
	
	// Get texture from target.
	pTexture_ = pRenderTarget_->getTexture( 0 );
	
	// Ready to go.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRenderTarget::destroy()
{
	RsCore::pImpl()->destroyResource( pRenderTarget_ );
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
	pFrame->setRenderTarget( pRenderTarget_ );	
}

//////////////////////////////////////////////////////////////////////////
// unbind
void ScnRenderTarget::unbind( RsFrame* pFrame )
{
	pFrame->setRenderTarget( NULL );	
}

