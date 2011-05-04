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
#include "OsEvents.h"
#include "SysSystem.h"

#include "CsResourceRef.h"

#include "ScnScript.h"

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
	
	/*
	 * Reset game.
	 */
	void						reset();
	
	/**
	 * Execute script.
	 */
	int							executeScript( const char* pScript, const char* pFileName, BcBool Now );
	
	/**
	 *	Add a resource block.
	 *	@return Has been blocked.
	 */
	BcBool						addResourceBlock( CsResource* pResource, class gmUserObject* pUserObject, class gmThread* pThread );
	
	/**
	 *	Check resource blocks.
	 */
	void						checkResourceBlocks();
		
private:
	/**
	 * Keyboard input.
	 */
	eEvtReturn					eventKey( BcU32 EvtID, const OsEventInputKeyboard& Event );
	
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

	// NEILO: Temporary for LD.
	enum eExecuteStage
	{
		ES_BOOT = 0,
		ES_WAIT,
		ES_EXECUTE,
	};
	
	ScnScriptRef			BootScript_;
	eExecuteStage			ExecuteStage_;
	
	// NEILO: Temporary for LD.
	class gmTableObject*	pKeyEnumMap_;
	class gmTableObject*	pKeyStateMap_;
	class gmTableObject*	pKeyOldStateMap_;
	BcBool					KeyStates_[ 512 ];

	// Input delegates.
	OsEventInputKeyboard::Delegate DelegateKey_; 
	OsEventInputMouse::Delegate DelegateMouse_;
	
};

#endif


