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

#include <functional>
#include <map>

#if USE_WEBBY
#include "webby.h"
#endif // USE_WEBBY

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
	static BcU32 JOB_QUEUE_ID;

public:
	DsCoreImpl();
	virtual ~DsCoreImpl();

	virtual void				open();
	virtual void				update();
	virtual void				close();

private:
//	BcMutex						Lock_;
	SysFence					GameThreadWaitFence_;


#if USE_WEBBY
	// Variables for Webby
	int							ConnectionCount_;
	WebbyServer *				Server_;
	void*						ServerMemory_;
#ifndef MAX_WSCONN
#define MAX_WSCONN 8
#endif
	WebbyConnection*			ws_connections[MAX_WSCONN];


	static int					externalWebbyDispatch(WebbyConnection *connection);
	static int					externalWebbyConnect(WebbyConnection *connection);
	static void					externalWebbyConnected(WebbyConnection *connection);
	static void					externalWebbyClosed(WebbyConnection *connection);
	static int					externalWebbyFrame(WebbyConnection *connection, const WebbyWsFrame *frame);

	int							webbyDispatch(WebbyConnection *connection);
	int							webbyConnect(WebbyConnection *connection);
	void						webbyConnected(WebbyConnection *connection);
	void						webbyClosed(WebbyConnection *connection);
	int							webbyFrame(WebbyConnection *connection, const WebbyWsFrame *frame);
#endif // USE_WEBBY

};



#endif
