/**************************************************************************
*
* File:		GaTitleComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Title component.
*		
*
*
* 
**************************************************************************/

#include "GaMatchmakingState.h"

#include "GaTitleComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaTitleComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaTitleComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaTitleComponent" )
		.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaTitleComponent::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// destroy
void GaTitleComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// GaTitleComponent
//virtual
void GaTitleComponent::update( BcReal Tick )
{
	if( pState_ && pState_->process() )
	{
		delete pState_;
		pState_ = NULL;
	}

	if( Canvas_.isValid() )
	{
		OsClient* pClient = OsCore::pImpl()->getClient( 0 );
		BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
		BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
		BcReal AspectRatio = HW / HH;

		BcMat4d Ortho;
		Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );

		// Clear canvas and push projection matrix.
		Canvas_->clear();   
		Canvas_->pushMatrix( Ortho );
		Canvas_->setMaterialComponent( Material_ );
		Canvas_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 1280.0f, 720.0f ), 0, RsColour::WHITE, 0 );

		Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), "HELLO!", RsColour::WHITE, BcFalse );
	}
}

//////////////////////////////////////////////////////////////////////////
// GaTitleComponent
//virtual
void GaTitleComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find canvas component on parent. TODO: Make a utility function for this.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		Canvas_ = Parent->getComponent( Idx );

		if( Canvas_.isValid() )
		{
			break;
		}
	}

	// Materials.
	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "title", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, Material_, Material, BcErrorCode ) )
		{
			Parent->attach( Material_ );
		}
	}

	// Font
	ScnFontRef Font;
	if( CsCore::pImpl()->requestResource( "default", Font ) && CsCore::pImpl()->requestResource( "font", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, Font_, Font, Material ) )
		{
			Parent->attach( Font_ );
		}
	}

	// Bind input events.
	OsEventInputKeyboard::Delegate OnKeyEvent = OsEventInputKeyboard::Delegate::bind< GaTitleComponent, &GaTitleComponent::onKeyEvent >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyEvent );

	// Create state.
	pState_ = new GaMatchmakingState();

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaTitleComponent
//virtual
void GaTitleComponent::onDetach( ScnEntityWeakRef Parent )
{
	if( pState_ )
	{
		pState_->leaveState();
		while( !pState_->process() );
		delete pState_;
		pState_ = NULL;
	}
	delete pState_;
	pState_ = NULL;

	// Null canvas reference.
	Canvas_ = NULL;

	// Detach materials.
	Parent->detach( Material_ );
	Parent->detach( FontMaterial_ );
	Parent->detach( Font_ );

	// Unsubscribe.
	OsCore::pImpl()->unsubscribeAll( this );

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onKeyEvent
eEvtReturn GaTitleComponent::onKeyEvent( EvtID ID, const OsEventInputKeyboard& Event )
{

	return evtRET_PASS;
}

