/**************************************************************************
*
* File:		GaCore.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaCore_H__
#define __GaCore_H__

#include "BcTypes.h"

#include "BcGlobal.h"
#include "SysSystem.h"

#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// GaCore
class GaCore:
	public SysSystem,
	public BcGlobal< GaCore >
{
public:
	GaCore();
	virtual ~GaCore();
	
	virtual void				open();
	virtual void				update();
	virtual void				close();
	
	/**
	 * Execute script.
	 */
	void						executeScript( const char* pScript );
	
	/**
	 *	Add a resource block.
	 *	@return Has been blocked.
	 */
	BcBool						addResourceBlock( CsResourceRef<> Resource, class gmUserObject* pUserObject, class gmThread* pThread );
	
	/**
	 *	Check resource blocks.
	 */
	void						checkResourceBlocks();
		
private:
	class gmMachine*			pGmMachine_;
	BcU32						LastTick_;
	BcReal						TickAccumulator_;

	struct TResourceBlock
	{
		CsResourceRef<> Resource_;
		class gmUserObject* pGmUserObject_;
		int ThreadID_;
	};
	
	typedef std::list< TResourceBlock >  TResourceBlockList;
	typedef TResourceBlockList::iterator TResourceBlockListIterator;

	TResourceBlockList		ResourceBlocks_;

};

#endif


