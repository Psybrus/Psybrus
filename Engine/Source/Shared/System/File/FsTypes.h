/**************************************************************************
*
* File:		FsTypes.h
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FsTypes_H__
#define __FsTypes_H__

#include "Base/BcTypes.h"
#include "Reflection/ReReflection.h"

#include <functional>

//////////////////////////////////////////////////////////////////////////
// eFsFileMode
enum eFsFileMode
{
	fsFM_READ = 0,
	fsFM_WRITE,
};

//////////////////////////////////////////////////////////////////////////
// File operation delegate
typedef std::function< void( void*, BcSize ) > FsFileOpCallback;

//////////////////////////////////////////////////////////////////////////
// FsTimestamp
struct FsTimestamp
{
	REFLECTION_DECLARE_BASIC( FsTimestamp )

	FsTimestamp();
	
	BcU32 Seconds_;
	BcU32 Minutes_;
	BcU32 Hours_;
	BcU32 MonthDay_;
	BcU32 Month_;
	BcU32 Year_;
	BcU32 WeekDay_;
	BcU32 YearDay_;
	BcU32 IsDST_;
	
	inline bool operator == ( const FsTimestamp& Other ) const
	{
		return ( Seconds_ == Other.Seconds_ ) &&
		       ( Minutes_ == Other.Minutes_ ) &&
		       ( Hours_  == Other.Hours_ ) &&
		       ( MonthDay_ == Other.MonthDay_ ) &&
		       ( Month_ == Other.Month_ ) &&
		       ( Year_ == Other.Year_ ) &&
		       ( WeekDay_ == Other.WeekDay_ ) &&
		       ( YearDay_ == Other.YearDay_ ) &&
		       ( IsDST_ == Other.IsDST_ );
	}

	inline bool operator != ( const FsTimestamp& Other ) const
	{
		return ( Seconds_ != Other.Seconds_ ) ||
		       ( Minutes_ != Other.Minutes_ ) ||
		       ( Hours_  != Other.Hours_ ) ||
		       ( MonthDay_ != Other.MonthDay_ ) ||
		       ( Month_ != Other.Month_ ) ||
		       ( Year_ != Other.Year_ ) |
		       ( WeekDay_ != Other.WeekDay_ ) ||
		       ( YearDay_ != Other.YearDay_ ) ||
		       ( IsDST_ != Other.IsDST_ );
	}
	
	inline bool isNull() const
	{
		return (*this) == FsTimestamp();
	}
};

//////////////////////////////////////////////////////////////////////////
// FsStats
struct FsStats
{
	REFLECTION_DECLARE_BASIC( FsStats );

	FsStats();

	FsTimestamp CreatedTime_;
	FsTimestamp ModifiedTime_;
};

#endif


