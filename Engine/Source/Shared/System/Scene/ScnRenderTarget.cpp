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
	return Super::import( Importer, Object );
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderTarget );

BCREFLECTION_EMPTY_REGISTER( ScnRenderTarget );
/*
BCREFLECTION_DERIVED_BEGIN( CsResource, ScnRenderTarget )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRenderTarget::initialise( BcU32 Width, BcU32 Height )
{
	ScnTexture::initialise();
	
	Header_.Width_ = Width;
	Header_.Height_ = Height;
	Header_.Levels_ = 1;
	Header_.Format_ = rsTF_RGBA8;	

	//
	pRenderTarget_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRenderTarget::create()
{
	// Create render target.
	pRenderTarget_ = RsCore::pImpl()->createRenderTarget( Header_.Width_, Header_.Height_, rsCF_A8R8G8B8, rsDSF_D24S8 );
	
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

