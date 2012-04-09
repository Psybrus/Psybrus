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

#include "System/Content/CsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const BcPath& FileName )
{
	FileName_ = FileName;
	updateStats();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const BcPath& FileName, const FsStats& Stats )
{
	FileName_ = FileName;
	Stats_ = Stats;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const CsDependency& Other )
{
	FileName_ = Other.FileName_;
	Stats_ = Other.Stats_;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsDependency::~CsDependency()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getFileName
const BcPath& CsDependency::getFileName() const
{
	return FileName_;
}

//////////////////////////////////////////////////////////////////////////
// getStats
const FsStats& CsDependency::getStats() const
{
	return Stats_;
}

//////////////////////////////////////////////////////////////////////////
// hasChanged
BcBool CsDependency::hasChanged()
{
	FsStats Stats;
	if( FsCore::pImpl()->fileStats( (*FileName_).c_str(), Stats ) )
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
void CsDependency::updateStats()
{
	FsCore::pImpl()->fileStats( (*FileName_).c_str(), Stats_ );
}
