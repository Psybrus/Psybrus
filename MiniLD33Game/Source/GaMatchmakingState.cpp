/**************************************************************************
*
* File:		GaMatchmakingState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		IRC match maker.
*		
*		
*
* 
**************************************************************************/

#include "GaMatchmakingState.h"

#include <stun.h>

int GaMatchmakingState::SocketFileDescriptor_ = 0;
BcU32 GaMatchmakingState::ClientID_ = BcErrorCode;
BcU32 GaMatchmakingState::RemoteHandshakeAddr_ = 0;
BcU16 GaMatchmakingState::RemoteHandshakePort_ = 0;
BcU32 GaMatchmakingState::LocalHandshakeAddr_ = 0;
BcU32 GaMatchmakingState::LocalHandshakePort_ = 0;
BcU32 GaMatchmakingState::MappedHandshakeAddr_ = 0;
BcU32 GaMatchmakingState::MappedHandshakePort_ = 0;
BcU32 GaMatchmakingState::LANHandshakeAddr_ = 0;
BcU16 GaMatchmakingState::LANHandshakePort_ = 0;

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaMatchmakingState::GaMatchmakingState()
{
	name( "GaMatchmakingState" );

	HandshakeState_ = HSS_IDLE;

	BcMemZero( &Callbacks_, sizeof( Callbacks_ ) );
	Callbacks_.event_connect = event_connect;
	Callbacks_.event_nick = event_nick;
	Callbacks_.event_nick = event_quit;
	Callbacks_.event_join = event_join;
	Callbacks_.event_part = event_part;
	Callbacks_.event_mode = event_mode;
	Callbacks_.event_umode = event_umode;
	Callbacks_.event_topic = event_topic;
	Callbacks_.event_kick = event_kick;
	Callbacks_.event_channel = event_channel;
	Callbacks_.event_privmsg = event_privmsg;
	Callbacks_.event_notice = event_notice;
	Callbacks_.event_channel_notice = event_channel_notice;
	Callbacks_.event_invite = event_invite;
	Callbacks_.event_ctcp_req = event_ctcp_req;
	Callbacks_.event_ctcp_rep = event_ctcp_rep;
	Callbacks_.event_ctcp_action = event_ctcp_action;
	Callbacks_.event_unknown = event_unknown;
	Callbacks_.event_numeric = event_numeric;
	Callbacks_.event_dcc_chat_req = event_dcc_chat_req;
	Callbacks_.event_dcc_send_req = event_dcc_send_req;

	ConnectTimer_ = 0.0f;
	InviteTimer_ = BcAbs( BcRandom::Global.randReal() ) * 5.0f + 5.0f;
	HandshakeTimer_ = 0.0f;

	pSession_ = NULL;

#if PLATFORM_WINDOWS
	BcRandom::Global = BcRandom( ::GetTickCount() );
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaMatchmakingState::~GaMatchmakingState()
{
	BcPrintf("~GaMatchmakingState();\n");

	if( pSession_ != NULL )
	{
		irc_destroy_session( pSession_ );
		pSession_ = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
// update
eSysStateReturn GaMatchmakingState::main()
{
	BcScopedLock< BcMutex > Lock( Lock_ );
	BcReal Delta = SysKernel::pImpl()->getFrameTime();

	switch( HandshakeState_ )
	{
	case HSS_IDLE:
		{
			ConnectTimer_ -= Delta;

			if( ConnectTimer_ < 0.0f )
			{
				if( pSession_ == NULL )
				{
					pSession_ = irc_create_session( &Callbacks_ );
				}

				if( pSession_ != NULL && !irc_is_connected( pSession_ ) )
				{
					irc_set_ctx( pSession_, this );

					BcSPrintf( ScreenName_, "%s_%x", "PSY_", BcRandom::Global.rand() );
					BcSPrintf( Channel_, "#psybrus_minild33" );

					// Connect to the server.
					int RetVal = irc_connect( pSession_, "www.neilo.gd", 8000, NULL, ScreenName_, ScreenName_, ScreenName_ );

					if( RetVal == 0 )
					{
						// Start the thread to tick the client.
						BcThread::start( "EvtBridgeIRC" );

						ClientID_ = BcErrorCode;
						RemoteHandshakeAddr_ = 0;
						RemoteHandshakePort_ = 0;
						LocalHandshakeAddr_ = 0;
						LocalHandshakePort_ = 0;
						MappedHandshakeAddr_ = 0;
						MappedHandshakePort_ = 0;
						HandshakeState_ = HSS_WAIT_INVITE;
					}
					else
					{
						BcThread::join();

						irc_destroy_session( pSession_ );
						pSession_ = NULL;
					}
				}
			}
		}
		break;

	case HSS_WAIT_INVITE:
		{
			InviteTimer_ -= Delta;

			if( InviteTimer_ < 0.0f )
			{
				InviteTimer_ = BcAbs( BcRandom::Global.randReal() ) * 5.0f + 5.0f;

				// Send play with me message to channel.
				irc_cmd_msg( pSession_, Channel_, "play with me!" );
			}
		}
		break;

	case HSS_WAIT_ADDR:
		{
			HandshakeTimer_ -= Delta;

			if( HandshakeTimer_ < 0.0f )
			{
				HandshakeState_ = HSS_WAIT_INVITE;
			}
		}
		break;

	case HSS_COMPLETE:
		{
			BcPrintf("GaMatchmakingState: Complete! ClientID of ours is %u\n", ClientID_);
			return sysSR_FINISHED;
		}
		break;
	}

	if( HandshakeState_ != HSS_IDLE && ( pSession_ == NULL || !irc_is_connected( pSession_ ) ) )
	{
		BcSleep( 0.1f );
		BcThread::join();
		BcSleep( 0.1f );
		if( pSession_ != NULL )
		{
			irc_destroy_session( pSession_ );
			pSession_ = NULL;
		}
		HandshakeState_ = HSS_IDLE;
		ConnectTimer_ = 10.0f;
	}
	
	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// leave
eSysStateReturn GaMatchmakingState::leave()
{
	BcScopedLock< BcMutex > Lock( Lock_ );
	BcPrintf( "Leave!\n" );
	if( pSession_ )
	{
		irc_disconnect( pSession_ );
		BcSleep( 0.1f );
		BcThread::join();
		BcSleep( 0.1f );
	}
	return sysSR_FINISHED;
}
////////////////////////////////////////////////////////////////////////////////
// bridge
BcBool GaMatchmakingState::sendLocalAddress( const BcChar* pDest )
{
	// Perform STUN to get our local address.
	StunAddress4 StunAddress;
	StunAddress4 MappedAddress;
	BcBool GotPort = BcFalse;

	BcPrintf("Connecting to STUN server..\n");
	if( stunParseServerName( "stunserver.org", StunAddress ) )
	{
		int BasePort = 6000;
		for( BcU32 TryIdx = 0; TryIdx < 10; ++TryIdx )
		{
			// Set port & NIC.
			int SrcPort = BasePort + TryIdx;
			StunAddress4 NICAddr;
			NICAddr.addr = 0; 
			NICAddr.port = SrcPort; 

			// Open port for NAT punchthrough.
			if( SocketFileDescriptor_ != 0 )
			{
				closesocket( SocketFileDescriptor_ );
				SocketFileDescriptor_ = 0;
			}
			BcPrintf("Opening socket on port %u..\n", SrcPort);
			SocketFileDescriptor_ = stunOpenSocket( StunAddress, &MappedAddress, SrcPort, &NICAddr, false );
			
			// Ok, we got one.
			if( SocketFileDescriptor_ > 0 )
			{
				GotPort = BcTrue;
				BcPrintf("Got socket!\n");
				
				LocalHandshakeAddr_ = 0;
				LocalHandshakePort_ = SrcPort;
				break;
			}
		}
	}
	
	if( GotPort )
	{
		// Got a port open, also grab our LAN address.
		struct in_addr Addr;
		BcMemZero( &Addr, sizeof( Addr ) );
		char HostName[ 128 ];
		if( gethostname( HostName, sizeof( HostName ) ) != SOCKET_ERROR )
		{
			struct hostent* pHostEntry = gethostbyname( HostName );
			if( pHostEntry != NULL )
			{
				for( int Idx = 0; pHostEntry->h_addr_list[ Idx ] != 0; ++Idx )
				{
					memcpy( &Addr, pHostEntry->h_addr_list[ Idx ], sizeof( struct in_addr ) );
					break;
				}
			}
		}

		//
		BcChar AddrBuffer[ 256 ];
		BcSPrintf( AddrBuffer, "ADDR:%u.%u.%u.%u:%u/%u.%u.%u.%u:%u", 
			Addr.S_un.S_un_b.s_b1,
			Addr.S_un.S_un_b.s_b2,
			Addr.S_un.S_un_b.s_b3,
			Addr.S_un.S_un_b.s_b4,
			LocalHandshakePort_,
			( MappedAddress.addr >> 24 ) & 0xff,
			( MappedAddress.addr >> 16 ) & 0xff,
			( MappedAddress.addr >> 8 ) & 0xff,
			( MappedAddress.addr ) & 0xff,
			MappedAddress.port
			);

		// Store mapped.
		MappedHandshakeAddr_ = MappedAddress.addr;
		MappedHandshakePort_ = MappedAddress.port;

		BcPrintf( "Send: %s (%u)\n", AddrBuffer, LocalHandshakePort_ );
				
		// Send message.
		BcPrintf( "pre-irc_cmd_msg:\n" );
		int RetVal = irc_cmd_msg( pSession_, pDest, AddrBuffer );
		BcPrintf( "irc_cmd_msg: %u\n", RetVal );
	}

	return GotPort;
}

////////////////////////////////////////////////////////////////////////////////
// getSocketFileDescriptor
int GaMatchmakingState::getSocketFileDescriptor()
{
	return SocketFileDescriptor_;
}

////////////////////////////////////////////////////////////////////////////////
// getClientID
BcU32 GaMatchmakingState::getClientID()
{
	return ClientID_;
}

////////////////////////////////////////////////////////////////////////////////
// getLocalAddr
BcU32 GaMatchmakingState::getLocalAddr()
{
	return LocalHandshakeAddr_;
}

////////////////////////////////////////////////////////////////////////////////
// getLocalPort
BcU16 GaMatchmakingState::getLocalPort()
{
	return LocalHandshakePort_;
}

////////////////////////////////////////////////////////////////////////////////
// getRemoteAddr
BcU32 GaMatchmakingState::getRemoteAddr()
{
	return RemoteHandshakeAddr_;
}

////////////////////////////////////////////////////////////////////////////////
// getRemotePort
BcU16 GaMatchmakingState::getRemotePort()
{
	return RemoteHandshakePort_;
}

////////////////////////////////////////////////////////////////////////////////
// getMappedAddr
BcU32 GaMatchmakingState::getMappedAddr()
{
	return MappedHandshakeAddr_;
}

////////////////////////////////////////////////////////////////////////////////
// getMappedPort
BcU16 GaMatchmakingState::getMappedPort()
{
	return MappedHandshakePort_;
}

////////////////////////////////////////////////////////////////////////////////
// getLANAddr
BcU32 GaMatchmakingState::getLANAddr()
{
	return LANHandshakeAddr_;
}

////////////////////////////////////////////////////////////////////////////////
// getLANAddr
BcU16 GaMatchmakingState::getLANPort()
{
	return LANHandshakePort_;
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void GaMatchmakingState::execute()
{
	irc_run( pSession_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_connect(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( unsigned int i = 0; i < count; ++i )
	{
		BcPrintf( "event_connect %s, %s\n", origin, params[i] );
	}

	// Join the channel.
	irc_cmd_join( session, pBridge->Channel_, NULL );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_nick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
	for( unsigned int i = 0; i < count; ++i )
	{
		BcPrintf( "event_nick %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_quit(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}


//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_join(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_part(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_mode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_umode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_topic(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_kick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_channel(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	// If we aren't currently handshaking
	if( pBridge->HandshakeState_ == HSS_WAIT_INVITE )
	{
		// Grab short name from origin.
		BcChar NameCopy[ 256 ];
		BcStrCopy( NameCopy, origin );
		BcChar* pNameEnd = BcStrStr( NameCopy, "!" );

		if( pNameEnd != NULL )
		{
			*pNameEnd = '\0';
		}

		// If the message isn't from ourself, and it's the play with me message, then send our local address.
		if( !BcStrCompare( pBridge->ScreenName_, NameCopy ) && BcStrCompare( params[1], "play with me!" ) )
		{
			if( pBridge->sendLocalAddress( NameCopy ) )
			{
				// Invite player to play, we send address first.
				ClientID_ = 0;

				// Give handshake 15 seconds to complete.
				pBridge->HandshakeTimer_ = 15.0f;

				// Set wait for address state.
				pBridge->HandshakeState_ = HSS_WAIT_ADDR;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_privmsg(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
	
	if ( pBridge->HandshakeState_ == HSS_WAIT_INVITE || pBridge->HandshakeState_ == HSS_WAIT_ADDR )
	{
		// Grab short name from origin.
		static BcChar NameCopy[ 256 ];
		BcStrCopy( NameCopy, origin );
		BcChar* pNameEnd = BcStrStr( NameCopy, "!" );

		if( pNameEnd != NULL )
		{
			*pNameEnd = '\0';
		}

		if(count > 1 && strstr( params[1], "ADDR:" ) == params[1] )
		{
			BcPrintf( "Recv: %s\n", params[1] );

			// We got send an address first, so send ours.
			if( ClientID_ == BcErrorCode )
			{
				if( pBridge->sendLocalAddress( NameCopy ) )
				{
					ClientID_ = 1;
				}
				else
				{
					// Failure :(
					pBridge->HandshakeState_ = HSS_WAIT_INVITE;
					return;
				}
			}

			BcU32 LA, LB, LC, LD, LE;
			BcU32 A, B, C, D, E;
			BcSScanf( params[1], "ADDR:%u.%u.%u.%u:%u/%u.%u.%u.%u:%u", &LA, &LB, &LC, &LD, &LE, &A, &B, &C, &D, &E );

			pBridge->LANHandshakeAddr_ = LA << 24 | LB << 16 | LC << 8 | LD;
			pBridge->LANHandshakePort_ = LE;

			pBridge->RemoteHandshakeAddr_ = A << 24 | B << 16 | C << 8 | D;
			pBridge->RemoteHandshakePort_ = E;
			pBridge->HandshakeState_ = HSS_COMPLETE;
		}
		else
		{
			pBridge->HandshakeState_ = HSS_WAIT_INVITE;
		}
	}

	BcPrintf("event_privmsg done.\n");
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( unsigned int i = 0; i < count; ++i )
	{
		BcPrintf( "event_notice: %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_channel_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( unsigned int i = 0; i < count; ++i )
	{
		BcPrintf( "event_notice: %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_invite(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( unsigned int i = 0; i < count; ++i )
	{
		BcPrintf( "event_invite %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_ctcp_req(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_ctcp_rep(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_ctcp_action(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_unknown(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_numeric(irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	if( event == LIBIRC_RFC_RPL_NAMREPLY )
	{
		for( unsigned int i = 0; i < count; ++i )
		{
			BcPrintf( "event_numeric LIBIRC_RFC_RPL_NAMREPLY: %s, %s\n", origin, params[i] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_dcc_chat_req(irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}

//////////////////////////////////////////////////////////////////////////
//
void GaMatchmakingState::event_dcc_send_req(irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid)
{
	GaMatchmakingState* pBridge = reinterpret_cast< GaMatchmakingState* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
}
