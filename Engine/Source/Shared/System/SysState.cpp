/**************************************************************************
*
* File:		SysState.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			SubStates process first
*
*		
*		
* 
**************************************************************************/

#include "SysState.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysState::SysState():
	bEnterOnce_( BcTrue ),
	bPreMain_( BcTrue ),
	bPreLeave_( BcTrue ),
	InternalStage_( sysBS_ENTER ),
	InternalRenderStage_( sysBS_ENTER ),
	RequestedStage_( sysBS_ENTER ),
	pParent_( NULL ),
	pChild_( NULL ),
	pPendingChild_( NULL )
{
	// Set all sub states to null.
	for ( BcU32 i = 0; i < SubStates_.size(); ++i )
	{
		SubStates_[i] = NULL;
		PendingSubStates_[i] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysState::~SysState()
{
	// Delete all pending states.
	if( pPendingChild_ != NULL )
	{
		delete pPendingChild_;
	}

	for ( BcU32 i = 0; i < SubStates_.size(); ++i )
	{
		if( PendingSubStates_[ i ] != NULL )
		{
			delete PendingSubStates_[ i ];
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// enterOnce
//virtual
void SysState::enterOnce()
{

}

//////////////////////////////////////////////////////////////////////////
// preMain
//virtual
void SysState::preMain()
{

}

//////////////////////////////////////////////////////////////////////////
// mainBg
//virtual
eSysStateReturn SysState::mainChild()
{
	return sysSR_CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
// doneMainTasks
//virtual
BcBool SysState::doneMainTasks()
{
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// preLeave
//virtual
void SysState::preLeave()
{

}

//////////////////////////////////////////////////////////////////////////
// leaveOnce
//virtual
void SysState::leaveOnce()
{

}

//////////////////////////////////////////////////////////////////////////
// always
//virtual
void SysState::always()
{

}

//////////////////////////////////////////////////////////////////////////
// spawnState
void SysState::spawnChildState( SysState* pState )
{
	//
	if( pChild_ != NULL )
	{
		pChild_->leaveState();
	}

	if( pPendingChild_ == NULL )
	{
		pPendingChild_ = pState;
	}
	else
	{
		BcPrintf( "SysState: Trying to spawn \"%s\" over existing child state \"%s\" in \"%s\".\n", pState->name().c_str(), pPendingChild_->name().c_str(), name().c_str() );
		delete pPendingChild_;
		pPendingChild_ = pState;
	}

	pState->pParent( this );
}


//////////////////////////////////////////////////////////////////////////
// spawnSubState
void SysState::spawnSubState( BcU32 iSubState, SysState* pSubState )
{
	// Delete pending substate.
	if( SubStates_[ iSubState ] != NULL )
	{
		SubStates_[ iSubState ]->leaveState();
	}

	if( PendingSubStates_[ iSubState ] == NULL )
	{
		PendingSubStates_[ iSubState ] = pSubState;
	}
	else
	{
		BcPrintf( "SysState: Trying to spawn \"%s\" over existing sub state %u \"%s\" in \"%s\".\n", pSubState->name().c_str(), iSubState, PendingSubStates_[ iSubState ]->name().c_str(), name().c_str() );
		delete PendingSubStates_[ iSubState ];
		PendingSubStates_[ iSubState ] = pSubState;
	}

	pSubState->pParent( this );
}

//////////////////////////////////////////////////////////////////////////
// leaveState
void SysState::leaveState()
{
	// Leave our child state.
	if( pChild_ != NULL )
	{
		pChild_->leaveState();
	}

	// Leave our substates.
	leaveSubStates();

	// Set ourselves to leave.
	if( InternalStage_ < sysBS_LEAVE )
	{
		RequestedStage_ = sysBS_LEAVE;
	}
}

//////////////////////////////////////////////////////////////////////////
// leaveState
void SysState::leaveSubStates()
{
	if( InternalStage_ < sysBS_LEAVE )
	{
		for( BcU32 i = 0; i < SubStates_.size(); ++i )
		{
			if( SubStates_[i] != NULL )
			{
				SubStates_[i]->leaveState();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// process
BcBool SysState::process()
{
	BcBool bDone = BcTrue;

	//
	BcAssert( StateName_[ 0 ] != '\0' );

	// Process child.
	processChildState();

	// Process substates if we have no child.
	if( pChild_ == NULL )
	{
		processSubStates();
	}

	// Do always.
	always();

	// Set processed to false.
	ProcessedLastFrame_ = BcFalse;

	// Call internal stage functions
	switch( InternalStage_ )
	{
	case sysBS_ENTER:
		{
			// No child state allowed here.
			if( pChild_ == NULL )
			{
				ProcessedLastFrame_ = BcTrue;
				if( bEnterOnce_ == BcTrue )
				{
					// Perform construction
					enterOnce();
					bEnterOnce_ = BcFalse;
				}

				if ( enter() == sysSR_FINISHED )
				{
					if( RequestedStage_ == sysBS_LEAVE )
					{
						// If we have done main tasks, we can go straight to leave,
						// otherwise we need to hit main.
						if( doneMainTasks() )
						{
							RequestedStage_ = sysBS_LEAVE;
							InternalStage_ = sysBS_LEAVE;
						}
						else
						{
							RequestedStage_ = sysBS_LEAVE;
							InternalStage_ = sysBS_MAIN;
						}
					}
					else
					{
						RequestedStage_ = sysBS_MAIN;
						InternalStage_ = sysBS_MAIN;
					}
				}
			}
			bDone = BcFalse;
		}
		break;

	case sysBS_MAIN:
		{
			eSysStateReturn MainRet = sysSR_CONTINUE;

			if( pChild_ == NULL )
			{
				ProcessedLastFrame_ = BcTrue;
				if( bPreMain_ == BcTrue )
				{
					preMain();
					bPreMain_ = BcFalse;
				}

				MainRet = main();
			}
			else
			{
				if( bPreMain_ == BcFalse )
				{
					MainRet = mainChild();
				}
			}
			
			if ( MainRet == sysSR_FINISHED || ( RequestedStage_ > sysBS_MAIN && doneMainTasks() ) )
			{
				leaveState();

				InternalStage_ = sysBS_LEAVE;
			}

			bDone = BcFalse;
		}
		break;

	case sysBS_LEAVE:
		{
			if( nActiveSubStates_ == 0 && pChild_ == NULL )
			{
				ProcessedLastFrame_ = BcTrue;
				if( bPreLeave_ == BcTrue )
				{
					preLeave();
					bPreLeave_ = BcFalse;
				}

				if ( leave() == sysSR_FINISHED )
				{
					InternalStage_ = sysBS_FINISHED;
					InternalRenderStage_ = sysBS_FINISHED;

					// Perform destruction
					leaveOnce();
				}
			}

			bDone = BcFalse;
		}
		break;

	case sysBS_FINISHED:
		{
			bDone = BcTrue;
		}
		break;
	}
	
	return bDone;
}

//////////////////////////////////////////////////////////////////////////
// processChildState
void SysState::processChildState()
{	
	// Process current
	if ( pChild_ != NULL )
	{
		BcBool bRetVal = pChild_->process();

		if ( bRetVal == BcTrue )
		{
			delete pChild_;
			pChild_ = NULL;
		}
	}

	// Check pending.
	if( pChild_ == NULL )
	{
		if( pPendingChild_ != NULL )
		{
			if( InternalStage_ == sysBS_LEAVE || RequestedStage_ == sysBS_LEAVE )
			{
				delete pPendingChild_;
				pPendingChild_ = NULL;
			}

			pChild_ = pPendingChild_;
			pPendingChild_ = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// processSubStates
void SysState::processSubStates()
{	
	// Process Substates
	nActiveSubStates_ = 0;

	for ( BcU32 i = 0; i < SubStates_.size(); ++i )
	{
		// Process current
		if ( SubStates_[i] != NULL )
		{
			BcBool bRetVal = SubStates_[i]->process();
			++nActiveSubStates_;

			if ( bRetVal == BcTrue )
			{
				delete SubStates_[i];
				SubStates_[i] = NULL;
			}
		}

		// Check pending.
		if( SubStates_[i] == NULL )
		{
			if( PendingSubStates_[i] != NULL )
			{
				if( InternalStage_ == sysBS_LEAVE || RequestedStage_ == sysBS_LEAVE )
				{
					delete PendingSubStates_[i];
					PendingSubStates_[i] = NULL;
				}

				SubStates_[i] = PendingSubStates_[i];
				PendingSubStates_[i] = NULL;
			}
		}
	}
}

