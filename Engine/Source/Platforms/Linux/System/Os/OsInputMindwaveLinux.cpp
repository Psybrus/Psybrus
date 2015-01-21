#include "System/Os/OsInputMindwaveLinux.h"

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <ThinkGearStreamParser.h>

namespace
{
	// Communications protocol:
	// http://wearcam.org/ece516/mindset_communications_protocol.pdf
}

//////////////////////////////////////////////////////////////////////////
// Ctor
OsInputMindwaveLinux::OsInputMindwaveLinux():
	State_( State::INIT ),
	Shutdown_( 0 ),
	DeviceId_( 0 ),
	Socket_( 0 ),
	RawDataIdx_( 0 )
{
	StreamParser_.reset( new _ThinkGearStreamParser );
	THINKGEAR_initParser( StreamParser_.get(), PARSER_TYPE_PACKETS, handleDataValue, this );
	WorkerThread_ = std::thread( std::bind( &OsInputMindwaveLinux::workerThread, this ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsInputMindwaveLinux::~OsInputMindwaveLinux()
{
	// If connected, force close socket.
	if( State_ == State::READ )
	{
		close( Socket_ );
		Socket_ = 0;
	}

	// Set shutdown to 1, then join.
	Shutdown_ = 1;
	WorkerThread_.join();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
void OsInputMindwaveLinux::update()
{
}


//////////////////////////////////////////////////////////////////////////
// workerThread
void OsInputMindwaveLinux::workerThread()
{
	while( State_ != State::DEINIT )
	{
		switch( State_.load() )
		{
		case State::INIT:
			{
				State_ = Shutdown_ == 0 ? State::SCAN : State::DEINIT;
			}
			break;

		case State::SCAN:
			{
				// TODO: Scan for a mindwave external to this?
				State_ = Shutdown_ == 0 ? State::CONNECT : State::DEINIT;
			}
			break;

		case State::CONNECT:
			{
				struct sockaddr_rc Addr = { 0 };
				char Dest[18] = "9C:B7:0D:89:E5:2D";
				int Status = 0;

				// allocate a socket
				Socket_ = ::socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );

				// set the connection parameters (who to connect to)
				Addr.rc_family = AF_BLUETOOTH;
				Addr.rc_channel = (uint8_t) 1;
				str2ba( Dest, &Addr.rc_bdaddr );

				// connect to server
				Status = ::connect( Socket_, (struct sockaddr *)&Addr, sizeof( Addr ) );

				if( Status < 0 ) 
				{
					BcPrintf( "Failed to connect to %s\n", Dest );
					State_ = Shutdown_ == 0 ? State::SCAN : State::DEINIT;
					close( Socket_ );
					Socket_ = 0;
					::usleep( 1000000 );
				}
				else
				{
					BcPrintf( "Connected to %s\n", Dest );
					State_ = Shutdown_ == 0 ? State::READ : State::DISCONNECT;
				}
			}
			break;

		case State::READ:
			{
				BcU8 SingleByte = 0;
				int RetVal = ::read( Socket_, &SingleByte, sizeof( SingleByte ) );
				if( RetVal >= 0 )
				{
					RetVal = THINKGEAR_parseByte( StreamParser_.get(), SingleByte );

 					if( RetVal > 0 )
 					{
						OsEventInputMindwaveData OldEvent;
						if( OldEvent.PoorSignal_ != EventData_.PoorSignal_ ||
							OldEvent.Attention_ != EventData_.Attention_ ||
							OldEvent.Meditation_ != EventData_.Meditation_ )
						{
							BcPrintf( "S: %u, A: %u, M: %u\n!", 
								EventData_.PoorSignal_,
								EventData_.Attention_,
								EventData_.Meditation_ );
							BcPrintf( " - %u\n - %u\n - %u\n - %u\n - %u\n - %u\n - %u\n\n - %u", 
								EventEEGPower_.Values_[ 0 ],
								EventEEGPower_.Values_[ 1 ],
								EventEEGPower_.Values_[ 2 ],
								EventEEGPower_.Values_[ 3 ],
								EventEEGPower_.Values_[ 4 ],
								EventEEGPower_.Values_[ 5 ],
								EventEEGPower_.Values_[ 6 ],
								EventEEGPower_.Values_[ 7 ] );
							OldEvent = EventData_;

						}
 					}

					State_ = Shutdown_ == 0 ? State::READ : State::DISCONNECT;
				}
				else
				{
					BcPrintf( "Disconnected!\n" );
					close( Socket_ );
					Socket_ = 0;
					State_ = Shutdown_ == 0 ? State::SCAN : State::DISCONNECT;
				}
			}
			break;

		case State::DISCONNECT:
			{
				close( Socket_ );
				State_ = State::DEINIT;
			}
			break;

		default:
			{
			}
			break;
		}
	}

	// TODO: Deinit.
}


//////////////////////////////////////////////////////////////////////////
// handleDataValue
//static
void OsInputMindwaveLinux::handleDataValue(
		unsigned char ExtendedCodeLevel,
		unsigned char Code, unsigned char NumBytes,
		const unsigned char* Value, void* CustomData )
{
	if( NumBytes == 24 )
	{
		int a = 0; ++a;
	}

	OsInputMindwaveLinux* This = (OsInputMindwaveLinux*)CustomData;
	switch( Code )
	{
	case PARSER_CODE_POOR_QUALITY:
		BcAssert( NumBytes == 1 );
		This->EventData_.PoorSignal_ = *Value;
		break;
	
	case PARSER_CODE_ATTENTION:
		BcAssert( NumBytes == 1 );
		This->EventData_.Attention_ = *Value;
		break;
	
	case PARSER_CODE_MEDITATION:
		BcAssert( NumBytes == 1 );
		This->EventData_.Meditation_ = *Value;
		break;
	
	case PARSER_CODE_ASIC_EEG_POWER_INT:
		BcAssert( NumBytes == 24 );
		This->EventEEGPower_.Values_[ 0 ] = 
			( Value[ 0 ] << 16 ) | ( Value[ 1 ] << 8 ) | ( Value [ 2 ] << 0 );

		This->EventEEGPower_.Values_[ 1 ] = 
			( Value[ 3 ] << 16 ) | ( Value[ 4 ] << 8 ) | ( Value [ 5 ] << 0 );
		
		This->EventEEGPower_.Values_[ 2 ] = 
			( Value[ 6 ] << 16 ) | ( Value[ 7 ] << 8 ) | ( Value [ 8 ] << 0 );
		
		This->EventEEGPower_.Values_[ 3 ] = 
			( Value[ 9 ] << 16 ) | ( Value[ 10 ] << 8 ) | ( Value [ 11 ] << 0 );
		
		This->EventEEGPower_.Values_[ 4 ] = 
			( Value[ 12 ] << 16 ) | ( Value[ 13 ] << 8 ) | ( Value [ 14 ] << 0 );
		
		This->EventEEGPower_.Values_[ 5 ] = 
			( Value[ 15 ] << 16 ) | ( Value[ 16 ] << 8 ) | ( Value [ 17 ] << 0 );
		
		This->EventEEGPower_.Values_[ 6 ] = 
			( Value[ 18 ] << 16 ) | ( Value[ 19 ] << 8 ) | ( Value [ 20 ] << 0 );

		This->EventEEGPower_.Values_[ 7 ] = 
			( Value[ 21 ] << 16 ) | ( Value[ 22 ] << 8 ) | ( Value [ 23 ] << 0 );
		break;

	case PARSER_CODE_RAW_SIGNAL:
		BcAssert( NumBytes == 2 );
		BcAssert( This->RawDataIdx_< OsEventInputMindwaveEEGRaw::BUFFER_SIZE )
		This->EventEEGRaw_.Buffer_[ This->RawDataIdx_++ ] =
			( Value[ 0 ] << 8 ) | ( Value[ 1 ] << 0 );

			if( This->RawDataIdx_ == OsEventInputMindwaveEEGRaw::BUFFER_SIZE )
			{
				This->RawDataIdx_ = 0;

				// TODO: SEND EVENT!
			}

		break;
	}
}
