/**************************************************************************
*
* File:		EvtBridgeIRC.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event bridge over IRC.
*		
*		
*
* 
**************************************************************************/

#include <libircclient.h>
#include <libirc_rfcnumeric.h>

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// EvtBridgeIRC
class EvtBridgeIRC: 
	public EvtBridge,
	public BcThread
{
public:
	EvtBridgeIRC( EvtPublisher* pPublisher );
	virtual ~EvtBridgeIRC();

private:
	virtual void bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

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
	static void dcc_callback(irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length);

	void dcc_publish_event( EvtID ID, BcU32 EventSize, char* pEventData );

private:
	BcChar ScreenName_[ 64 ];
	BcChar Channel_[ 64 ];
	irc_callbacks_t Callbacks_;
	irc_session_t* pSession_;

	BcMutex Lock_;
	std::list< std::string > UserList_;

	irc_dcc_t DCC_;

	BcBool HasJoined_;
	BcBool HasDCC_;

};