/**************************************************************************
*
* File:		GaCore.cpp
* Author:	Neil Richardson 
* Ver/Date:	21/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaCore.h"

#include "GaLibraryScene.h"

#include "gmMachine.h"
#include "gmThread.h"
#include "gmUserObject.h"

#include "gmMachineLib.h"

SYS_CREATOR( GaCore );

//////////////////////////////////////////////////////////////////////////
// Game monkey callbacks.
void GM_CDECL GmCallback_Print( gmMachine* pMachine, const char* pString )
{
	BcPrintf( pString );
}

bool GM_CDECL GmCallback_Machine(gmMachine * a_machine, gmMachineCommand a_command, const void * a_context)
{
	switch( a_command )
	{
		case MC_THREAD_EXCEPTION:
		{
			gmLog& GmLog = a_machine->GetLog();
			bool First = true;
			const char* Entry = NULL;
			while( Entry = GmLog.GetEntry( First ) ) // Yes this is an assign. It's intended!
			{
				BcPrintf( "GmLog: %s\n", Entry );
			}
		}
		break;
			
		default: 
			break;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// Ctor
GaCore::GaCore()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaCore::~GaCore()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void GaCore::open()
{
	// Setup callbacks.
	gmMachine::s_printCallback = GmCallback_Print;
	gmMachine::s_machineCallback = GmCallback_Machine;
	
	// Create machine.
	pGmMachine_ = new gmMachine();

	// Bind gm libs.
	gmMachineLib( pGmMachine_ );
	
	// Bind Psybrus libs.
	GaLibrarySceneBinder( pGmMachine_ );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaCore::update()
{
	// Calculate tick.
	TickAccumulator_ += 1000.0f / 60.0f; // TOOD: Get from timer..	
	LastTick_ = (BcU32)( BcFloor( TickAccumulator_ ) );
	TickAccumulator_ -= (BcReal)LastTick_;

	// Execute VM for specified number of milliseconds.
	pGmMachine_->Execute( LastTick_ );
	
	// Check resource blocks.
	checkResourceBlocks();	
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void GaCore::close()
{
	// Destroy machine.
	delete pGmMachine_;
	pGmMachine_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// executeScript
void GaCore::executeScript( const char* pScript )
{
	gmVariable ThisVar( pGmMachine_->GetGlobals() );

	pGmMachine_->ExecuteString( pScript, NULL, true, "unknown", &ThisVar );
}

//////////////////////////////////////////////////////////////////////////
// addResourceBlock
BcBool GaCore::addResourceBlock( CsResourceRef<> Resource, class gmUserObject* pGmUserObject, class gmThread* pGmThread )
{
	BcBool IsBlocked = BcFalse;
	TResourceBlock Block;
	Block.Resource_ = Resource;
	Block.pGmUserObject_ = pGmUserObject;
	Block.ThreadID_ = pGmThread->GetId();
	
	// If resource is not ready, but also valid, put into queue.
	if( Resource.isReady() == BcFalse && Resource.isValid() == BcTrue )
	{
		// Block thread.
		gmVariable SignalVar( pGmUserObject );
		pGmMachine_->Sys_Block( pGmThread, 1, &SignalVar );
		
		// Put into list.
		ResourceBlocks_.push_back( Block );
		IsBlocked = BcTrue;
	}
	
	return IsBlocked;
}

//////////////////////////////////////////////////////////////////////////
// checkResourceBlocks
void GaCore::checkResourceBlocks()
{
	TResourceBlockListIterator Iter = ResourceBlocks_.begin();
	
	while( Iter != ResourceBlocks_.end() )
	{
		TResourceBlock Block = (*Iter);
		
		// If the package is ready, signal and advance.
		if( Block.Resource_.isReady() == BcTrue || Block.Resource_.isValid() == BcFalse )
		{
			gmVariable SignalVar( Block.pGmUserObject_ );
			pGmMachine_->Signal( SignalVar, Block.ThreadID_, NULL );
			Iter = ResourceBlocks_.erase( Iter );
		}
		else
		{
			++Iter;
		}
	}
}