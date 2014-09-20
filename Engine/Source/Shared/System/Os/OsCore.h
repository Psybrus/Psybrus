/**************************************************************************
*
* File:		OsCore.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSCORE_H__
#define __OSCORE_H__

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

#include "OsEvents.h"
#include "OsClient.h"

//////////////////////////////////////////////////////////////////////////
// OsCore
class OsCore:
	public BcGlobal< OsCore >,
	public SysSystem
{
public:
	OsCore();
	virtual ~OsCore();

	/**
	 * Update.
	 */
	virtual void update();

	/**
	 * Register client.
	 */
	void registerClient( OsClient* pClient );

	/**
	 * Unregister client.
	 */
	void unregisterClient( OsClient* pClient );

	/**
 	 * Get number of clients.
	 */
	size_t getNoofClients() const;

	/**
	 * Get client.
	 */
	OsClient* getClient( size_t Index );

private:
	typedef std::vector< OsClient* > TClientList;
	typedef TClientList::iterator TClientListIterator;

	TClientList ClientList_;

};

#endif

