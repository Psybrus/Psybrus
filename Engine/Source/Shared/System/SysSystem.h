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

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcName.h"
#include "Base/BcTimer.h"
#include "Base/BcMisc.h"

#include "Events/EvtPublisher.h"

#include "System/SysEvents.h"
#include "System/SysFence.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SysSystem;
class SysKernel;

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

#define SYS_REGISTER( ImplName, ClassName ) \
	extern SYS_REGFUNC_DECL( ClassName ); \
	SysKernel::pImpl()->registerSystem( ImplName, SYS_REGFUNC( ClassName ) );

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
	*	Is system opened?
	*/
	BcBool						isOpened() const;

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
	BcF32						lastTickTime() const;

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
	friend class SysKernel;

	void						setName( const BcName& Name );

private:
	enum eProcessState
	{
		STATE_OPEN = 0,
		STATE_UPDATE,
		STATE_CLOSE,
		STATE_FINISHED,
		
		STATE_MAX
	};
	
	BcName						Name_;
	SysKernel*					pKernel_;								///!< Parent kernel.
	eProcessState				ProcessState_;							///!< State we're in.
	BcBool						StopTriggered_;							///!< Has a stop been triggered?
	
	typedef BcBool(SysSystem::*ProcessFunc)();
	ProcessFunc					ProcessFuncs_[ STATE_MAX ];
	
	BcTimer						PerfTimer_;
	BcF32						LastTickTime_;
};


#endif

