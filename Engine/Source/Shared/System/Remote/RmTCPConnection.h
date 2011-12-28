/**************************************************************************
*
* File:		RmTCPConnection.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		Socket implementation.
*		
*
*
* 
**************************************************************************/

#ifndef __RMTCPCONNECTION_H__
#define __RMTCPCONNECTION_H__

#include "BcTypes.h"
#include "BcDebug.h"

////////////////////////////////////////////////////////////////////////////////
// RmTCPConnection
class RmTCPConnection
{
public:
	RmTCPConnection();
	~RmTCPConnection();
	
	/**
	 * Connect to a server.
	 * @param Address Address.
	 * @param Port Port.
	 */
	BcBool							connect( const BcChar* Address, BcU16 Port );
	
	/**
	 * Connect to a server on port range.
	 * @param Address Address.
	 * @param StartPort Start port.
	 * @param TotalPorts Total ports to try.
	 */
	BcBool							connectRange( const BcChar* Address, BcU16 StartPort, BcU16 TotalPorts );

	/**
	 * Listen on port.
	 * @param Address Address.
	 * @param Port Port.
	 */
	BcBool							listen( const BcChar* Address, BcU16 Port );
	
	/**
	 * Listen on port range.
	 * @param Address Address.
	 * @param StartPort Start port.
	 * @param TotalPorts Total ports to try.
	 */
	BcBool							listenRange( const BcChar* Address, BcU16 StartPort, BcU16 TotalPorts );

	/**
	 * Disconnect.
	 */
	void							disconnect();
	
	/**
	 * Send data.
	 * @param pData Data.
	 * @param Bytes Bytes to send.
	 * @return Success.
	 */
	BcBool							send( const void* pData, BcU32 Bytes );
	
	/**
	 * Receive data.
	 * @param pBuffer Buffer to read into.
	 * @param Bytes Bytes to read.
	 * @param Peek Peek at data?
	 * @param Block Block and wait for data?
	 * @return Bytes read.
	 */
	BcU32							recv( void* pBuffer, BcU32 Bytes, BcBool Peek, BcBool Block );
	
	/**
	 * Are we connected?
	 */
	BcBool							isConnected() const;
	
	/**
	 * Get port we're running on.
	 */
	BcU16							getPort() const;
	
	
private:
	BcBool							parseAddressInfo( const BcChar* Address, BcU16 Port );

private:
	/*
	BcBool							IsConnected_;
	BcU16							Port_;
	addrinfo*						ServInfo_;
	int								SocketDesc_;
	
	// Used when listening.
	sockaddr						RemoteInfo_;
	socklen_t						RemoteSize_;
	*/
};

BcForceInline BcBool RmTCPConnection::isConnected() const
{
	/*
	return IsConnected_;
	*/

	return BcFalse;
}

BcForceInline BcU16 RmTCPConnection::getPort() const
{
	/*
	return Port_;
	*/
	return 0;
}


#endif
