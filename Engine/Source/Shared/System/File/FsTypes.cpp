/**************************************************************************
*
* File:		FsTypes.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsTypes.h"

//////////////////////////////////////////////////////////////////////////
// FsTypes
REFLECTION_DEFINE_BASIC( FsTimestamp )

void FsTimestamp::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Seconds_",		&FsTimestamp::Seconds_ ),
		ReField( "Minutes_",		&FsTimestamp::Minutes_ ),
		ReField( "Hours_",			&FsTimestamp::Hours_ ),
		ReField( "MonthDay_",		&FsTimestamp::MonthDay_ ),
		ReField( "Month_",			&FsTimestamp::Month_ ),
		ReField( "Year_",			&FsTimestamp::Year_ ),
		ReField( "WeekDay_",		&FsTimestamp::WeekDay_ ),
		ReField( "YearDay_",		&FsTimestamp::YearDay_ ),
		ReField( "IsDST_",			&FsTimestamp::IsDST_ ),
	};

	ReRegisterClass< FsTimestamp >( Fields );
}

FsTimestamp::FsTimestamp():
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

//////////////////////////////////////////////////////////////////////////
// FsStats
REFLECTION_DEFINE_BASIC( FsStats )

void FsStats::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "CreatedTime_",	&FsStats::CreatedTime_ ),
		ReField( "ModifiedTime_",	&FsStats::ModifiedTime_ ),
	};

	ReRegisterClass< FsStats >( Fields );
}

FsStats::FsStats()
{

}
