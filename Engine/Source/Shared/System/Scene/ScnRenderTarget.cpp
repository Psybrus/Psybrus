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

#include "ScnRenderTarget.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "json.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnRenderTarget::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderTarget );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRenderTarget::initialise( BcU32 Width, BcU32 Height )
{
	TextureHeader_.Width_ = Width;
	TextureHeader_.Height_ = Height;
	TextureHeader_.Levels_ = 1;
	TextureHeader_.Format_ = rsTF_RGBA8;	
	
	// Pass header to parent.
	pHeader_ = &TextureHeader_;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRenderTarget::create()
{
	// Create render target.
	pRenderTarget_ = RsCore::pImpl()->createRenderTarget( TextureHeader_.Width_, TextureHeader_.Height_, rsCF_A8R8G8B8, rsDSF_D24S8 );
	
	// Get texture from target.
	pTexture_ = pRenderTarget_->getTexture();
	
	// Render target manages this.
	CreateNewTexture_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRenderTarget::destroy()
{
	RsCore::pImpl()->destroyResource( pRenderTarget_ );
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnRenderTarget::isReady()
{
	return ScnTexture::isReady() && ( pRenderTarget_ != NULL && pRenderTarget_->hasHandle() );
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

