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
	ReField* Fields[] = 
	{
		new ReField( "Seconds_",		&FsTimestamp::Seconds_ ),
		new ReField( "Minutes_",		&FsTimestamp::Minutes_ ),
		new ReField( "Hours_",			&FsTimestamp::Hours_ ),
		new ReField( "MonthDay_",		&FsTimestamp::MonthDay_ ),
		new ReField( "Month_",			&FsTimestamp::Month_ ),
		new ReField( "Year_",			&FsTimestamp::Year_ ),
		new ReField( "WeekDay_",		&FsTimestamp::WeekDay_ ),
		new ReField( "YearDay_",		&FsTimestamp::YearDay_ ),
		new ReField( "IsDST_",			&FsTimestamp::IsDST_ ),
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
	ReField* Fields[] = 
	{
		new ReField( "CreatedTime_",	&FsStats::CreatedTime_ ),
		new ReField( "ModifiedTime_",	&FsStats::ModifiedTime_ ),
	};

	ReRegisterClass< FsStats >( Fields );
}

FsStats::FsStats()
{

}
