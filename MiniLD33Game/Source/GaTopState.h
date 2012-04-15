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


#include "GaGameComponent.h"
#include "GaTitleComponent.h"


////////////////////////////////////////////////////////////////////////////////
// GaTopState
class GaTopState: 
	public SysState,
	public BcGlobal< GaTopState >
{
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

	void							startMatchmaking();
	void							startGame( BcBool Networked );

	void							playSound( const BcChar* pSoundName, const BcFixedVec2d& Position );

private:
	typedef std::vector< ScnEntityRef > TEntityList;
	
	CsPackage*						pPackage_;
	
	GaGameComponentRef				GameComponent_;
	GaTitleComponentRef				TitleComponent_;

	ScnEntityRef					GameEntity_;

	TEntityList						EntityList_;

	BcBool							SpawnTitle_;
	BcBool							SpawnGame_;
	BcBool							Networked_;
};

#endif
