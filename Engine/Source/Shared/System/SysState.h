/**************************************************************************
*
* File:		SysState.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			SubStates process first
*			Parents render first
*		
*		
* 
**************************************************************************/

#ifndef __SysState_H__
#define __SysState_H__

#include "BcTypes.h"
#include "BcDebug.h"
#include "BcName.h"
#include "BcString.h"
#include "BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// STAGE INC/DEC
#define INC_ENUMSTAGE(x) (++(*(int*)&x))
#define DEC_ENUMSTAGE(x) (--(*(int*)&x))

//////////////////////////////////////////////////////////////////////////
// Enums
enum eSysStateReturn
{
	sysSR_FINISHED = 0,
	sysSR_CONTINUE
};


//////////////////////////////////////////////////////////////////////////
// Internal Enums
enum eSysStateInternalStage
{
	sysBS_ENTER = 0,
	sysBS_MAIN,
	sysBS_LEAVE,
	sysBS_FINISHED
};

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_STATE_NAME			( 32 )
#define MAX_STATE_SUB_STATES	( 8 )

//////////////////////////////////////////////////////////////////////////
/**	\class SysState
*	\brief Stackable state machine.
*
*	A state machine which allows states to transition smoothly, as well
*	as stack on top of each other.
*
*	It is processed from the bottom to the top (children -> substates -> self), where as
*
*	The child state is an inhibiting state which prevents its parent and substates from being processed,
*	whereas substates run concurrently along side their parent state.
*
*	States have the ability to message each other, allowing communication without needing to throw
*	pointers around.
*/
class SysState
{
public:
	SysState();
	virtual ~SysState();

	/**
	*	Called once prior to entering the state.
	*/
	virtual void					enterOnce();

	/**
	*	Enter stage of the state.
	*	@return sysSR_FINISHED to move to next stage, sysSR_CONTINUE to continue in this stage.
	*/
	virtual eSysStateReturn			enter(){return sysSR_FINISHED;}

	/**
	*	Called once before main is ran. Same frame as first main.
	*/
	virtual void					preMain();

	/**
	*	Main stage of the state. Call of this is inhibited by a child state.
	*	@return sysSR_FINISHED to move to next stage, sysSR_CONTINUE to continue in this stage.
	*/
	virtual eSysStateReturn			main(){return sysSR_FINISHED;}

	/**
	*	Main stage of the state if a child exists. Call of this is inhibited by lack of a child state.
	*	@return sysSR_FINISHED to move to next stage, sysSR_CONTINUE to continue in this stage.
	*/
	virtual eSysStateReturn			mainChild();

	/**
	*	Called if a leave is requested.
	*	@return BcTrue if it can move onto the leave stage.
	*/
	virtual BcBool					doneMainTasks();

	/**
	*	Called once before leave is ran. Same frame as first leave.
	*/
	virtual void					preLeave();

	/**
	*	Leave stage of the state.
	*	@return sysSR_FINISHED to move to next stage, sysSR_CONTINUE to continue in this stage.
	*/
	virtual eSysStateReturn			leave(){return sysSR_FINISHED;}

	/**
	*	Called once after the leave returns sysSR_FINISHED.
	*/
	virtual void					leaveOnce();

	/**
	*	Called every frame with normal stages.
	*/
	virtual void					always();

	/**
	*	Set the state name.
	*	@param StateName State name.
	*/
	void							name( BcName StateName );

	/**
	*	Get the state name.
	*	@return State name.
	*/
	BcName							name() const;

	/**
	*	Spawn a child state from this state. Blocking.
	*	@param pState Pointer to state.
	*/
	void							spawnChildState( SysState* pState );

	/**
	*	Spawn a substate state from this state. Non-blocking.
	*	@param Slot Substate slot.
	*	@param pSubState Pointer to state.
	*/
	void							spawnSubState( BcU32 Slot, SysState* pSubState );

	/**
	*	Leave this state. Will cause child and substates to leave first.
	*/
	void							leaveState();

	/**
	*	Get our parent state.
	*	@return Pointer to parent state.
	*/
	SysState*						pParent(){ return pParent_; }

	/**
	*	Get a substate.
	*	@param Slot Substate slot.
	*	@return Pointer to substate. NULL for empty slot.
	*/
	SysState*						pSubState( BcU32 Slot ){ BcAssert( Slot < MAX_STATE_SUB_STATES ); return SubStates_[ Slot ]; }

	/**
	*	Get child state.
	*	@return Pointer to child state. NULL for none.
	*/
	SysState*						pChild(){ return pChild_; }

	/**
	*	Get internal stage. 
	*/
	eSysStateInternalStage 			internalStage();

	/**
	*	Process the state. PSYBRUS USE ONLY.
	*/
	BcBool							process();

private:
	void							pParent( SysState* pParent ){ pParent_ = pParent; }
	void							leaveSubStates();
	void							processChildState();
	void							processSubStates();


private:

	//
	BcName							StateName_;

	// Once shot calls.
	BcBool							bEnterOnce_;
	BcBool							bPreMain_;
	BcBool							bPreLeave_;

	// Internal stages + logic.
	eSysStateInternalStage			InternalStage_;
	eSysStateInternalStage			InternalRenderStage_;
	eSysStateInternalStage			RequestedStage_;
	BcU32							nActiveSubStates_;
	SysState*						pParent_;
	SysState*						pChild_;
	SysState*						pPendingChild_;

	// Child and substates
	SysState*						SubStates_[ MAX_STATE_SUB_STATES ];
	SysState*						PendingSubStates_[ MAX_STATE_SUB_STATES ];
	
	// Proccessing.
	BcBool							ProcessedLastFrame_;


};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline void SysState::name( BcName StateName )
{
	StateName_ = StateName;
}

inline BcName SysState::name() const
{
	return StateName_;
}

inline eSysStateInternalStage SysState::internalStage()
{
	return InternalStage_;
}

#endif

