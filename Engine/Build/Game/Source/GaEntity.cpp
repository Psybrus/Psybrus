/**************************************************************************
*
* File:		GaSwarmEntity.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaEntity.h"

#include "GaMainGameState.h"
#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaEntity::GaEntity()
{
	LastEmote_ = BcErrorCode;
	EmoteTimer_ = 0;

	ScnMaterialRef Material;

	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_EMOTE, Material );
	Material->createInstance( "emotematerialinstance", EmoteMaterialInstance_, BcErrorCode );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaEntity::~GaEntity()
{
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaEntity::update( BcReal Tick )
{
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
void GaEntity::render( ScnCanvasRef Canvas )
{
	Canvas->setMaterialInstance( EmoteMaterialInstance_ );
	pParent()->setMaterialInstanceParams( EmoteMaterialInstance_, BcMat4d() );

	TEmoteList::iterator It( EmoteList_.begin() );

	while( It != EmoteList_.end() )
	{
		TEmote& Emote = (*It);
		Canvas->drawSpriteCenteredUp3D( Emote.Position_, BcVec2d( 64.0f, -64.0f ), Emote.Idx_, RsColour( 1.0f, 1.0f, 1.0f, Emote.Timeleft_ ), 10 );
		++It;
	}
}

////////////////////////////////////////////////////////////////////////////////
// doEmote
BcBool GaEntity::doEmote( BcU32 Idx, const BcVec3d& Position )
{
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
