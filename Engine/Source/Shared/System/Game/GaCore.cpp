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

#include "GaLibraryMath.h"
#include "GaLibraryScene.h"

#include "gmMachine.h"
#include "gmThread.h"
#include "gmUserObject.h"

#include "gmMachineLib.h"
#include "gmArrayLib.h"
#include "gmListLib.h"
#include "gmStringLib.h"
#include "gmMathLib.h"

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
				BcPrintf( "%s", Entry );
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
	
	// Setup memory limits (7MB initial soft limit, 8MB hard limit, auto adjust).
	pGmMachine_->SetDesiredByteMemoryUsageSoft( 7 * 1024 * 1024 );
	pGmMachine_->SetDesiredByteMemoryUsageHard( 8 * 1024 * 1024 );
	pGmMachine_->SetAutoMemoryUsage( true );
	
	// Set debug mode.
	pGmMachine_->SetDebugMode( true );

	// Bind gm libs.
	gmMachineLib( pGmMachine_ );
	gmBindArrayLib( pGmMachine_ );
	gmBindListLib( pGmMachine_ );
	gmBindStringLib( pGmMachine_ );
	gmBindMathLib( pGmMachine_ );
		
	// Bind Psybrus libs.
	GaLibraryMathBinder( pGmMachine_ );
	GaLibrarySceneBinder( pGmMachine_ );
	
	// Setup execute stage;
	ExecuteStage_ = ES_BOOT;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaCore::update()
{
	switch( ExecuteStage_ )
	{
		case ES_BOOT:
		{
			// Request the boot script.
			if( CsCore::pImpl()->requestResource( "boot", BootScript_ ) )
			{
				ExecuteStage_ = ES_WAIT;
			}
			break;
		}
			
		case ES_WAIT:
		{
			// When boot script is ready, start executing.
			if( BootScript_.isReady() )
			{
				BootScript_->execute();
				
				ExecuteStage_ = ES_EXECUTE;
			}			
			break;
		}

		case ES_EXECUTE:
		{
			// Calculate tick.
			TickAccumulator_ += 1000.0f / 60.0f; // TOOD: Get from timer..	
			LastTick_ = (BcU32)( BcFloor( TickAccumulator_ ) );
			TickAccumulator_ -= (BcReal)LastTick_;
			
			// Execute VM for specified number of milliseconds.
			pGmMachine_->Execute( LastTick_ );
			
			// Check resource blocks.
			checkResourceBlocks();				
			break;
		}
	}
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
void GaCore::reset()
{
	// Reset machine.
	pGmMachine_->ResetAndFreeMemory();

	// Remove all resource blocks.
	ResourceBlocks_.clear();
}

//////////////////////////////////////////////////////////////////////////
// executeScript
int GaCore::executeScript( const char* pScript, const char* pFileName )
{
	int ThreadID = -1;
	pGmMachine_->ExecuteString( pScript, &ThreadID, false, pFileName, NULL );
	return ThreadID;
}

//////////////////////////////////////////////////////////////////////////
// addResourceBlock
BcBool GaCore::addResourceBlock( CsResource* pResource, class gmUserObject* pGmUserObject, class gmThread* pGmThread )
{
	BcBool IsBlocked = BcFalse;
	TResourceBlock Block;
	Block.Resource_ = pResource;
	Block.pGmUserObject_ = pGmUserObject;
	Block.ThreadID_ = pGmThread->GetId();
	
	// If resource is valid but not ready, queue it up.
	if( pResource != NULL && pResource->isReady() == BcFalse )
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