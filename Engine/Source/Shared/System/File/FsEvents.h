/**************************************************************************
*
* File:		FsEvents.h
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FsEvents_H__
#define __FsEvents_H__

#include "EvtEvent.h"
#include "BcString.h"

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define FS_EVENTGROUP_CORE		EVT_MAKE_ID( 'F', 's', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum fsEvents
{
	// All core events.
	fsEVT_CORE_FIRST			= FS_EVENTGROUP_CORE,
	
	// File monitor.
	fsEVT_MONITOR_CREATED,
	fsEVT_MONITOR_MODIFIED,
	fsEVT_MONITOR_DELETED,
	
	//
	fsEVT_MAX
};

////////////////////////////////////////////////////////////////////////////////
// FsEventCore
struct FsEventCore: EvtEvent< FsEventCore >
{
	
};

////////////////////////////////////////////////////////////////////////////////
// FsEventMonitor
struct FsEventMonitor: EvtEvent< FsEventMonitor >
{
	FsEventMonitor();
	FsEventMonitor( const BcChar* FileName, const FsStats& OldFileStats, const FsStats& NewFileStats )
	{
		BcStrCopyN( FileName_, FileName, sizeof( FileName_ ) );
		OldFileStats_ = OldFileStats;
		NewFileStats_ = NewFileStats;
	}
	
	BcChar FileName_[ 512 ];
	FsStats OldFileStats_;
	FsStats NewFileStats_;
};

#endif


