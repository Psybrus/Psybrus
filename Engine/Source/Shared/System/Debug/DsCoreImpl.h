/**************************************************************************
*
* File:		DsCoreImpl.h
* Author:	Neil Richardson 
* Ver/Date:	11/12/12	
* Description:
*		Debug system core.
*		
*
*
* 
**************************************************************************/

#ifndef __DSCOREIMPL_H__
#define __DSCOREIMPL_H__

#include "Base/BcGlobal.h"
#include "Base/BcHtml.h"
#include "Base/BcRegex.h"
#include "System/SysSystem.h"

#include "System/Content/CsCore.h"
#include "System/Scene/ScnCore.h"
#include "DsCore.h"

#include <mongoose.h>
#include <functional>
#include <map>

//////////////////////////////////////////////////////////////////////////
/**	\class DsCore
*	\brief Debug System Core
*
*	Debugging system core.
*/
class DsCoreImpl :
	public DsCore
{
public:
	static BcU32 WORKER_MASK;

public:
	DsCoreImpl();
	virtual ~DsCoreImpl();

	virtual void				open();
	virtual void				update();
	virtual void				close();

	void						gameThreadMongooseCallback( enum mg_event Event, struct mg_connection* pConn );
	void*						mongooseCallback( enum mg_event Event, struct mg_connection* pConn );
	static void*				MongooseCallback( enum mg_event Event, struct mg_connection* pConn );



private:
	mg_context*					pContext_;
	std::mutex					Lock_;
	SysFence					GameThreadWaitFence_;

};


#endif
