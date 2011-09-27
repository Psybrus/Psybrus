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

#include "OsCore.h"

#include "GaLibraryMath.h"
#include "GaLibraryGame.h"
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
// Filthy hack, move into OsCore.
#include "OsCoreImplSDL.h"

struct TKeyEnum
{
	const char* pEnumName_;
	int KeyCode_;
};

/*
const TKeyEnum GKeyEnums[] = 
{
	{ "A",			SDLK_a },
	{ "B",			SDLK_b },
	{ "C",			SDLK_c },
	{ "D",			SDLK_d },
	{ "E",			SDLK_e },
	{ "F",			SDLK_f },
	{ "G",			SDLK_g },
	{ "H",			SDLK_h },
	{ "I",			SDLK_i },
	{ "J",			SDLK_j },
	{ "K",			SDLK_k },
	{ "L",			SDLK_l },
	{ "M",			SDLK_m },
	{ "N",			SDLK_n },
	{ "O",			SDLK_o },
	{ "P",			SDLK_p },
	{ "Q",			SDLK_q },
	{ "R",			SDLK_r },
	{ "S",			SDLK_s },
	{ "T",			SDLK_t },
	{ "U",			SDLK_u },
	{ "V",			SDLK_v },
	{ "W",			SDLK_w },
	{ "X",			SDLK_x },
	{ "Y",			SDLK_y },
	{ "Z",			SDLK_z },
	{ "1",			SDLK_1 },
	{ "2",			SDLK_2 },
	{ "3",			SDLK_3 },
	{ "4",			SDLK_4 },
	{ "5",			SDLK_5 },
	{ "6",			SDLK_6 },
	{ "7",			SDLK_7 },
	{ "8",			SDLK_8 },
	{ "9",			SDLK_9 },
	{ "0",			SDLK_0 },
	{ "ESC",		SDLK_ESCAPE },
	{ "UP",			SDLK_UP },
	{ "DOWN",		SDLK_DOWN },
	{ "LEFT",		SDLK_LEFT },
	{ "RIGHT",		SDLK_RIGHT },
	{ "SPACE",		SDLK_SPACE },
};
 */

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
GaCore::GaCore():
	pGmMachine_( NULL )
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
	
	// Subscribe to input events.
	DelegateKey_ = OsEventInputKeyboard::Delegate::bind< GaCore, &GaCore::eventKey >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN,	DelegateKey_ );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP,		DelegateKey_ );
	
	// Reset game.
	reset();	
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
				BootScript_->execute( BcFalse );
				
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
			
			// Set new key state.
			for( BcU32 Idx = 0; Idx < 512; ++Idx )
			{
				pKeyStateMap_->Set( pGmMachine_, Idx, gmVariable( (int)KeyStates_[ Idx ] ) );
			}
			
			// Pass in last tick time.
			pGmMachine_->GetGlobals()->Set( pGmMachine_, "STAT_LASTTICK", gmVariable( SysSystem::lastTickTime() ) );
			
			// Execute VM for specified number of milliseconds.
			pGmMachine_->Execute( LastTick_ );
			
			// Check resource blocks.
			checkResourceBlocks();	

			// Set old new key state.
			for( BcU32 Idx = 0; Idx < 512; ++Idx )
			{
				pKeyOldStateMap_->Set( pGmMachine_, Idx, gmVariable( (int)KeyStates_[ Idx ] ) );
			}
			
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void GaCore::close()
{
	//
	OsCore::pImpl()->unsubscribe( osEVT_INPUT_KEYDOWN,	DelegateKey_ );
	OsCore::pImpl()->unsubscribe( osEVT_INPUT_KEYUP,	DelegateKey_ );

	// Destroy machine.
	delete pGmMachine_;
	pGmMachine_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// reset
void GaCore::reset()
{
	// Detach all components from entities.
	for( GaEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		(*It)->detachAllComponents();
	}
	
	// Remove all entities.
	EntityList_.clear();

	// Remove all resource blocks.
	ResourceBlocks_.clear();

	// Delete old machine.
	delete pGmMachine_;
	
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
	GaLibraryGameBinder( pGmMachine_ );
	GaLibrarySceneBinder( pGmMachine_ );
	
	// Allocate input tables.
	gmTableObject* pGlobalsTable = pGmMachine_->GetGlobals();
	pKeyEnumMap_ = pGmMachine_->AllocTableObject();
	pKeyStateMap_ = pGmMachine_->AllocTableObject();
	pKeyOldStateMap_ = pGmMachine_->AllocTableObject();
	pGlobalsTable->Set( pGmMachine_, "Keys", gmVariable( pKeyEnumMap_ ) );
	pGlobalsTable->Set( pGmMachine_, "KeyState", gmVariable( pKeyStateMap_ ) );
	pGlobalsTable->Set( pGmMachine_, "KeyOldState", gmVariable( pKeyOldStateMap_ ) );
	
	// Setup temporary key enums.
	/*
	BcU32 NoofKeyEnums = sizeof( GKeyEnums ) / sizeof( GKeyEnums[ 0 ] );
	for( BcU32 Idx = 0; Idx < NoofKeyEnums; ++Idx )
	{
		const TKeyEnum& KeyEnum = GKeyEnums[ Idx ];
		pKeyEnumMap_->Set( pGmMachine_, gmVariable( pGmMachine_->AllocStringObject( KeyEnum.pEnumName_ ) ), gmVariable( (int)KeyEnum.KeyCode_ ) );
	}
	 */
	
	// Setup execute stage;
	ExecuteStage_ = ES_BOOT;
	
	// Tick accumulator.
	TickAccumulator_ = 0.0f;
	
	// Clear key states.
	BcMemSet( &KeyStates_[ 0 ], 0, sizeof( KeyStates_ ) );
}

//////////////////////////////////////////////////////////////////////////
// executeScript
int GaCore::executeScript( const char* pScript, const char* pFileName, BcBool Now )
{
	int ThreadID = -1;
	pGmMachine_->ExecuteString( pScript, &ThreadID, Now, pFileName, NULL );
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

		//BcPrintf( "GaCore::Block thread %u as %s.%s is not ready.\n", Block.ThreadID_, Block.Resource_->getName().c_str(), Block.Resource_->getTypeString().c_str() );
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
			//BcPrintf( "GaCore::Unblock thread %u as %s.%s is ready.\n", Block.ThreadID_, Block.Resource_->getName().c_str(), Block.Resource_->getTypeString().c_str() );

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

#include "SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// eventKey
eEvtReturn GaCore::eventKey( BcU32 EvtID, const OsEventInputKeyboard& Event )
{
	if( Event.KeyCode_ < 512 )
	{
		switch( EvtID )
		{
			case osEVT_INPUT_KEYDOWN:
				KeyStates_[ Event.KeyCode_ ] = BcTrue;
				break;
			
			case osEVT_INPUT_KEYUP:
				KeyStates_[ Event.KeyCode_ ] = BcFalse;
				break;
		}
	}
	
	/*
	// HACK:
	if( Event.KeyCode_ == SDLK_ESCAPE )
	{
		pKernel()->stop();
	}

	// HACK:
	if( Event.KeyCode_ == SDLK_F1 )
	{
		reset();
	}
	 */

	return evtRET_PASS;
}
