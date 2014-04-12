/**************************************************************************
*
* File:		DsCore.h
* Author:	Neil Richardson 
* Ver/Date:	11/12/12	
* Description:
*		Debug system core.
*		
*
*
* 
**************************************************************************/

#ifndef __DSCORE_H__
#define __DSCORE_H__

#include "Base/BcGlobal.h"
#include "Base/BcMutex.h"
#include "Base/BcScopedLock.h"
#include "System/SysSystem.h"

#include "System/Content/CsCore.h"
#include "System/Scene/ScnCore.h"

#include <mongoose.h>

//////////////////////////////////////////////////////////////////////////
/**	\class DsCore
*	\brief Debug System Core
*
*	Debugging system core.
*/
class DsCore:
	public BcGlobal< DsCore >,
	public SysSystem
{
public:
	static BcU32 WORKER_MASK;

public:
	DsCore();
	virtual ~DsCore();

	virtual void				open();
	virtual void				update();
	virtual void				close();

	void						cmdMenu( std::string& Output );

	void						cmdContent( std::string& Output );

	void						cmdContent_Resource( std::string& Output );

	void						cmdScene( std::string& Output );
	void						cmdScene_Entity( ScnEntityRef Entity, std::string& Output, BcU32 Depth);
	void						cmdScene_Component( ScnComponentRef Entity, std::string& Output, BcU32 Depth );

	void						cmdResource(std::string EntityId, std::string& Output);

	void						gameThreadMongooseCallback( enum mg_event Event, struct mg_connection* pConn );
	void*						mongooseCallback( enum mg_event Event, struct mg_connection* pConn );
	static void*				MongooseCallback( enum mg_event Event, struct mg_connection* pConn );

	void						writeHeader(std::string& Output);
	void						writeFooter(std::string& Output);
	BcU8*						writeFile(std::string filename, int& OutLength, std::string& type);
private:
	mg_context*					pContext_;
	BcMutex						Lock_;
	SysFence					GameThreadWaitFence_;
};


#endif
