/**************************************************************************
*
* File:		SysSystem.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Base system.
*		
*
*
* 
**************************************************************************/

#ifndef __SysSystem_H__
#define __SysSystem_H__

#include "BcTypes.h"
#include "BcDebug.h"
#include "BcTimer.h"

#include "EvtPublisher.h"

#include "SysEvents.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SysSystem;
class SysKernel;

//////////////////////////////////////////////////////////////////////////
// SysSystemParams
struct SysSystemParams
{

};

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef SysSystem* (*SysSystemCreator)();

//////////////////////////////////////////////////////////////////////////
// Macros
#define SYS_REGFUNC( ClassName ) \
	sysCreate_##ClassName
	
#define SYS_REGFUNC_DECL( ClassName ) \
	SysSystem* SYS_REGFUNC( ClassName )() \

#define SYS_CREATOR( ClassName ) \
	SYS_REGFUNC_DECL( ClassName ) \
	{ \
		return new ClassName(); \
	}

#define SYS_REGISTER( Kernel, ClassName ) \
	extern SYS_REGFUNC_DECL( ClassName ); \
	(Kernel)->registerSystem( #ClassName, SYS_REGFUNC( ClassName ) );

//////////////////////////////////////////////////////////////////////////
/**	\class SysSystem
*	\brief Base system
*
*	All low level systems derive from this class, and will be updated
*	accordingly by the SysKernel. open(), update() & close() are guarenteed
*	to be called from the same thread.
*/
class SysSystem:
	public EvtPublisher
{
public:
	enum
	{
		MAX_SYSTEM_NAME = 32
	};
	
public:
	SysSystem();
	virtual ~SysSystem();

	/**
	*	Set kernel.
	*/
	void						pKernel( SysKernel* pKernel );
	
	/**
	*	Get kernel.
	*/
	SysKernel*					pKernel();
	
	/**
	*	Stop the system.
	*/
	void						stop();
	
	/**
	*	Is system finished?
	*/
	BcBool						isFinished() const;
	
	/**
	*	Process the system.
	*	@return BcTrue if still processing. BcFalse if finished.
	*/
	BcBool						process();
	
	/**
	*	Last tick time.
	*/
	BcReal						lastTickTime() const;

protected:

	/**
	*	Open system.
	*/
	virtual void				open() = 0;

	/**
	*	Update system.
	*/
	virtual void				update() = 0;

	/**
	*	Close system.
	*/
	virtual void				close() = 0;
	
private:
	/**
	* Process open.
	*/
	BcBool						processOpen();

	/**
	* Process update.
	*/
	BcBool						processUpdate();

	/**
	* Process open.
	*/
	BcBool						processClose();

	/**
	* Process finished.
	*/
	BcBool						processFinished();
	
private:

private:
	enum eProcessState
	{
		STATE_OPEN = 0,
		STATE_UPDATE,
		STATE_CLOSE,
		STATE_FINISHED,
		
		STATE_MAX
	};
	
	SysKernel*					pKernel_;								///!< Parent kernel.
	eProcessState				ProcessState_;							///!< State we're in.
	BcBool						StopTriggered_;							///!< Has a stop been triggered?
	
	typedef BcBool(SysSystem::*ProcessFunc)();
	typedef std::array< ProcessFunc, STATE_MAX > ProcessFuncArray;
	ProcessFuncArray			ProcessFuncs_;
	
	BcTimer						PerfTimer_;
	BcReal						LastTickTime_;
};


#endif

