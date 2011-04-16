/**************************************************************************
*
* File:		RmCore.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RMCORE_H__
#define __RMCORE_H__

#include "SysSystem.h"

#include "BcDelegate.h"
#include "BcEndian.h"
#include "BcGlobal.h"
#include "BcHash.h"
#include "BcMutex.h"
#include "BcScopedLock.h"

#include "RmEvents.h"
#include "RmTCPConnection.h"
#include "RmParameters.h"

////////////////////////////////////////////////////////////////////////////////
// RmDelegate
typedef BcDelegate< void(*)( void*, BcU32 ) > RmDelegate;

////////////////////////////////////////////////////////////////////////////////
// RmCore
class RmCore:
	public BcGlobal< RmCore >,
	public SysSystem
{
public:
	RmCore();
	virtual ~RmCore();
	
	/**
	 * Register a delegate.
	 * @param UniqueID Unique ID for delegate.
	 * @param Delegate Delegate to be called.
	 */
	void							registerDelegate( BcU32 UniqueID, RmDelegate Delegate );
	
	/**
	 * Unregister a delegate.
	 * @param UniqueID Unique ID for delegate.
	 */
	void							unregisterDelegate( BcU32 UniqueID );
	
	/**
	 * Send a data to a delegate.
	 * @param UniqueID Delegate to receive.
	 * @param pData Data to send.
	 * @param Bytes Bytes to send.
	 * @param Success.
	 */
	BcBool							send( BcU32 UniqueID, void* pData, BcU32 Bytes );

	/**
	 * Send a data to a delegate.
	 * @param UniqueID Delegate to receive.
	 * @param Params Parameter object.
	 * @param Success.
	 */
	BcBool							send( BcU32 UniqueID, RmParameters& Params );

	/**
	 * Connect to another RmCore.
	 * @param pAddress Address.
	 */
	void							connect( const BcChar* pAddress );
	
	/**
	 * Listen for another RmCore.
	 */
	void							listen();
	
	/*
	 * Is connected?
	 */
	BcBool							isConnected() const;

protected:
	virtual void					open();
	virtual void					update();
	virtual void					close();
	
protected:
	struct TMessageHeader
	{
		TMessageHeader()
		{
			Destination_ = 0;
			Bytes_ = 0;
		}
		
		TMessageHeader( BcU32 Destination, BcU32 Bytes, BcU32 Hash )
		{
			Destination_ = Destination;
			Bytes_ = Bytes;
			Hash_ = Hash;
		}
		
		BcEndianU32 Destination_;
		BcEndianU32 Bytes_;
		BcEndianU32 Hash_;
	};
	
	struct TMessage
	{
		TMessage( const TMessageHeader& Header, void* pData )
		{
			Header_ = Header;
			pData_ = pData;
		}
		
		TMessageHeader Header_;
		void* pData_;
	};
	
	void							dispatch( const TMessageHeader& Header, void* pData );
	void							keepAlive();

private:	
	RmTCPConnection					Connection_;
	BcBool							ConnectToggle_;
	
	typedef std::map< BcU32, RmDelegate > TDelegateMap;
	typedef TDelegateMap::iterator TDelegateMapIterator;
	TDelegateMap					DelegateMap_;
	
	typedef std::list< TMessage > TMessageList;
	typedef TMessageList::iterator TMessageListIterator;
	TMessageList					MessageList_;
	
	BcMutex							ConnectionLock_;
	BcMutex							ContainerLock_;
	
	BcU32							KeepAliveTicker_;
};

#endif
