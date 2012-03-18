/**************************************************************************
*
* File:		CsTypes.h
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSTYPES_H__
#define __CSTYPES_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcName.h"
#include "Base/BcPath.h"

#include "System/File/FsFile.h"
#include "Base/BcAtomic.h"
#include "Base/BcEndian.h"
#include "Base/BcHash.h"
#include "Base/BcMutex.h"
#include "Base/BcScopedLock.h"

//////////////////////////////////////////////////////////////////////////
// CsDependancyList
typedef std::list< class CsDependancy > CsDependancyList;
typedef CsDependancyList::iterator CsDependancyListIterator;

//////////////////////////////////////////////////////////////////////////
// CsDependancy
class CsDependancy
{
public:
	CsDependancy( const BcPath& FileName );
	CsDependancy( const BcPath& FileName, const FsStats& Stats );
	CsDependancy( const CsDependancy& Other );
	~CsDependancy();

	/**
	 * Get file name.
	 */
	const BcPath& getFileName() const;

	/**
	 * Get stats.
	 */
	const FsStats& getStats() const;

	/**
	 * Had dependancy changed?
	 */
	BcBool hasChanged();

	/**
	 * Update stats.
	 */
	void updateStats();


private:
	BcPath FileName_;
	FsStats Stats_;
};

#endif
