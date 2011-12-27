/**************************************************************************
*
* File:		CsTypes.cpp
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "CsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependancy::CsDependancy( const BcPath& FileName )
{
	FileName_ = FileName;
	updateStats();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependancy::CsDependancy( const CsDependancy& Other )
{
	FileName_ = Other.FileName_;
	Stats_ = Other.Stats_;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsDependancy::~CsDependancy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getFileName
const BcPath& CsDependancy::getFileName() const
{
	return FileName_;
}

//////////////////////////////////////////////////////////////////////////
// hasChanged
BcBool CsDependancy::hasChanged()
{
	FsStats Stats;
	if( FsCore::pImpl()->fileStats( *FileName_, Stats ) )
	{
		if( Stats.ModifiedTime_ != Stats_.ModifiedTime_ )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// updateStats
void CsDependancy::updateStats()
{
	FsCore::pImpl()->fileStats( *FileName_, Stats_ );
}
