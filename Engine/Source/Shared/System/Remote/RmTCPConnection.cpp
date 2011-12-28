/**************************************************************************
*
* File:		RmTCPConnection.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		Berkeley socket implementation.
*		
*
*
* 
**************************************************************************/

#include "RmTCPConnection.h"

#include "BcRandom.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RmTCPConnection::RmTCPConnection()
{
	/*
	ServInfo_ = NULL;
	SocketDesc_ = BcErrorCode;
	IsConnected_ = BcFalse;
	Port_ = 0;

#if PLATFORM_WINDOWS
	static BcBool IsInitialised = BcFalse;
	if( IsInitialised == BcFalse )
	{
		WSADATA WsaData;
		::WSAStartup( MAKEWORD(2,2), &WsaData );
		IsInitialised = BcTrue;
	}
#endif
	*/
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
RmTCPConnection::~RmTCPConnection()
{
	/*
	disconnect();
	*/
}

////////////////////////////////////////////////////////////////////////////////
// connect
BcBool RmTCPConnection::connect( const BcChar* Address, BcU16 Port )
{
	/*
	int ReturnCode = 0;

	// Disconnect first.
	if( ServInfo_ != NULL || SocketDesc_ != BcErrorCode )
	{
		disconnect();
	}
	
	// Parse address info for connection.
	if( parseAddressInfo( Address, Port ) )
	{
		SocketDesc_ = socket( ServInfo_->ai_family, ServInfo_->ai_socktype, ServInfo_->ai_protocol );
		if( ( ReturnCode = ::connect( SocketDesc_, ServInfo_->ai_addr, sizeof( sockaddr ) ) ) >= 0 )
		{
			IsConnected_ = BcTrue;
			Port_ = Port;
			return BcTrue;
		}
	}
	
	// Clean up.
	disconnect();
	return BcFalse;
	*/
	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// connectRange
BcBool RmTCPConnection::connectRange( const BcChar* Address, BcU16 StartPort, BcU16 TotalPorts )
{
	/*
	BcBool Success = BcFalse;
	
	do
	{
		for( BcU16 Port = StartPort; Port < ( StartPort + TotalPorts ); ++Port )
		{
			Success = connect( Address, Port );
			if( Success == BcTrue )
			{
				break;
			}
		}
	}
	while( Success == BcFalse );
	
	return Success;
	*/
	return BcFalse;
}	

////////////////////////////////////////////////////////////////////////////////
// listen
BcBool RmTCPConnection::listen( const BcChar* Address, BcU16 Port )
{
	/*
	// Disconnect first.
	if( ServInfo_ != NULL || SocketDesc_ != BcErrorCode )
	{
		disconnect();
	}
		
	// Parse address info for connection.
	if( parseAddressInfo( Address, Port ) )
	{
		SocketDesc_ = socket( ServInfo_->ai_family, ServInfo_->ai_socktype, ServInfo_->ai_protocol );
		
		BcPrintf( "RmTCPConnection: Listening on connection on port %u\n", Port );

		::bind( SocketDesc_, ServInfo_->ai_addr, ServInfo_->ai_addrlen );
		::listen( SocketDesc_, 1 );
		
		RemoteSize_ = sizeof( RemoteInfo_ );
		int NewSocketDesc = ::accept( SocketDesc_, &RemoteInfo_, &RemoteSize_ );
		
		if( SocketDesc_ >= 0 )
		{
			::closesocket( SocketDesc_ ); // No more incoming connections.
			SocketDesc_ = NewSocketDesc;
			IsConnected_ = BcTrue;
			Port_ = Port;
			return BcTrue;
		}
		
		BcPrintf( "RmTCPConnection: Can't listen on %s:%u\n", Address, Port );
	}
	
	// Clean up.
	disconnect();
	return BcFalse;
	*/
	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// listenRange
BcBool RmTCPConnection::listenRange( const BcChar* Address, BcU16 StartPort, BcU16 TotalPorts )
{
	/*
	BcBool Success = BcFalse;
	
	BcU16 InitialPort = 4 % TotalPorts;
	for( BcU16 Idx = 0; Idx < TotalPorts; ++Idx )
	{
		BcU16 Port = StartPort + ( ( InitialPort + Idx ) % TotalPorts );
		Success = listen( Address, Port );
		if( Success == BcTrue )
		{
			break;
		}

	}
	
	return Success;
	*/
	return BcFalse;
}	

////////////////////////////////////////////////////////////////////////////////
// disconnect
void RmTCPConnection::disconnect()
{
	/*
	if( ServInfo_ != NULL )
	{
		::freeaddrinfo( ServInfo_ );
		ServInfo_ = NULL;
	}
	
	if( SocketDesc_ != 0 )
	{
		::closesocket( SocketDesc_ );
		SocketDesc_ = 0;
	}
	
	Port_ = 0;
	IsConnected_ = BcFalse;
	*/
}

////////////////////////////////////////////////////////////////////////////////
// send
BcBool RmTCPConnection::send( const void* pData, BcU32 Bytes )
{
	/*
	if( SocketDesc_ != BcErrorCode )
	{
		const BcU8* pDataBuffer( (const BcU8*)pData );
	
		while( Bytes > 0 )
		{
#if PLATFORM_WINDOWS
			//u_long Mode = 1;
			//::ioctlsocket( SocketDesc_, FIONBIO, &Mode );
#else
			fcntl( SocketDesc_, F_SETFL, O_RDWR );
#endif
			BcU32 DataSent = ::send( SocketDesc_, (const char*)pDataBuffer, Bytes, 0 );
			Bytes -= DataSent;
		
			pDataBuffer += DataSent;
		
			// Handle error.
			if( DataSent <= 0 )
			{
				return BcFalse;
			}
		}
		return BcTrue;
	}
	
	return BcFalse;
	*/
	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// recv
BcU32 RmTCPConnection::recv( void* pBuffer, BcU32 Bytes, BcBool Peek, BcBool Block )
{
	/*
	if( SocketDesc_ != 0 )
	{
#if PLATFORM_WINDOWS
		u_long Mode = Block ? 1 : 0;
		::ioctlsocket( SocketDesc_, FIONBIO, &Mode );
#else
		fcntl( SocketDesc_, F_SETFL, O_RDWR | ( Block ? 0 : O_NONBLOCK ) );
#endif
		int RetVal = ::recv( SocketDesc_, (char*)pBuffer, Bytes, ( Peek ? MSG_PEEK : 0 ) | ( Block ? MSG_WAITALL : 0 ) );
		return RetVal >= 0 ? RetVal : BcErrorCode;
	}
	
	return BcErrorCode;
	*/
	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// parseAddressInfo
BcBool RmTCPConnection::parseAddressInfo( const BcChar* Address, BcU16 Port )
{
	/*
	BcAssert( ServInfo_ == NULL );
	
	int Status;		
	addrinfo AddrHints;
	memset( &AddrHints, 0, sizeof( AddrHints ) ); // Clear all to 0.
	AddrHints.ai_family = AF_INET;
	AddrHints.ai_socktype = SOCK_STREAM; // TCP.
	BcChar PortBuffer[8];
	sprintf( PortBuffer, "%u", Port );
	Status = ::getaddrinfo( Address, PortBuffer, &AddrHints, &ServInfo_ );

	/* code to list IP addrs.
	printf("IP addresses for %s:\n\n", Address);
	addrinfo* p = ServInfo_;
	addrinfo* res = ServInfo_;
	char ipstr[256];
	
    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
		
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
		
        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }
	 */ /*
	
	return Status == 0;
	*/
	return BcFalse;
}
