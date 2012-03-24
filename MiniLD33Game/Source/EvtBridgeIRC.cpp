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

#include "EvtBridgeIRC.h"

#define BUFFERSIZE ( 64 * 1024 )
#include <b64/encode.h>
#include <b64/decode.h>
#undef BUFFERSIZE

#include <zlib.h>

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtBridgeIRC::EvtBridgeIRC( EvtPublisher* pPublisher ):
	EvtBridge( pPublisher )
{
	HasJoined_ = BcFalse;
	DCC_ = -1;

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

	pSession_ = irc_create_session( &Callbacks_ );

#if PLATFORM_WINDOWS
	BcRandom::Global = BcRandom( ::GetTickCount() );
#endif

	if( pSession_ )
	{
		irc_set_ctx( pSession_, this );

		BcSPrintf( ScreenName_, "%s_%x", "ScreenName", BcRandom::Global.rand() );
		BcSPrintf( Channel_, "#psybrus" );

		// Connect to the server.
		int RetVal = irc_connect( pSession_, "irc.afternet.org", 6667, NULL, ScreenName_, ScreenName_, ScreenName_ );
		
		// Start the thread to tick the client.
		BcThread::start( "EvtBridgeIRC" );
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtBridgeIRC::~EvtBridgeIRC()
{
	if( pSession_ )
	{
		irc_disconnect( pSession_ );
		irc_destroy_session( pSession_ );
	}
}

////////////////////////////////////////////////////////////////////////////////
// bridge
//virtual
void EvtBridgeIRC::bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	if( irc_is_connected( pSession_ ) && HasJoined_ && DCC_ != -1 )
	{
		BcStream Stream;
		Stream.push( &ID, sizeof( ID ) );
		Stream.push( &EventSize, sizeof( EventSize ) );
		Stream.push( &EventBase, EventSize );

		// Compress.
		BcU32 CompressionBufferSize = Stream.dataSize() * 2;
		BcChar* pCompressionBuffer = new BcChar[ CompressionBufferSize ];
		uLongf CompressedSize = CompressionBufferSize;
		compress((Bytef*)pCompressionBuffer, &CompressedSize, (Bytef*)Stream.pData(), Stream.dataSize());

		// Base-64 encode.
		BcU32 B64BufferSize = Stream.dataSize() * 2;
		BcChar* pB64Buffer = new BcChar[ B64BufferSize ];
		base64::base64_encodestate EncodeState;
		BcMemZero( pB64Buffer, B64BufferSize );
		BcMemZero( &EncodeState, sizeof( EncodeState ) );
		base64::base64_encode_block( reinterpret_cast< const char* >( pCompressionBuffer ), (int)CompressedSize, reinterpret_cast< char* >( pB64Buffer ), &EncodeState );

		// Send message.
		irc_dcc_msg( pSession_, DCC_, pB64Buffer );

		// Delete 
		delete [] pB64Buffer;
		delete [] pCompressionBuffer;
	}
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void EvtBridgeIRC::execute()
{
	irc_run( pSession_ );
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_connect(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_connect %s, %s\n", origin, params[i] );
	}

	// Join the channel.
	irc_cmd_join( session, pBridge->Channel_, NULL );
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_nick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_nick %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_quit(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_quit %s, %s\n", origin, params[i] );
	}

	if( params != NULL && BcStrCompare( params[0], pBridge->Channel_ ) )
	{
		// Grab short name from origin.
		BcChar NameCopy[ 256 ];
		BcStrCopy( NameCopy, origin );
		BcChar* pNameEnd = BcStrStr( NameCopy, "!" );

		if( pNameEnd != NULL )
		{
			*pNameEnd = '\0';
		}

		if( BcStrCompare( NameCopy, pBridge->ScreenName_ ) )
		{
			pBridge->HasJoined_ = BcFalse;
		}

		pBridge->UserList_.remove( NameCopy );
	}
}


//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_join(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_join %s, %s\n", origin, params[i] );
	}

	if( params != NULL && BcStrCompare( params[0], pBridge->Channel_ ) )
	{
		// Grab short name from origin.
		BcChar NameCopy[ 256 ];
		BcStrCopy( NameCopy, origin );
		BcChar* pNameEnd = BcStrStr( NameCopy, "!" );

		if( pNameEnd != NULL )
		{
			*pNameEnd = '\0';
		}

		if( BcStrCompare( NameCopy, pBridge->ScreenName_ ) )
		{
			pBridge->HasJoined_ = BcTrue;

			//irc_cmd_names( session, pBridge->Channel_ );
		}

		pBridge->UserList_.push_back( NameCopy );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_part(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );
	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_part %s, %s\n", origin, params[i] );
	}
	
	if( params != NULL && BcStrCompare( params[0], pBridge->Channel_ ) )
	{
		// Grab short name from origin.
		BcChar NameCopy[ 256 ];
		BcStrCopy( NameCopy, origin );
		BcChar* pNameEnd = BcStrStr( NameCopy, "!" );

		if( pNameEnd != NULL )
		{
			*pNameEnd = '\0';
		}

		if( BcStrCompare( NameCopy, pBridge->ScreenName_ ) )
		{
			pBridge->HasJoined_ = BcFalse;
		}

		pBridge->UserList_.remove( NameCopy );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_mode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_mode %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_umode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_umode %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_topic(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_topic %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_kick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_kick %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_channel(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_channel %s, %s\n", origin, params[i] );
	}

	// If we see a play with me message that isn't from us, attempt to DCC.
	if( pBridge->DCC_ == -1 )
	{
		// Grab short name from origin.
		BcChar NameCopy[ 256 ];
		BcStrCopy( NameCopy, origin );
		BcChar* pNameEnd = BcStrStr( NameCopy, "!" );

		if( pNameEnd != NULL )
		{
			*pNameEnd = '\0';
		}

		if( !BcStrCompare( pBridge->ScreenName_, NameCopy ) && BcStrCompare( params[1], "play with me!" ) )
		{
			irc_dcc_chat( session, pBridge, NameCopy, dcc_callback, &pBridge->DCC_ );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_privmsg(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_privmsg %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_notice: %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_channel_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_notice: %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_invite(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	for( int i = 0; i < count; ++i )
	{
		BcPrintf( "event_invite %s, %s\n", origin, params[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_ctcp_req(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_ctcp_rep(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_ctcp_action(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_unknown(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_numeric(irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	if( event == LIBIRC_RFC_RPL_NAMREPLY )
	{
		for( int i = 0; i < count; ++i )
		{
			BcPrintf( "event_numeric LIBIRC_RFC_RPL_NAMREPLY: %s, %s\n", origin, params[i] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_dcc_chat_req(irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::event_dcc_send_req(irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );


}

//////////////////////////////////////////////////////////////////////////
//
void EvtBridgeIRC::dcc_callback(irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length)
{
	EvtBridgeIRC* pBridge = reinterpret_cast< EvtBridgeIRC* >( irc_get_ctx( session ) );
	BcScopedLock< BcMutex > Lock( pBridge->Lock_ );

	if(status == 0 && data != NULL && length > 0)
	{
		// Base-64 decode.
		char* pB64DecodedBuffer = new char[ length ];
		base64::base64_decodestate DecodeState;
		BcMemZero( &DecodeState, sizeof( DecodeState ) );
		base64::base64_decode_block( data, length, pB64DecodedBuffer, &DecodeState );

		// Decompress
		BcU32 DecompressionBufferSize = length * 32; // hacky
		BcChar* pDecompressedBuffer = new BcChar[ DecompressionBufferSize ];
		uLongf DecompressedSize = 0;
		uncompress((Bytef*)pDecompressedBuffer, &DecompressedSize, (Bytef*)pB64DecodedBuffer, length);

		EvtID ID = *reinterpret_cast< EvtID* >( &pDecompressedBuffer[ 0 ] );
		BcU32 EventSize = *reinterpret_cast< EvtID* >( &pDecompressedBuffer[ 4 ] );

		// Enqueue a callback for the main thread to pick up.
		typedef BcDelegate< void(*)( EvtID, BcU32, char* ) > PublishDelegate;
		PublishDelegate PublishCallback( PublishDelegate::bind< EvtBridgeIRC, &EvtBridgeIRC::dcc_publish_event >( pBridge ) );
		SysKernel::pImpl()->enqueueCallback( PublishCallback, ID, EventSize, pDecompressedBuffer );
	}
}

//////////////////////////////////////////////////////////////////////////
// dcc_publish_event
void EvtBridgeIRC::dcc_publish_event( EvtID ID, BcU32 EventSize, char* pEventData )
{
	// Publish.
	publish( ID, *reinterpret_cast< EvtBaseEvent* >( &pEventData[ 8 ] ), EventSize );

	// Free event data.
	delete [] pEventData;
}