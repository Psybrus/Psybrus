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

//////////////////////////////////////////////////////////////////////////
// FsTimestamp
struct FsTimestamp
{
	FsTimestamp():
		Seconds_( 0 ),
		Minutes_( 0 ),
		Hours_( 0 ),
		MonthDay_( 0 ),
		Month_( 0 ),
		Year_( 0 ),
		WeekDay_( 0 ),
		YearDay_( 0 ),
		IsDST_( 0 )
	{
		
	}
	
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
	FsTimestamp CreatedTime_;
	FsTimestamp ModifiedTime_;
};

#endif


