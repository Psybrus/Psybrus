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
CsDependency::CsDependency( const std::string& FileName )
{
	FileName_ = FileName;
	updateStats();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const std::string& FileName, const FsStats& Stats )
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
const std::string& CsDependency::getFileName() const
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
BcBool CsDependency::hasChanged() const
{
	FsStats Stats;
	if( FsCore::pImpl()->fileStats( FileName_.c_str(), Stats ) )
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
	FsCore::pImpl()->fileStats( FileName_.c_str(), Stats_ );
}

//////////////////////////////////////////////////////////////////////////
// operator < 
bool CsDependency::operator < ( const CsDependency& Dep ) const
{
	return FileName_ < Dep.FileName_;
}
