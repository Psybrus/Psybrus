/**************************************************************************
*
* File:		GaSwarmComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaGameComponent.h"

#include "GaMainGameState.h"
#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaGameComponent )

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaGameComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaGameComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaGameComponent::initialise()
{
	LastEmote_ = BcErrorCode;
	EmoteTimer_ = 0;

	ScnMaterialRef Material;

	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_EMOTE, Material );
	CsCore::pImpl()->createResource( BcName::INVALID, EmoteMaterialComponent_, Material, scnSPF_DEFAULT );
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaGameComponent::update( BcReal Tick )
{
	BcScopedLock< BcMutex > Lock( EmoteLock_ );

	TEmoteList::iterator It( EmoteList_.begin() );

	while( It != EmoteList_.end() )
	{
		TEmote& Emote = (*It);
		Emote.Timeleft_ -= Tick;
		Emote.Position_ += BcVec3d( 0.0f, 0.0f, 4.0f ) * Tick;

		if( Emote.Timeleft_ < 0.0f )
		{
			It = EmoteList_.erase( It );
		}
		else
		{
			++It;
		}
	}

	EmoteTimer_ -= Tick;
}

////////////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaGameComponent::render( ScnCanvasComponentRef Canvas )
{
	BcScopedLock< BcMutex > Lock( EmoteLock_ );

	Canvas->setMaterialComponent( EmoteMaterialComponent_ );
	pParent()->setMaterialComponentParams( EmoteMaterialComponent_, BcMat4d() );

	TEmoteList::iterator It( EmoteList_.begin() );

	while( It != EmoteList_.end() )
	{
		TEmote& Emote = (*It);
		Canvas->drawSpriteCenteredUp3D( Emote.Position_, BcVec2d( 64.0f, -64.0f ), Emote.Idx_, RsColour( 1.0f, 1.0f, 1.0f, Emote.Timeleft_ ), 10 );
		++It;
	}
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );	

	Parent->attach( EmoteMaterialComponent_ );
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Parent->detach( EmoteMaterialComponent_ );
}

////////////////////////////////////////////////////////////////////////////////
// doEmote
BcBool GaGameComponent::doEmote( BcU32 Idx, const BcVec3d& Position )
{
	BcScopedLock< BcMutex > Lock( EmoteLock_ );

	if( EmoteTimer_ < 0.0f || Idx == EMOTE_SCARED )
	{
		TEmote Emote = 
		{
			Idx,
			1.0f,
			Position
		};

		EmoteList_.push_back( Emote );

		LastEmote_ = Idx;
		EmoteTimer_ = 0.25f;

		return BcTrue;
	}

	return BcFalse;
}
