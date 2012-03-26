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

#include <libircclient.h>
#include <libirc_rfcnumeric.h>
#include <enet/enet.h>

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaMatchmakingState
class GaMatchmakingState: 
	public BcGlobal< GaMatchmakingState >,
	public BcThread,
	public SysState
{
public:
	GaMatchmakingState();
	virtual ~GaMatchmakingState();

	eSysStateReturn main();
	
	BcBool sendLocalAddress( const BcChar* pDest );

	static int getSocketFileDescriptor();
	static BcU32 getClientID();
	static BcU32 getLocalAddr();
	static BcU16 getLocalPort();
	static BcU32 getRemoteAddr();
	static BcU16 getRemotePort();
	static BcU32 getMappedAddr();
	static BcU16 getMappedPort();
	static BcU32 getLANAddr();
	static BcU16 getLANPort();

private:
	virtual void execute();

private:
	static void event_connect(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_nick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_quit(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_join(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_part(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_mode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_umode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_topic(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_kick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_channel(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_privmsg(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_channel_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_invite(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_ctcp_req(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_ctcp_rep(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_ctcp_action(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_unknown(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count);
	static void event_numeric(irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count);
	static void event_dcc_chat_req(irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid);
	static void event_dcc_send_req(irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid);

private:
	BcChar ScreenName_[ 64 ];
	BcChar Channel_[ 64 ];
	irc_callbacks_t Callbacks_;
	irc_session_t* pSession_;

	BcMutex Lock_;
	
	enum HandshakeState
	{
		HSS_IDLE = 0,
		HSS_WAIT_INVITE,
		HSS_WAIT_ADDR,
		HSS_COMPLETE
	};

	HandshakeState HandshakeState_;

	BcReal ConnectTimer_;
	BcReal InviteTimer_;
	BcReal HandshakeTimer_;


	// Out socket file descriptor.
	static int SocketFileDescriptor_;

	// Our client ID.
	static BcU32 ClientID_;

	// Remote address. (We join this).
	static BcU32 RemoteHandshakeAddr_;
	static BcU16 RemoteHandshakePort_;

	// Local address. (We host on this).
	static BcU32 LocalHandshakeAddr_;
	static BcU32 LocalHandshakePort_;

	// Mapped local address. (We send this).
	static BcU32 MappedHandshakeAddr_;
	static BcU32 MappedHandshakePort_;

	// LAN address. (For LAN play).
	static BcU32 LANHandshakeAddr_;
	static BcU16 LANHandshakePort_;
};
