#include "System/Os/OsInputMindwaveLinux.h"

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

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

	/**
	 * Parse off data point.
	 * @param Buffer Buffer to parse from.
	 * @param OutValue Output value.
	 * @return Bytes read.
	 */
	template< typename _OutType >
	int TryDecodeDataPoint( std::deque< BcU8 >& Buffer, BcU8 ID, _OutType& OutValue )
	{
		int BytesRead = 0;
		if( Buffer[ 0 ] == ID &&
			Buffer.size() >= ( 1 + sizeof( OutValue ) ) )
		{
			Buffer.pop_front();
			BytesRead++;

			// Multibyte data point.
			int BytesToParse = sizeof( _OutType );
			if( BytesToParse > 1 )
			{
				BytesToParse = Buffer[ 0 ];
				Buffer.pop_front();
				BytesRead++;
			}

			//
			BcU8* OutData = reinterpret_cast< BcU8* >( &OutValue );
			for( int Idx = 0; Idx < BytesToParse; ++Idx )
			{
				// Only actually write if we have space in output to.
				if( Idx < sizeof( OutValue ) )
				{
					*OutData++ = Buffer[ 0 ];
				}

				Buffer.pop_front();
				BytesRead++;
			}
		}
		return BytesRead;
	}

	/**
	 * Try to parse the payload.
	 * @param Buffer Buffer to parse from.
	 * @param OutEvent Event to write out to.
	 * @return true is successfully parsed. false if not..
	 */
	bool TryParsePayload( std::deque< BcU8 >& Buffer, OsEventInputMindwave& OutEvent )
	{
		bool SearchForSync = true;
		int PayloadSize = 0;
		const int MinimumSize = 3;
		while( SearchForSync && Buffer.size() >= MinimumSize )
		{
			// Search through until we hit sync.
			while( Buffer.size() > 0 && 
				Buffer[ 0 ] != ID_SYNC )
			{
				Buffer.pop_front();
			}

			// Verify that it is a sync (2 bytes of sync)
			if( Buffer.size() >= MinimumSize &&
				Buffer[ 0 ] == ID_SYNC && 
				Buffer[ 1 ] == ID_SYNC )
			{
				// Grab payload size.
				PayloadSize = Buffer[ 2 ];

				// Bail if payload size if ID_SYNC.
				if( PayloadSize == ID_SYNC )
				{
					Buffer.pop_front();
					continue;
				}

				// Search for sync is over.
				SearchForSync = false;

				// Check if we can actually try reading the payload in.
				if( ( Buffer.size() + MinimumSize ) >= PayloadSize )
				{
					// Pop off the sync tag + payload size.
					Buffer.pop_front();
					Buffer.pop_front();
					Buffer.pop_front();
				}
				else
				{
					return false;
				}
			}
		}

		// TODO: Parse.
		BcU8 ChecksumCalc = 0;
		BcAssert( ( PayloadSize + 1 ) < Buffer.size() );

		// Payload checksum.
		for( int Idx = 0; Idx < PayloadSize; ++Idx )
		{
			ChecksumCalc += Buffer[ Idx ];
		}

		struct EEGPowerValue
		{
			BcU8 Bytes_[ 3 ];
		};
		EEGPowerValue EEGPowerValue;
		BcU32 RawEEGValue = 0;

		BcPrintf( "Payload (%u): ", PayloadSize );
		for( int Idx = 0; Idx < PayloadSize ; ++Idx )
		{
			BcPrintf( "0x%x, ", Buffer[ Idx ] );
		}
		BcPrintf( "\n" );

		while( PayloadSize > 0 )
		{
			PayloadSize -= TryDecodeDataPoint( Buffer, ID_POOR_QUALITY, OutEvent.PoorSignal_ );
			BcAssert( PayloadSize > 0 );

			PayloadSize -= TryDecodeDataPoint( Buffer, ID_ATTENTION, OutEvent.Attention_ );
			BcAssert( PayloadSize > 0 );
	
			PayloadSize -= TryDecodeDataPoint( Buffer, ID_MEDITATION, OutEvent.Meditation_ );
			BcAssert( PayloadSize > 0 );
	
			PayloadSize -= TryDecodeDataPoint( Buffer, ID_BLINK_STRENGTH, OutEvent.Blink_ );
			BcAssert( PayloadSize > 0 );
	
			PayloadSize -= TryDecodeDataPoint( Buffer, ID_EEG_POWERS, EEGPowerValue );
			BcAssert( PayloadSize > 0 );
	
			PayloadSize -= TryDecodeDataPoint( Buffer, ID_RAW_EEG, RawEEGValue );
			BcAssert( PayloadSize > 0 );
		}

		ChecksumCalc ^= 0xff;
		BcU8 Checksum = Buffer[ 0 ];
		Buffer.pop_front();
		if( Checksum == ChecksumCalc )
		{
			return true;
		}

		return false;
	}

}

//////////////////////////////////////////////////////////////////////////
// Ctor
OsInputMindwaveLinux::OsInputMindwaveLinux():
	State_( State::INIT ),
	Shutdown_( 0 ),
	DeviceId_( 0 ),
	Socket_( 0 )
{
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
				auto RetVal = ::read( Socket_, &SingleByte, sizeof( SingleByte ) );
				if( RetVal >= 0 )
				{
					ByteBuffer_.push_back( SingleByte );

					// Only try to parse the payload once we've buffered a small amount.
					if( ByteBuffer_.size() > 128 )
					{
						bool SuccessfulParse = TryParsePayload( ByteBuffer_, Event_ );

						if( SuccessfulParse )
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
