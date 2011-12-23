/**************************************************************************
*
* File:		GaTopState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Game top state.
*		
*
*
* 
**************************************************************************/

#ifndef __GATOPSTATE_H__
#define __GATOPSTATE_H__

#include "Psybrus.h"

#include "GaBaseGameState.h"

////////////////////////////////////////////////////////////////////////////////
// GaTopState
class GaTopState: 
	public SysState,
	public BcGlobal< GaTopState >
{
public:
	enum TMaterials
	{
		MATERIAL_BUNNY = 0,
		MATERIAL_KITTY,
		MATERIAL_LETTUCE,
		MATERIAL_BACKGROUND,
		MATERIAL_FOREGROUND,
		MATERIAL_BAR,
		MATERIAL_EMOTE,
		MATERIAL_SPLASH,
		MATERIAL_WIN,
		MATERIAL_LOSE,

		//
		MATERIAL_MAX
	};

	enum TSounds
	{
		SOUND_CHEW0 = 0,
		SOUND_CHEW1,
		SOUND_CHEW2,
		SOUND_HOP0,
		SOUND_HOP1,
		SOUND_HOP2,
		SOUND_HOP3,
		SOUND_HOP4,
		SOUND_SCARED,

		SOUND_MAX
	};

public:
	GaTopState();
	virtual ~GaTopState();

	virtual void					enterOnce();
	virtual eSysStateReturn			enter();
	virtual void					preMain();
	virtual eSysStateReturn			main();
	virtual void					preLeave();
	virtual eSysStateReturn			leave();
	virtual void					leaveOnce();

	void							addState( GaBaseGameState* pState );
	void							removeState( GaBaseGameState* pState );

	void							getMaterial( BcU32 Idx, ScnMaterialRef& Material );
	ScnSoundRef						getSound( BcU32 Idx );

private:
	typedef std::list< GaBaseGameState* > TStateList;

	ScnCanvasRef					Canvas_;

	TStateList						StateList_;

	ScnMaterialRef					Materials_[ MATERIAL_MAX ];
	ScnSoundRef						Sounds_[ SOUND_MAX ];

};

#endif
