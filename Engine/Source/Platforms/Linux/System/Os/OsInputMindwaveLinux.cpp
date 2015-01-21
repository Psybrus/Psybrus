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
	static const BcU8 ID_SYNC = 0xaa;
	static const BcU8 ID_EXCODE = 0x55;
	static const BcU8 ID_POOR_QUALITY = 0x02;
	static const BcU8 ID_ATTENTION = 0x04;
	static const BcU8 ID_MEDITATION = 0x05;
	static const BcU8 ID_BLINK_STRENGTH = 0x16;
	static const BcU8 ID_RAW_EEG = 0x80;
	static const BcU8 ID_EEG_POWERS = 0x83;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
OsInputMindwaveLinux::OsInputMindwaveLinux():
	State_( State::INIT ),
	Shutdown_( 0 ),
	DeviceId_( 0 ),
	Socket_( 0 )
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
						OsEventInputMindwave OldEvent;
						if( OldEvent.PoorSignal_ != Event_.PoorSignal_ ||
							OldEvent.Attention_ != Event_.Attention_ ||
							OldEvent.Meditation_ != Event_.Meditation_ ||
							OldEvent.Blink_ != Event_.Blink_ )
						{
							BcPrintf( "S: %u, A: %u, M: %u, B: %u\n!", 
								Event_.PoorSignal_,
								Event_.Attention_,
								Event_.Meditation_,
								Event_.Blink_ );
							OldEvent = Event_;

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
	OsInputMindwaveLinux* This = (OsInputMindwaveLinux*)CustomData;
	switch( Code )
	{
	case PARSER_CODE_POOR_QUALITY:
		This->Event_.PoorSignal_ = *Value;
		break;
	case PARSER_CODE_ATTENTION:
		This->Event_.Attention_ = *Value;
		break;
	case PARSER_CODE_MEDITATION:
		This->Event_.Meditation_ = *Value;
		break;
	case ID_BLINK_STRENGTH:
		This->Event_.Blink_ = *Value;
		break;
	}
}
