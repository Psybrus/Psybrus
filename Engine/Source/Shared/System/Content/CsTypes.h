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
// Callbacks
typedef BcDelegate< void(*)( class CsPackage*, BcU32 ) > CsPackageReadyCallback;

//////////////////////////////////////////////////////////////////////////
// CsDependencyList
typedef std::list< class CsDependency > CsDependencyList;
typedef CsDependencyList::iterator CsDependencyListIterator;

//////////////////////////////////////////////////////////////////////////
// CsDependency
class CsDependency
{
public:
	CsDependency( const BcPath& FileName );
	CsDependency( const BcPath& FileName, const FsStats& Stats );
	CsDependency( const CsDependency& Other );
	~CsDependency();

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
