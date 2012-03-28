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

	void							playSound( const BcChar* pSoundName, const BcFixedVec2d& Position );

private:
	typedef std::vector< CsResourceRef<> > TResourceList;
	typedef std::vector< ScnEntityRef > TEntityList;

	typedef std::map< std::string, ScnSoundRef > TSoundMap;

	ScnMaterialRef					BackgroundMaterial_;
	ScnMaterialRef					SpriteSheetMaterial0_;
	ScnMaterialRef					SpriteSheetMaterial1_;
	ScnMaterialRef					HUDMaterial_;

	TSoundMap						SoundMap_;

	TResourceList					ResourceList_;
	TEntityList						EntityList_;
};

#endif
