#include "System/Debug/DsCoreImpl.h"
#include "System/Debug/DsCoreLogging.h"
#include "System/Debug/DsImGui.h"
#include "System/Debug/DsImGuiFieldEditor.h"
#include "System/Debug/DsTemplate.h"
#include "System/Debug/DsProfilerChromeTracing.h"

#include "Base/BcFile.h"
#include "Base/BcHtml.h"
#include "Base/BcProfiler.h"

#include "Math/MaQuat.h"
#include "Math/MaMat4d.h"

#include "Serialisation/SeJsonWriter.h"

#include "System/SysKernel.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnTexture.h"
#if USE_WEBBY
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#endif
#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( DsCoreImpl );

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCoreImpl::DsCoreImpl() :
#if USE_WEBBY
	ConnectionCount_( 0 ),
	ws_connections(),
#endif // USE_WEBBY
	DrawMenu_( false ),
	PanelFunctions_(),
	PageFunctions_(),
	ButtonFunctions_(),
	NextHandle_( 0 )
{
	// Start profiler.
#if PSY_USE_PROFILER
	if( GCommandLine_.hasArg( 'p', "profile" ) )
	{
		new DsProfilerChromeTracing();
	}
#endif


	using namespace std::placeholders;

	registerPage( "", { }, std::bind( &DsCoreImpl::cmdMenu, this, _1, _2, _3 ) );
	registerPage( "Content", { }, std::bind( &DsCoreImpl::cmdContent, this, _1, _2, _3 ), "Content" );
	registerPage( "Scene", { }, std::bind( &DsCoreImpl::cmdScene, this, _1, _2, _3 ), "Scene" );
	registerPage( "Log", { }, std::bind( &DsCoreImpl::cmdLog, this, _1, _2, _3 ), "Log" );
	registerPage( "Functions", { }, std::bind( &DsCoreImpl::cmdViewFunctions, this, _1, _2, _3 ) );

	registerPage( "Resource/(.*)", { "Id" }, std::bind( &DsCoreImpl::cmdResource, this, _1, _2, _3 ) );
	registerPage( "ResourceEdit/(.*)", { "Id" }, std::bind( &DsCoreImpl::cmdResourceEdit, this, _1, _2, _3 ) );
	registerPageNoHtml( "Json/(\\d*)", { "Id" }, std::bind( &DsCoreImpl::cmdJson, this, _1, _2, _3 ) );
	registerPageNoHtml( "JsonSerialise/(\\d*)", { "Id" }, std::bind( &DsCoreImpl::cmdJsonSerialiser, this, _1, _2, _3 ) );
	registerPageNoHtml( "Wadl", { }, std::bind( &DsCoreImpl::cmdWADL, this, _1, _2, _3 ) );

	// Setup key lookup.
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F1 ] = "F1";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F2 ] = "F2";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F3 ] = "F3";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F4 ] = "F4";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F5 ] = "F5";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F6 ] = "F6";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F7 ] = "F7";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F8 ] = "F8";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F9 ] = "F9";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F10 ] = "F10";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F11 ] = "F11";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F12 ] = "F12";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F13 ] = "F13";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F14 ] = "F14";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F15 ] = "F15";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F16 ] = "F16";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F17 ] = "F17";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F18 ] = "F18";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F19 ] = "F19";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F20 ] = "F20";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F21 ] = "F21";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F22 ] = "F22";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F23 ] = "F23";
	ShortcutLookup_[ OsEventInputKeyboard::KEYCODE_F24 ] = "F24";

	ShortcutLookup_[ 'A' ] = "A";
	ShortcutLookup_[ 'B' ] = "B";
	ShortcutLookup_[ 'C' ] = "C";
	ShortcutLookup_[ 'D' ] = "D";
	ShortcutLookup_[ 'E' ] = "E";
	ShortcutLookup_[ 'F' ] = "F";
	ShortcutLookup_[ 'G' ] = "G";
	ShortcutLookup_[ 'H' ] = "H";
	ShortcutLookup_[ 'I' ] = "I";
	ShortcutLookup_[ 'J' ] = "J";
	ShortcutLookup_[ 'K' ] = "K";
	ShortcutLookup_[ 'L' ] = "L";
	ShortcutLookup_[ 'M' ] = "M";
	ShortcutLookup_[ 'N' ] = "N";
	ShortcutLookup_[ 'O' ] = "O";
	ShortcutLookup_[ 'P' ] = "P";
	ShortcutLookup_[ 'Q' ] = "Q";
	ShortcutLookup_[ 'R' ] = "R";
	ShortcutLookup_[ 'S' ] = "S";
	ShortcutLookup_[ 'T' ] = "T";
	ShortcutLookup_[ 'U' ] = "U";
	ShortcutLookup_[ 'V' ] = "V";
	ShortcutLookup_[ 'W' ] = "W";
	ShortcutLookup_[ 'X' ] = "X";
	ShortcutLookup_[ 'Y' ] = "Y";
	ShortcutLookup_[ 'Z' ] = "Z";

	ShortcutLookup_[ '0' ] = "0";
	ShortcutLookup_[ '1' ] = "1";
	ShortcutLookup_[ '2' ] = "2";
	ShortcutLookup_[ '3' ] = "3";
	ShortcutLookup_[ '4' ] = "4";
	ShortcutLookup_[ '5' ] = "5";
	ShortcutLookup_[ '6' ] = "6";
	ShortcutLookup_[ '7' ] = "7";
	ShortcutLookup_[ '8' ] = "8";
	ShortcutLookup_[ '9' ] = "9";
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCoreImpl::~DsCoreImpl()
{
	OsCore::pImpl()->unsubscribeAll( this );
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void DsCoreImpl::open()
{
#if USE_WEBBY
	int memory_size;
	WebbyServerConfig config;

#if defined(_WIN32)
	{
		WORD wsa_version = MAKEWORD( 2, 2 );
		WSADATA wsa_data;
		if ( 0 != WSAStartup( wsa_version, &wsa_data ) )
		{
			PSY_LOG( "WSAStartup failed" );
			fprintf( stderr, "WSAStartup failed\n" );
			return;
		}
	}
#endif
	std::vector<std::string> bindAddresses = getIPAddresses();
	for ( int i = 0; i < bindAddresses.size(); ++i )
	{
		PSY_LOG( "Binding to %s", bindAddresses[ i ].c_str() );
		memset( &config, 0, sizeof config );
		config.bind_address = bindAddresses[ i ].c_str();
		config.listening_port = 1337;
		config.flags = WEBBY_SERVER_WEBSOCKETS;
		config.connection_max = 4;
		config.request_buffer_size = 2048;
		config.io_buffer_size = 8192;
		config.dispatch = &DsCoreImpl::externalWebbyDispatch;
		//config.log = &DsCoreImpl::externalWebbyLog;
		config.ws_connect = &DsCoreImpl::externalWebbyConnect;
		config.ws_connected = &DsCoreImpl::externalWebbyConnected;
		config.ws_closed = &DsCoreImpl::externalWebbyClosed;
		config.ws_frame = &DsCoreImpl::externalWebbyFrame;
		memory_size = WebbyServerMemoryNeeded( &config );
		void* TempMemory = malloc( memory_size );
		WebbyServer* TempServer_ = WebbyServerInit( &config, TempMemory, memory_size );

		if ( !TempServer_ )
		{
			PSY_LOG( "Failed to initialise Webby server" );
			fprintf( stderr, "failed to init server\n" );
		}
		if( TempServer_ != nullptr )
		{
			Servers_.push_back( TempServer_ );
		}
		ServerMemory_.push_back( TempMemory );
	}
#endif

	// Setup init/deinit hooks.
	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, this,
		[]( EvtID, const EvtBaseEvent& )
	{
		if( OsCore::pImpl()->getClient( 0 ) )
		{
			ImGui::Psybrus::Init();
		}
		return evtRET_REMOVE;
	} );

	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_CLOSE, this,
		[]( EvtID, const EvtBaseEvent& )
	{
		if( OsCore::pImpl()->getClient( 0 ) )
		{
			ImGui::Psybrus::Shutdown();
		}
		return evtRET_REMOVE;
	} );

	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, this,
		[ this ]( EvtID, const EvtBaseEvent& )
	{
		PSY_PROFILER_SECTION( UpdateImGui, "ImGui" );
		if( OsCore::pImpl()->getClient( 0 ) )
		{
			if ( ImGui::Psybrus::NewFrame() )
			{
				if ( DrawMenu_ )
				{
					// Do main menu bar.
					if( ImGui::BeginMainMenuBar() )
					{
						// Categorised panels.
						std::string LastCategory;
						bool MenuOpen = false;
						for ( auto& Panel : PanelFunctions_ )
						{
							if( Panel.Category_ != LastCategory )
							{
								if( MenuOpen && !LastCategory.empty() )
								{
									ImGui::EndMenu();
								}
								MenuOpen = ImGui::BeginMenu( Panel.Category_.c_str() );
								LastCategory = Panel.Category_;
							}
							if( MenuOpen )
							{
								if( ImGui::MenuItem( Panel.Name_.c_str(), Panel.Shortcut_.c_str(), nullptr ) )
								{
									Panel.IsVisible_ = !Panel.IsVisible_; 
								}
							}
						}
						if( MenuOpen && !LastCategory.empty() )
						{
							ImGui::EndMenu();
						}

						ImGui::EndMainMenuBar();
					}
				}

				for ( auto& Panel : PanelFunctions_ )
				{
					if( Panel.IsVisible_ )
					{
						Panel.Function_( Panel.Handle_ );
					}
				}
			}
		}
		return evtRET_PASS;
	} );

	DrawMenu_ = false;
#if PLATFORM_HTML5
	DrawMenu_ = true;
#endif

#if PLATFORM_ANDROID
		auto& Style = ImGui::GetStyle();
		Style.FramePadding.x *= 2.0f;
		Style.FramePadding.y *= 2.0f;
		Style.GrabMinSize *= 2.0f;
#endif

	// Setup toggle of debug panels.
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )
	{
		const auto& Event = InEvent.get< OsEventInputKeyboard >();

		// Handle toggling menu.
		if ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_F11 ||
			Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_VOLUME_UP ||
			Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_VOLUME_DOWN )
		{
			DrawMenu_ = !DrawMenu_;
		}

		CtrlModifier_ |= ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_CONTROL ) ? 1 : 0;
		AltModifier_ |= ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_ALT ) ? 1 : 0;
		ShiftModifier_ |= ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_SHIFT ) ? 1 : 0;

		// Check if a shortcut combo has been pressed.
		if( ShortcutLookup_.find( Event.KeyCode_ ) != ShortcutLookup_.end() )
		{
			// Check panels.
			for ( auto& Panel : PanelFunctions_ )
			{
				bool CtrlRequired = Panel.Shortcut_.find( "Ctrl" ) != std::string::npos;
				bool AltRequired = Panel.Shortcut_.find( "Alt" ) != std::string::npos;
				bool ShiftRequired = Panel.Shortcut_.find( "Shift" ) != std::string::npos;
			
				if( CtrlRequired == !!CtrlModifier_ &&
					AltRequired == !!AltModifier_ &&
					ShiftRequired == !!ShiftModifier_ )
				{
					std::string Shortcut = ShortcutLookup_[ Event.KeyCode_ ];
					if( CtrlRequired || AltRequired || ShiftRequired )
					{
						Shortcut = "+" + Shortcut;
					}

					PSY_LOG( "Shortcut: Ctrl %u, Alt %u, Shift %u, %s", CtrlModifier_, AltModifier_, ShiftModifier_, Panel.Shortcut_.c_str() );

					if( Panel.Shortcut_.find( Shortcut ) == Panel.Shortcut_.length() - Shortcut.length() )
					{
						Panel.IsVisible_ = !Panel.IsVisible_;
						return evtRET_BLOCK;
					}
				}
			}
		}

		return evtRET_PASS;
	} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )
	{
		auto Event = InEvent.get< OsEventInputKeyboard >();
		CtrlModifier_ &= ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_CONTROL ) ? 0 : 1;
		AltModifier_ &= ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_ALT ) ? 0 : 1;
		ShiftModifier_ &= ( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_SHIFT ) ? 0 : 1;
		return evtRET_PASS;
	} );

	// Setup debug attributes for reflection.
	setupReflectionEditorAttributes();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void DsCoreImpl::update()
{
#if USE_WEBBY
	for ( unsigned int Idx = 0; Idx < Servers_.size(); ++Idx )
	{
		PSY_PROFILER_SECTION( UpdateWebby, "WebbyServerUpdate" );

		WebbyServerUpdate( Servers_[ Idx ] );
	}
#endif // USE_WEBBY
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void DsCoreImpl::close()
{

#if USE_WEBBY
	for ( unsigned int Idx = 0; Idx < ServerMemory_.size(); ++Idx )
	{
		free( ServerMemory_[ Idx ] );
	}
#endif
}


#if USE_WEBBY

int DsCoreImpl::webbyDispatch( WebbyConnection *connection )
{
	int size = 0;
	char* data = new char[ connection->request.content_length + 1 ];
	BcMemZero( data, connection->request.content_length + 1 );
	WebbyRead( connection, data, connection->request.content_length );
	char* file = handleFile( connection->request.uri, size, data );
	WebbyBeginResponse( connection, 200, size, NULL, 0 );
	WebbyWrite( connection, file, size );
	WebbyEndResponse( connection );

	delete data;
	return 0;
}

int DsCoreImpl::webbyConnect( WebbyConnection *connection )
{
	/* Allow websocket upgrades on /wstest */
	if ( 0 == strcmp( connection->request.uri, "/wstest" ) && ConnectionCount_ < MAX_WSCONN )
		return 0;
	else
		return 1;
}

void DsCoreImpl::webbyConnected( WebbyConnection *connection )
{
	printf( "WebSocket connected\n" );
	ws_connections[ ConnectionCount_++ ] = connection;
}

void DsCoreImpl::webbyClosed( WebbyConnection *connection )
{
	int i;
	printf( "WebSocket closed\n" );

	for ( i = 0; i < ConnectionCount_; i++ )
	{
		if ( ws_connections[ i ] == connection )
		{
			int remain = ConnectionCount_ - i;
			memmove( ws_connections + i, ws_connections + i + 1, remain * sizeof( WebbyConnection * ) );
			--ConnectionCount_;
			break;
		}
	}
}

int DsCoreImpl::webbyFrame( WebbyConnection *connection, const WebbyWsFrame *frame )
{
	size_t i = 0;

	printf( "WebSocket frame incoming\n" );
	printf( "  Frame OpCode: %d\n", frame->opcode );
	printf( "  Final frame?: %s\n", ( frame->flags & WEBBY_WSF_FIN ) ? "yes" : "no" );
	printf( "  Masked?     : %s\n", ( frame->flags & WEBBY_WSF_MASKED ) ? "yes" : "no" );
	printf( "  Data Length : %d\n", ( int ) frame->payload_length );

	while ( i < frame->payload_length )
	{
		unsigned char buffer[ 16 ];
		size_t remain = frame->payload_length - i;
		size_t read_size = remain >( int ) sizeof buffer ? sizeof buffer : ( size_t ) remain;
		size_t k;

		printf( "%08x ", ( int ) i );

		if ( 0 != WebbyRead( connection, buffer, read_size ) )
			break;

		for ( k = 0; k < read_size; ++k )
			printf( "%02x ", buffer[ k ] );

		for ( k = read_size; k < 16; ++k )
			printf( "   " );

		printf( " | " );

		for ( k = 0; k < read_size; ++k )
			printf( "%c", isprint( buffer[ k ] ) ? buffer[ k ] : '?' );

		printf( "\n" );

		i += read_size;
	}

	return 0;
}
int DsCoreImpl::externalWebbyDispatch( WebbyConnection *connection )
{
	return static_cast<DsCoreImpl*>( DsCore::pImpl() )->webbyDispatch( connection );
}

int DsCoreImpl::externalWebbyConnect( WebbyConnection *connection )
{
	return static_cast<DsCoreImpl*>( DsCore::pImpl() )->webbyConnect( connection );
}

void DsCoreImpl::externalWebbyConnected( WebbyConnection *connection )
{
	return static_cast<DsCoreImpl*>( DsCore::pImpl() )->webbyConnected( connection );
}

void DsCoreImpl::externalWebbyClosed( WebbyConnection *connection )
{
	return static_cast<DsCoreImpl*>( DsCore::pImpl() )->webbyClosed( connection );
}

int DsCoreImpl::externalWebbyFrame( WebbyConnection *connection, const WebbyWsFrame *frame )
{
	return static_cast<DsCoreImpl*>( DsCore::pImpl() )->webbyFrame( connection, frame );
}

std::vector< std::string > DsCoreImpl::getIPAddresses()
{
	std::vector< std::string > result;
#if !PLATFORM_WINPHONE
	result.push_back( "127.0.0.1" );
	RakNet::RakPeerInterface* peer = NULL;
	peer = RakNet::RakPeerInterface::GetInstance();

	int port = 1337;

	RakNet::SocketDescriptor sd( port, 0 );

	peer->Startup( 1, &sd, 1 );

	unsigned int addressCount = peer->GetNumberOfAddresses();
	for ( unsigned int Idx = 0; Idx < addressCount; ++Idx )
	{
		result.push_back( peer->GetLocalIP( Idx ) );
	}
	peer->Shutdown( 500, 0, LOW_PRIORITY );
	RakNet::RakPeerInterface::DestroyInstance( peer );
#endif
	return result;
}
#endif // USE_WEBBY

//////////////////////////////////////////////////////////////////////////
// registerPanel
BcU32 DsCoreImpl::registerPanel( const char* Category, const char* Name, const char* Shortcut, std::function < void( BcU32 )> Func )
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;
	PanelFunctions_.emplace_back( Category, Name, Shortcut, Func, Handle );
	std::sort( PanelFunctions_.begin(), PanelFunctions_.end(), 
		[]( const DsPanelDefinition& A, const DsPanelDefinition& B ) 
		{
			return A.Name_ < B.Name_;
		} );

	return Handle;

}

//////////////////////////////////////////////////////////////////////////
// deregisterPanel
void DsCoreImpl::deregisterPanel( BcU32 Handle )
{
	auto FoundIt = std::find_if( PanelFunctions_.begin(), PanelFunctions_.end(),
		[ &]( DsPanelDefinition& PanelDef )
	{
		return PanelDef.Handle_ == Handle;
	} );

	if ( FoundIt != PanelFunctions_.end() )
	{
		PanelFunctions_.erase( FoundIt );
	}
	BcAssert( BcIsGameThread() );
}

//////////////////////////////////////////////////////////////////////////
// drawObjectEditor
void DsCoreImpl::drawObjectEditor( DsImGuiFieldEditor* ThisFieldEditor, void* Data, const ReClass* Class, BcU32 Flags )
{
	BcAssert( Class );
	std::string Name = *Class->getName();
	if ( Class->hasBaseClass( ReObject::StaticGetClass() ) )
	{
		ReObject* Object = static_cast< ReObject* >( Data );
		Class = Object->getClass();
		Name = *Object->getName() + " (" + *Class->getName() + ")";
	}
	ImGui::BulletText( "%s", Name.c_str() );
	ImGui::ScopedID ScopedIDData( Data );
	ImGui::ScopedID ScopedIDClass( Class );
	ImGui::ScopedIndent ScopedIndent;

	// List fields.
	while ( Class != nullptr )
	{
		for ( size_t FieldIdx = 0; FieldIdx < Class->getNoofFields(); ++FieldIdx )
		{
			const auto* Field = Class->getField( FieldIdx );
			ImGui::ScopedID ScopedID( Field );

			ReFieldAccessor FieldAccessor( Data, Field );
			if ( !FieldAccessor.isTransient() &&
				!FieldAccessor.isConst() )
			{
				if ( !FieldAccessor.isContainerType() )
				{
					auto UpperFieldType = FieldAccessor.getUpperClass();
					auto FieldType = UpperFieldType;

					// Find editor.
					DsImGuiFieldEditor* FieldEditor = DsImGuiFieldEditor::Get( Field );
					auto Value = FieldAccessor.getData();
					if ( Value != nullptr )
					{
						if ( FieldEditor )
						{
							if ( ImGui::TreeNode( Value, "%s", ( *Field->getName() ).c_str() ) )
							{
								FieldEditor->onEdit( " ", Value, UpperFieldType,
									ReFieldFlags( FieldAccessor.getFlags() | ( Flags & bcRFF_CONST ) ) );
								ImGui::TreePop();
							}
						}
					}
				}
				else
				{
					if ( ImGui::TreeNode( Field, "%s", ( *Field->getName() ).c_str() ) )
					{
						if ( Field->getKeyType() == nullptr )
						{
							ReContainerAccessor::ReadIteratorUPtr It( FieldAccessor.newReadIterator() );
							BcU32 Idx = 0;							
							for ( ; It->isValid(); It->next() )
							{
								void* Value = nullptr;
								if ( FieldAccessor.isContainerOfPointerValues() )
								{
									Value = *reinterpret_cast< void** >( It->getValue() );
								}
								else
								{
									Value = It->getValue();
								}
								if ( Value != nullptr )
								{
									auto UpperValueType = FieldAccessor.getValueUpperClass( Value );
									auto ValueType = UpperValueType;

									DsImGuiFieldEditor* FieldEditor = DsImGuiFieldEditor::Get( ValueType );
									if ( FieldEditor )
									{
										std::string ItemName = *UpperValueType->getName();
										if ( UpperValueType->hasBaseClass( ReObject::StaticGetClass() ) )
										{
											ReObject* Object = static_cast< ReObject* >( Value );
											ItemName = *Object->getName() + " (" + *Object->getClass()->getName() + ")";
										}
										ImGui::ScopedID ScopedID( Idx );
										if ( ImGui::TreeNode( Value, "[%u] %s", Idx++, ItemName.c_str() ) )
										{
											FieldEditor->onEdit( " ", Value, UpperValueType,
												ReFieldFlags( FieldAccessor.getFlags() | ( Flags & bcRFF_CONST ) ) );
											ImGui::TreePop();
										}
									}
								}
							}
						}
						else
						{
							ReContainerAccessor::ReadIteratorUPtr It( FieldAccessor.newReadIterator() );
							BcU32 Idx = 0;
							for ( ; It->isValid(); It->next() )
							{
								void* Key = nullptr;
								void* Value = nullptr;
								if ( FieldAccessor.isContainerOfPointerKeys() )
								{
									Key = *reinterpret_cast< void** >( It->getKey() );
								}
								else
								{
									Key = It->getKey();
								}

								if ( FieldAccessor.isContainerOfPointerValues() )
								{
									Value = *reinterpret_cast< void** >( It->getValue() );
								}
								else
								{
									Value = It->getValue();
								}


								if ( Key != nullptr && Value != nullptr )
								{
									auto UpperKeyType = FieldAccessor.getKeyUpperClass( Key );
									auto KeyType = UpperKeyType;
									auto UpperValueType = FieldAccessor.getValueUpperClass( Value );
									auto ValueType = UpperValueType;

									DsImGuiFieldEditor* KeyFieldEditor = DsImGuiFieldEditor::Get( KeyType );
									DsImGuiFieldEditor* ValueFieldEditor = DsImGuiFieldEditor::Get( ValueType );

									if ( KeyFieldEditor || ValueFieldEditor )
									{
										std::string ShortName = " ";
										ImGui::ScopedID ScopedID( Idx );
										if ( ImGui::TreeNode( Value, "[%u] %s", Idx++, ShortName.c_str() ) )
										{
											if ( KeyFieldEditor )
											{
												KeyFieldEditor->onEdit( " ", Key, UpperKeyType,
													ReFieldFlags( FieldAccessor.getFlags() | ( Flags & bcRFF_CONST ) ) );
											}
											else
											{
												ImGui::Text( "No key editor" );
											}

											if ( ValueFieldEditor )
											{
												ValueFieldEditor->onEdit( "", Value, UpperValueType,
													ReFieldFlags( FieldAccessor.getFlags() | ( Flags & bcRFF_CONST ) ) );
											}
											else
											{
												ImGui::Text( "No value editor" );
											}
											ImGui::TreePop();
										}
									}
								}
							}
						}
						ImGui::TreePop();
					}
				}
			}
		}
		Class = Class->getSuper();
	}
}

//////////////////////////////////////////////////////////////////////////
// registerFunction
BcU32 DsCoreImpl::registerFunction( std::string Display, std::function<void()> Function )
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;
	ButtonFunctions_.push_back( DsFunctionDefinition( Display, Function, Handle ) );
	PSY_LOG( "Function registered." );
	PSY_LOG( "\t%s (%u)", Display.c_str(), Handle );
	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCoreImpl::deregisterFunction( BcU32 Handle )
{
	BcAssert( BcIsGameThread() );
	BcBool functionRemoved = false;
	for ( auto iter = ButtonFunctions_.begin(); iter != ButtonFunctions_.end(); ++iter )
	{
		if ( ( *iter ).Handle_ == Handle )
		{
			PSY_LOG( "Function deregistered." );
			PSY_LOG( "\t%s (%u)", ( *iter ).DisplayText_.c_str(), Handle );
			ButtonFunctions_.erase( iter );
			functionRemoved = true;
			break;
		}
	}
	if ( !functionRemoved )
	{
		PSY_LOG( "Function deregister failed." );
		PSY_LOG( "\tHandle: %u", Handle );

	}
}

//////////////////////////////////////////////////////////////////////////
// registerPage
BcU32 DsCoreImpl::registerPage( std::string regex, std::vector<std::string> namedCaptures, std::function < void( DsParameters, BcHtmlNode&, std::string )> fn, std::string display )
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;

	DsPageDefinition cm( regex, namedCaptures, display );
	cm.Function_ = fn;
	PageFunctions_.push_back( cm );
	if( DsCoreLogging::pImpl() )
	{
		DsCoreLogging::pImpl()->addLog( "DsCore", rand(), "Registering page: " + display );
	}
	PSY_LOG( "Registered page" );
	PSY_LOG( "\t%s (%s)", regex.c_str(), display.c_str() );

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPage
BcU32 DsCoreImpl::registerPage( std::string regex, std::vector<std::string> namedCaptures, std::function < void( DsParameters, BcHtmlNode&, std::string )> fn )
{
	++NextHandle_;
	BcU32 Handle = NextHandle_;

	DsPageDefinition cm( regex, namedCaptures );
	cm.Function_ = fn;
	cm.IsHtml_ = true;
	PageFunctions_.push_back( cm );
	PSY_LOG( "Registered page (No content index)" );
	PSY_LOG( "\t%s (%u)", regex.c_str(), Handle );

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPageNoHtml
BcU32 DsCoreImpl::registerPageNoHtml( std::string regex, std::vector<std::string> namedCaptures, std::function < void( DsParameters, BcHtmlNode&, std::string )> fn )
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;

	DsPageDefinition cm( regex, namedCaptures );
	cm.Function_ = fn;
	cm.IsHtml_ = false;
	PageFunctions_.push_back( cm );
	PSY_LOG( "Registered page without html" );
	PSY_LOG( "\t%s (%u)", regex.c_str(), Handle );

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// deregisterPage
void DsCoreImpl::deregisterPage( BcU32 Handle )
{
	BcAssert( BcIsGameThread() );
	BcBool pageRemoved = false;
	for ( auto iter = PageFunctions_.begin(); iter != PageFunctions_.end(); ++iter )
	{
		if ( ( *iter ).Handle_ == Handle )
		{
			PageFunctions_.erase( iter );
			pageRemoved = true;
			break;
		}
	}
	PSY_LOG( "Page deregistration failed." );
	PSY_LOG( "\tHandle: %u", Handle );

}

//////////////////////////////////////////////////////////////////////////
// setupReflectionEditorAttributes
void DsCoreImpl::setupReflectionEditorAttributes()
{
	// Add some custom editors.
	ReManager::GetClass( "BcU8" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcU8* Value = ( BcU8* ) Object;
			int ValueInt = *Value;
			if ( ImGui::InputInt( Name.c_str(), &ValueInt ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ( BcU8 ) ValueInt;
			}
		} ) );

	ReManager::GetClass( "BcS8" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcS8* Value = ( BcS8* ) Object;
			int ValueInt = *Value;
			if ( ImGui::InputInt( Name.c_str(), &ValueInt ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ( BcS8 ) ValueInt;
			}
		} ) );

	ReManager::GetClass( "BcU16" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcU16* Value = ( BcU16* ) Object;
			int ValueInt = *Value;
			if ( ImGui::InputInt( Name.c_str(), &ValueInt ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ( BcU16 ) ValueInt;
			}
		} ) );

	ReManager::GetClass( "BcS16" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcS16* Value = ( BcS16* ) Object;
			int ValueInt = *Value;
			if ( ImGui::InputInt( Name.c_str(), &ValueInt, 1, 100 ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ( BcS16 ) ValueInt;
			}
		} ) );

	ReManager::GetClass( "BcU32" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcU32* Value = ( BcU32* ) Object;
			int ValueInt = *Value;
			if ( ImGui::InputInt( Name.c_str(), &ValueInt, 1, 100 ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ( BcU32 ) ValueInt;
			}
		} ) );

	ReManager::GetClass( "BcS32" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcS32* Value = ( BcS32* ) Object;
			int ValueInt = *Value;
			if ( ImGui::InputInt( Name.c_str(), &ValueInt, 1, 100 ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ( BcS32 ) ValueInt;
			}
		} ) );

	ReManager::GetClass( "BcBool" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcBool* Value = ( BcBool* ) Object;
			bool ValueBool = *Value ? true : false;
			if ( ImGui::Checkbox( Name.c_str(), &ValueBool ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ValueBool ? BcTrue : BcFalse;
			}
		} ) );

	ReManager::GetClass( "bool" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			bool* Value = ( bool* ) Object;
			bool ValueBool = *Value;
			if ( ImGui::Checkbox( Name.c_str(), &ValueBool ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ValueBool;
			}
		} ) );

	ReManager::GetClass( "BcF32" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcF32* Value = ( BcF32* ) Object;
			BcF32 ValueF32 = *Value;
			if ( ImGui::InputFloat( Name.c_str(), &ValueF32, 0.1f, 1.0f ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ValueF32;
			}
		} ) );

	ReManager::GetClass( "BcF64" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcF64* Value = ( BcF64* ) Object;
			float ValueF32 = *Value;
			if ( ImGui::InputFloat( Name.c_str(), &ValueF32, 0.1f, 1.0f ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = ValueF32;
			}
		} ) );

	ReManager::GetClass( "string" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			std::string* Value = ( std::string* )Object;
			char Buffer[ 1024 ] = { 0 };
			BcStrCopy( Buffer, BcArraySize( Buffer ) - 1, Value->c_str() );
			if ( ImGui::InputText( Name.c_str(), Buffer, BcArraySize( Buffer ) ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = Buffer;
			}
		} ) );

	ReManager::GetClass( "BcName" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			BcName* Value = ( BcName* ) Object;
			char Buffer[ 1024 ] = { 0 };
			BcStrCopy( Buffer, BcArraySize( Buffer ), ( **Value ).c_str() );
			if ( ImGui::InputText( Name.c_str(), Buffer, BcArraySize( Buffer ) ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				*Value = Buffer;
			}
		} ) );

	ReManager::GetClass( "MaVec2d" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaVec2d* Value = ( MaVec2d* ) Object;
			float Array[ 2 ] = { Value->x(), Value->y() };
			if ( ImGui::InputFloat2( Name.c_str(), Array ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value->x( Array[ 0 ] );
				Value->y( Array[ 1 ] );
			}
		} ) );

	ReManager::GetClass( "MaVec3d" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaVec3d* Value = ( MaVec3d* ) Object;
			float Array[ 3 ] = { Value->x(), Value->y(), Value->z() };
			if ( ImGui::InputFloat3( Name.c_str(), Array ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value->x( Array[ 0 ] );
				Value->y( Array[ 1 ] );
				Value->z( Array[ 2 ] );
			}
		} ) );

	ReManager::GetClass( "MaVec4d" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaVec4d* Value = ( MaVec4d* ) Object;
			float Array[ 4 ] = { Value->x(), Value->y(), Value->z(), Value->w() };
			if ( ImGui::InputFloat4( Name.c_str(), Array ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value->x( Array[ 0 ] );
				Value->y( Array[ 1 ] );
				Value->z( Array[ 2 ] );
				Value->w( Array[ 3 ] );
			}
		} ) );

	ReManager::GetClass( "MaAABB" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaAABB* Value = ( MaAABB* ) Object;
			float ArrayMin[ 3 ] = { Value->min().x(), Value->min().y(), Value->min().z() };
			float ArrayMax[ 3 ] = { Value->max().x(), Value->max().y(), Value->max().z() };
			if ( ImGui::InputFloat3( "Min", ArrayMin ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value->min( MaVec3d( ArrayMin[ 0 ], ArrayMin[ 1 ], ArrayMin[ 2 ] ) );
			}
			if ( ImGui::InputFloat3( "Max", ArrayMax ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value->max( MaVec3d( ArrayMax[ 0 ], ArrayMax[ 1 ], ArrayMax[ 2 ] ) );
			}
		} ) );

	ReManager::GetClass( "MaQuat" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaQuat* Value = ( MaQuat* ) Object;
			float Array[ 4 ] = { Value->x(), Value->y(), Value->z(), Value->w() };
			ImGui::Text( "%s", Name.c_str() );
			if ( ImGui::InputFloat4( "Values", Array ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value->x( Array[ 0 ] );
				Value->y( Array[ 1 ] );
				Value->z( Array[ 2 ] );
				Value->w( Array[ 3 ] );
			}
		} ) );

	ReManager::GetClass( "MaMat4d" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaMat4d& Value = *( MaMat4d* ) Object;
			float* Array = ( float* ) &Value;
			if ( ImGui::InputFloat4( "Row0", &Array[ 0 ] ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value[ 0 ][ 0 ] = Array[ 0 ];
				Value[ 0 ][ 1 ] = Array[ 1 ];
				Value[ 0 ][ 2 ] = Array[ 2 ];
				Value[ 0 ][ 3 ] = Array[ 3 ];
			}
			if ( ImGui::InputFloat4( "Row1", &Array[ 4 ] ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value[ 1 ][ 0 ] = Array[ 4 ];
				Value[ 1 ][ 1 ] = Array[ 5 ];
				Value[ 1 ][ 2 ] = Array[ 6 ];
				Value[ 1 ][ 3 ] = Array[ 7 ];
			}
			if ( ImGui::InputFloat4( "Row2", &Array[ 8 ] ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value[ 2 ][ 0 ] = Array[ 8 ];
				Value[ 2 ][ 1 ] = Array[ 9 ];
				Value[ 2 ][ 2 ] = Array[ 10 ];
				Value[ 2 ][ 3 ] = Array[ 11 ];
			}
			if ( ImGui::InputFloat4( "Row3", &Array[ 12 ] ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
			{
				Value[ 3 ][ 0 ] = Array[ 12 ];
				Value[ 3 ][ 1 ] = Array[ 13 ];
				Value[ 3 ][ 2 ] = Array[ 14 ];
				Value[ 3 ][ 3 ] = Array[ 15 ];
			}
		} ) );

	ReManager::GetClass( "RsColour" )->addAttribute(
		new DsImGuiFieldEditor(
		[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
		{
			MaVec4d* Value = ( MaVec4d* ) Object;
			{
				ImGui::ScopedID ScopedID( "RGB" );
				ImGui::ColorEditMode( ImGuiColorEditMode_RGB );
				float Array[ 4 ] = { Value->x(), Value->y(), Value->z(), Value->w() };
				if ( ImGui::ColorEdit4( Name.c_str(), Array ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
				{
					Value->x( Array[ 0 ] );
					Value->y( Array[ 1 ] );
					Value->z( Array[ 2 ] );
					Value->w( Array[ 3 ] );
				}
			}
			{
				ImGui::ScopedID ScopedID( "HSV" );
				ImGui::ColorEditMode( ImGuiColorEditMode_HSV );
				float Array[ 4 ] = { Value->x(), Value->y(), Value->z(), Value->w() };
				if ( ImGui::ColorEdit4( Name.c_str(), Array ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
				{
					Value->x( Array[ 0 ] );
					Value->y( Array[ 1 ] );
					Value->z( Array[ 2 ] );
					Value->w( Array[ 3 ] );
				}
			}
		} ) );

	ReManager::GetClass( "ReObject" )->addAttribute(
		new DsImGuiFieldEditor(
		[ this ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* ObjectData, const ReClass* Class, ReFieldFlags Flags )
		{
			drawObjectEditor( ThisFieldEditor, ObjectData, Class, Flags );
		} ) );


	// Setup editors for enums.
	// TODO: Share for *all* enums somehow?
	auto Enums = ReManager::GetEnums();
	for( auto* Enum : Enums )
	{
		Enum->addAttribute(
			new DsImGuiFieldEditor(
			[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				const ReEnum* Enum = static_cast< const ReEnum* >( Class );

				// Grab value.
				bool SetValue = false;
				BcU32 EnumValue = 0;
				switch( Enum->getSize() )
				{
				case 1:
					EnumValue = BcU32( *(BcU8*)Object );
					break;
				case 2:
					EnumValue = BcU32( *(BcU16*)Object );
					break;
				case 4:
					EnumValue = BcU32( *(BcU32*)Object );
					break;
				default:
					BcBreakpoint;
					break;
				}

				if( ( Flags & bcRFF_FLAGS ) == bcRFF_FLAGS )
				{
					// If flags, we need to give checkbox options.
					for( auto EnumConstant : Enum->getEnumConstants() )
					{
						const auto EnumConstantName = (*EnumConstant->getName());
						const auto BitValue = EnumConstant->getValue();
						if( BitValue )
						{
							std::array< char, 1024 > NameBuffer = { 0 };
							BcSPrintf( NameBuffer.data(), NameBuffer.size() - 1, "%s (%x)", EnumConstantName.c_str(), BitValue );
							bool IsSet = ( EnumValue & BitValue ) == BitValue;
							if( ImGui::Checkbox( NameBuffer.data(), &IsSet ) )
							{
								SetValue = true;
								EnumValue = ( EnumValue & ~BitValue ) | ( IsSet ? BitValue : 0 );
							}
						}
					}
				}
				else
				{
					// If not flags, drop down menu.
					std::array< char, 4096 > ComboBuffer = { 0 };
					size_t ComboPosition = 0;
					int CurrentItem = 0;
					int Item = 0;
					for( auto EnumConstant : Enum->getEnumConstants() )
					{
						const auto EnumConstantName = (*EnumConstant->getName());
						BcAssert( ComboPosition < ComboBuffer.size() );
						BcStrCopy( &ComboBuffer[ ComboPosition ], ComboBuffer.size() - ComboPosition, EnumConstantName.c_str() );
						ComboPosition += EnumConstantName.size() + 1;

						if( EnumConstant->getValue() == EnumValue )
						{
							Item = CurrentItem;
						}

						++CurrentItem;
					}
					if( ImGui::Combo( "Value", &Item, ComboBuffer.data() ) )
					{
						SetValue = true;
						EnumValue = Enum->getEnumConstants()[ Item ]->getValue();
					}
				}

				if( SetValue )
				{
					switch( Enum->getSize() )
					{
					case 1:
						*(BcU8*)Object = BcU8( EnumValue );
						break;
					case 2:
						*(BcU16*)Object = BcU16( EnumValue );
						break;
					case 4:
						*(BcU32*)Object = BcU32( EnumValue );
						break;
					default:
						BcBreakpoint;
						break;
					}

				}
			} ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// handleFile
char* DsCoreImpl::handleFile( std::string Uri, int& FileSize, std::string PostContent )
{
	std::string type;
	char* Output;
	if ( BcStrStr( Uri.c_str(), "/files/" ) )
	{
		Output = writeFile( &Uri[ 7 ], FileSize, type );
		return Output;
	}
	else
	{
		std::string out = loadHtmlFile( Uri, PostContent );
		FileSize = ( int ) out.length();
		Output = new char[ FileSize + 1 ];
		BcMemSet( Output, 0, FileSize + 1 );
		BcMemCopy( Output, &out[ 0 ], FileSize );
		return Output;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Gets a plain text file
std::string DsCoreImpl::loadTemplateFile( std::string Filename )
{
	BcFile File;
	std::string F = Filename;
	File.open( F.c_str() );
	if ( !File.isOpen() )
		return 0;
	auto Data = File.readAllBytes();
	std::string Output = ( const char* ) Data.get();
	return Output;
}


std::string DsCoreImpl::loadHtmlFile( std::string Uri, std::string Content )
{
	BcHtml HtmlContent;

	DsTemplate::loadTemplate( HtmlContent.getRootNode(), "Content/Debug/fullpage_template.html" );
	HtmlContent.getRootNode().findNodeById( "id-title" ).setContents( GPsySetupParams.Name_ );

	std::vector<std::string> data;
	bool success = false;
	std::string uri = &Uri[ 1 ];

	for ( auto Item : ButtonFunctions_ )
	{
		if ( uri == ( "Functions/" + std::to_string( Item.Handle_ ) ) )
		{
			BcHtmlNode redirect = HtmlContent.getRootNode().findNodeById( "meta" );
			redirect.setAttribute( "http-equiv", "refresh" );
			redirect.setAttribute( "content", "0; url=/" );
			Item.Function_();
			success = true;
		}
	}

	if ( !success )
	{
		for ( int Idx = ( int ) PageFunctions_.size() - 1; Idx >= 0; --Idx )
		{
			std::cmatch match;
			std::regex_match( &Uri[ 1 ], match, PageFunctions_[ Idx ].Regex_ );
			if ( match.size() > 0 )
			{
				std::string javaScript = "var params = [";
				for ( BcU32 Idx2 = 1; Idx2 < match.size(); ++Idx2 )
				{
					std::string u = match[ Idx2 ];
					data.push_back( u );
					if ( Idx2 > 1 )
						javaScript += ",";
					javaScript += "\n\"";
					javaScript += u;
					javaScript += "\"";
				}
				javaScript += "];";
				HtmlContent.getRootNode().findNodeById( "js-params" ).setContents( javaScript );
				auto innerBodyNode = HtmlContent.getRootNode().findNodeById( "innerBody" );
				PageFunctions_[ Idx ].Function_( data, innerBodyNode, Content );
				if ( !PageFunctions_[ Idx ].IsHtml_ )
					return HtmlContent.getRootNode().findNodeById( "innerBody" ).getContents();
				break;
			}
		}
	}

	std::string Output = HtmlContent.getHtml();
	return Output;
}

//////////////////////////////////////////////////////////////////////////
// writeHeader
void DsCoreImpl::writeHeader( BcHtmlNode& Output )
{
	BcHtmlNode header = Output.createChildNode( "div" ).setAttribute( "id", "headerSection" );
	header.createChildNode( "h1" ).setContents( GPsySetupParams.Name_ );

	BcHtmlNode link = Output.createChildNode( "div" ).setAttribute( "id", "headerLink" );
	link.createChildNode( "a" ).setAttribute( "href", "/" ).setContents( "Menu" ).setAttribute( "id", "headerLinkText" );

}

//////////////////////////////////////////////////////////////////////////
// writeFooter
void DsCoreImpl::writeFooter( BcHtmlNode& Output )
{
	BcHtmlNode footer = Output.createChildNode( "div" ).setAttribute( "id", "footer" ).setContents( "Footer" );
}

//////////////////////////////////////////////////////////////////////////
// Gets a file for the output stream
char* DsCoreImpl::writeFile( std::string Filename, int& OutLength, std::string& Type )
{
	BcFile File;
	std::string F = "Content/Debug/";
	F += Filename;
	File.open( F.c_str() );
	if ( !File.isOpen() )
		return 0;
	auto Data = File.readAllBytes();
	OutLength = ( int ) File.size();
	Type = "css";
	// TODO: Actually load files
	return ( char* ) Data.release();
}

//////////////////////////////////////////////////////////////////////////
// cmdMenu
void DsCoreImpl::cmdMenu( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	DsTemplate::loadTemplate( Output, "Content/Debug/main_items.html" );

	BcHtmlNode ul = Output.findNodeById( "page_listing" );

	for ( BcU32 Idx = 0; Idx < PageFunctions_.size(); ++Idx )
	{
		if ( PageFunctions_[ Idx ].Visible_ )
		{
			BcHtmlNode a = ul.createChildNode( "li" ).createChildNode( "a" );
			a.setAttribute( "href", PageFunctions_[ Idx ].Text_ );
			a.setContents( PageFunctions_[ Idx ].Display_ );
		}
	}
	BcHtmlNode functions = Output.findNodeById( "function_listing" );
	for ( auto Item : ButtonFunctions_ )
	{
		BcHtmlNode ahref = functions.createChildNode( "a" );
		std::string v = std::to_string( Item.Handle_ );
		ahref.setAttribute( "href", "Functions/" + v );

		BcHtmlNode button = ahref.createChildNode( "button" );
		button.setAttribute( "type", "button" );
		button.setContents( Item.DisplayText_ );
		functions.createChildNode( "br" );
	}
	/**/
}


//////////////////////////////////////////////////////////////////////////
// cmdContent
void DsCoreImpl::cmdContent( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	BcHtmlNode node = DsTemplate::loadTemplate( Output, "Content/Debug/contents_template.html" );
	node.findNodeById( "id-resources" ).setContents( std::to_string( CsCore::pImpl()->getNoofResources() ) );

	BcHtmlNode table = node.findNodeById( "id-table" );

	for ( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResource( Idx ) );
		BcHtmlNode row = DsTemplate::loadTemplate( table, "Content/Debug/content_row_template.html" );

		std::string id = std::to_string( Resource->getUniqueId() );
		row.findNodeById( "id-link" ).setAttribute( "href", "Resource/" + id ).setContents( *Resource->getName() );
		row.findNodeById( "id-name" ).setContents( *Resource->getClass()->getName() );
		row.findNodeById( "id-package-name" ).setContents( *Resource->getPackageName() );

	}
}

//////////////////////////////////////////////////////////////////////////
// cmdScene
void DsCoreImpl::cmdScene( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	BcU32 Idx = 0;
	while ( ScnEntityRef Entity = ScnCore::pImpl()->getEntity( Idx++ ) )
	{
		if ( Entity->getParentEntity() == NULL )
		{
			cmdScene_Entity( Entity, Output, 0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Entity
void DsCoreImpl::cmdScene_Entity( ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth )
{
	BcHtmlNode ul = Output.createChildNode( "ul" );
	BcChar Id[ 32 ] = { 0 };
	BcSPrintf( Id, sizeof( Id ) - 1, "%d", Entity->getUniqueId() );

	// Entity name.
	BcHtmlNode li = ul.createChildNode( "li" );
	li.setContents( "Entity; " );
	BcHtmlNode a = li.createChildNode( "a" );
	a.setAttribute( "href", "/Resource/" + std::string( Id ) );
	a.setContents( *Entity->getName() );

	for ( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Entity->getComponent( Idx ) );

		if ( Component->isTypeOf< ScnEntity >() )
		{
			cmdScene_Entity( ScnEntityRef( Component ), li, Depth + 1 );
		}
		else
		{
			cmdScene_Component( Component, li, Depth + 1 );
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Component
void DsCoreImpl::cmdScene_Component( ScnComponentRef Component, BcHtmlNode& Output, BcU32 Depth )
{
	BcChar Id[ 32 ] = { 0 };
	BcSPrintf( Id, sizeof( Id ) - 1, "%d", Component->getUniqueId() );
	BcHtmlNode tmp = DsTemplate::loadTemplate( Output, "Content/Debug/scene_component_template.html" );

	tmp.findNodeById( "component-link" ).setAttribute( "href", "/Resource/" + std::string( Id ) );
	tmp.findNodeById( "component-link" ).setContents( *Component->getName() );

}


//////////////////////////////////////////////////////////////////////////
// cmdLog
void DsCoreImpl::cmdLog( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	/*
	BcLog* log = BcLog::pImpl();

	BcHtmlNode ul = Output.createChildNode("ul");
	std::vector<std::string> logs = log->getLogData();
	for (auto val : logs)
	{
	ul.createChildNode("li").setContents(val);
	}//*/
	if( DsCoreLogging::pImpl() )
	{
		BcHtmlNode ul = Output.createChildNode( "ul" );
		std::vector< DsCoreLogEntry > logs = DsCoreLogging::pImpl()->getEntries( nullptr, 0 );

		for ( auto val : logs )
		{
			ul.createChildNode( "li" ).setContents( val.Entry_ ).setAttribute( "id", "Log-" + val.Category_ );
		}//*/
	}
}

//////////////////////////////////////////////////////////////////////////
// cmdResource
void DsCoreImpl::cmdResource( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	std::string EntityId = "";

	EntityId = params[ 0 ];
	if ( !BcStrIsNumber( EntityId.c_str() ) )
	{
		Output.createChildNode( "" ).setContents( "Invalid resource Id" );
		Output.createChildNode( "br" );
		return;
	}
	BcU32 id = BcStrAtoi( EntityId.c_str() );

	ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResourceByUniqueId( id ) );

	if ( Resource == nullptr )
	{
		Output.createChildNode( "" ).setContents( "Invalid resource Id" );
		Output.createChildNode( "br" );
		return;
	}

	if ( Resource->getClass() == ScnEntity::StaticGetClass() )
	{
		cmdScene_Entity( ScnEntityRef( Resource ), Output, 0 );
	}
	else
	{
		Output.createChildNode( "h2" ).setContents( "Information" );
		BcHtmlNode table = Output.createChildNode( "table" );
		table.createChildNode( "col" ).setAttribute( "wid", "150px" );
		table.createChildNode( "col" ).setAttribute( "wid", "150px" );

		BcHtmlNode row = table.createChildNode( "tr" );
		row.createChildNode( "td" ).setContents( "Resource:" );
		row.createChildNode( "td" ).setContents( *Resource->getName() );

		row = table.createChildNode( "tr" );
		row.createChildNode( "td" ).setContents( "Type:" );
		row.createChildNode( "td" ).setContents( *Resource->getClass()->getName() );

		row = table.createChildNode( "tr" );
		row.createChildNode( "td" ).setContents( "Package:" );
		row.createChildNode( "td" ).setContents( *Resource->getPackageName() );
		BcHtmlNode ul = Output.createChildNode( "ul" );

		Output.createChildNode( "h2" ).setContents( "Fields" );
		// Iterate over all properties and do stuff.
		const ReClass* pClass = Resource->getClass();

		// NOTE: Do not want to hit this. Ever.
		if ( pClass == NULL )
		{
			int a = 0; ++a;
		}
		// Iterate over to grab offsets for classes.
		while ( pClass != NULL )
		{
			BcHtmlNode base = Output.createChildNode( "div" );
			base.createChildNode( "div" ).setContents( pClass->getName().getValue() ).setAttribute( "id", "classHeader" );
			BcHtmlNode div = base.createChildNode( "div" ).setAttribute( "id", "innerData" );
			BcHtmlNode tbl = div.createChildNode( "table" );
			BcHtmlNode header = tbl.createChildNode( "tr" );
			header.createChildNode( "th" ).setContents( "Name" ).setAttribute( "width", "15%" );
			header.createChildNode( "th" ).setContents( "Type" ).setAttribute( "width", "20%" );
			header.createChildNode( "th" ).setContents( "Value" ).setAttribute( "width", "70%*" );


			for ( BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx )
			{
				BcHtmlNode row = tbl.createChildNode( "tr" );
				ReFieldAccessor SrcFieldAccessor( Resource, pClass->getField( Idx ) );
				auto Field = pClass->getField( Idx );

				row.createChildNode( "td" ).setContents( Field->getName().getValue() );
				row.createChildNode( "td" ).setContents( Field->getType()->getName().getValue() );
				BcHtmlNode fValue = row.createChildNode( "td" );

				if ( !SrcFieldAccessor.isContainerType() )
				{
					const ReClass* FieldClass = SrcFieldAccessor.getUpperClass();
					void* data = SrcFieldAccessor.getData();
					std::string str = "UNKNOWN";

					if ( SrcFieldAccessor.getUpperClass()->hasBaseClass( CsResource::StaticGetClass() ) )
					{
						CsResource* resource = static_cast<CsResource*>( SrcFieldAccessor.getData() );
						if ( resource != nullptr )
						{
							fValue.createChildNode( "a" ).setAttribute( "href", "/Resource/" + std::to_string( resource->getUniqueId() ) ).setContents( "Resource" );
							str = "";
						}
						else
						{
							str = "NULL";
						}

					}
					else if ( FieldClass->getTypeSerialiser() != nullptr )
					{
						FieldClass->getTypeSerialiser()->serialiseToString( data, str );
					}
					fValue.setContents( str );
				}
				else
				{
					fValue.setContents( "CONTAINER" );
					auto KeyType = Field->getKeyType();

					if ( KeyType == nullptr )
					{
						// Do something I guess
					}
					else
					{
					}
				}
				// Ignore null pointers, transients, and shallow copies.
				if ( !SrcFieldAccessor.isNullptr() &&
					!SrcFieldAccessor.isTransient() &&
					!SrcFieldAccessor.isShallowCopy() )
				{
					if ( SrcFieldAccessor.isPointerType() )
					{
						//gatherFieldPointer(SrcFieldAccessor);
					}
					else if ( SrcFieldAccessor.isContainerType() && SrcFieldAccessor.isContainerOfPointerValues() )
					{
						//gatherFieldContainer(SrcFieldAccessor);
					}
				}
			}
			Output.createChildNode( "br" );
			pClass = pClass->getSuper();

		}
	}
}

//////////////////////////////////////////////////////////////////////////
// cmdResourceEdit
void DsCoreImpl::cmdResourceEdit( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	BcHtmlNode root = Output.createChildNode( "div" );
	BcHtmlNode table = root.createChildNode( "table" ).setAttribute( "id", "items" );
	BcHtmlNode header = table.createChildNode( "th" );
	header.createChildNode( "td" ).setContents( "Variable" );
	header.createChildNode( "td" ).setContents( "Value" );
}

void DsCoreImpl::cmdWADL( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	BcHtml html;
	html.getRootNode().setTag( "application" );
	html.getRootNode().setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" )
		.setAttribute( "xsi:schemaLocation", "http://wadl.dev.java.net/2009/02 wadl.xsd" )
		.setAttribute( "xmlns:tns", "urn:yahoo:yn" )
		.setAttribute( "xmlns:xsd", "http://www.w3.org/2001/XMLSchema" )
		.setAttribute( "xmlns:yn", "urn:yahoo:yn" )
		.setAttribute( "xmlns:ya", "urn:yahoo:api" )
		.setAttribute( "xmlns", "http://wadl.dev.java.net/2009/02" );
	BcHtmlNode node = html.getRootNode();
	// TODO: Make this adjust depending on the port somehow :S
	BcHtmlNode resources = node.createChildNode( "resources" ).setAttribute( "base", "http://127.0.0.1:1337" );
	// THAT REGEX
	// \(\?\<(?<name>\w*)\>\.\*\)
	// THIS WILL LOOK HORRIBLE
#if !PLATFORM_HTML5
	BcBreakpoint; // neilogd: Not sure if this is a correct conversion to std::regex.
	std::regex re( "\\(\\?\\<(?<name>\\w*)\\>\\.\\*\\)" );

	for ( BcU32 Idx = 0; Idx < PageFunctions_.size(); ++Idx )
	{
		std::smatch results;
		BcHtmlNode resource = resources.createChildNode( "resource" );
		std::string replacement = PageFunctions_[ Idx ].Text_;
		if ( std::regex_search( replacement, results, re ) )
		{
			for ( auto item : results )
			{
				resource.createChildNode( "item" ).setContents( item.str() );
			}
		}

		std::regex_replace( replacement, re, "{$1}" );

		//std::string other = boost::regex_replace();
		resource.setAttribute( "path", PageFunctions_[ Idx ].Text_ );


	}
#else
	BcBreakpoint; // TODO: Switch to std::regex.
#endif

	Output.setContents( html.getHtml() );
}

void DsCoreImpl::cmdJsonSerialiser( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	std::string EntityId = "";

	EntityId = params[ 0 ];
	if ( !BcStrIsNumber( EntityId.c_str() ) )
	{
		Output.createChildNode( "" ).setContents( "Invalid resource Id" );
		Output.createChildNode( "br" );
		return;
	}
	BcU32 id = BcStrAtoi( EntityId.c_str() );
	std::string OutputString = "{\n";
	ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResourceByUniqueId( id ) );

	if ( Resource == nullptr )
	{
		Output.createChildNode( "" ).setContents( "Invalid resource Id" );
		Output.createChildNode( "br" );
		OutputString = " { } ";
		Output.setContents( OutputString );
		return;
	}
	Json::Value readRoot;
	Json::Reader reader;
	bool PostContentAvailable = PostContent.size() > 0;
	reader.parse( PostContent, readRoot );

	Json::Value root;

	Json::Value classes = Json::Value( Json::arrayValue );
	if ( Resource->getClass() == ScnEntity::StaticGetClass() )
	{
		cmdScene_Entity( ScnEntityRef( Resource ), Output, 0 );
	}
	else
	{
		// Iterate over all properties and do stuff.
		const ReClass* pClass = Resource->getClass();

		// NOTE: Do not want to hit this. Ever.
		if ( pClass == NULL )
		{
			int a = 0; ++a;
		}
		// Iterate over to grab offsets for classes.
		while ( pClass != NULL )
		{
			Json::Value theClass;
			theClass[ "className" ] = pClass->getName().getValue();

			Json::Value readNode;
			for ( auto v : readRoot[ "classes" ] )
			{
				if ( v[ "className" ].asString() == pClass->getName().getValue() )
				{
					readNode = v;
				}
			}
			for ( BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx )
			{
				ReFieldAccessor SrcFieldAccessor( Resource, pClass->getField( Idx ) );
				auto Field = pClass->getField( Idx );

				if ( !SrcFieldAccessor.isContainerType() )
				{
					const ReClass* FieldClass = SrcFieldAccessor.getUpperClass();
					void* data = SrcFieldAccessor.getData();
					std::string str = "UNKNOWN";
					std::string fieldName = Field->getName().getValue();

					if ( SrcFieldAccessor.getUpperClass()->hasBaseClass( CsResource::StaticGetClass() ) )
					{
						CsResource* resource = static_cast<CsResource*>( SrcFieldAccessor.getData() );
						if ( resource != nullptr )
						{
							str = "";
						}

					}
					else if ( FieldClass->getTypeSerialiser() != nullptr )
					{
						if ( PostContentAvailable && ( Field->getFlags() & ReFieldFlags::bcRFF_DEBUG_EDIT ) )
						{
							std::string newValue = readNode[ "data" ][ fieldName ].asString();
							FieldClass->getTypeSerialiser()->serialiseFromString( data, newValue );
						}
						FieldClass->getTypeSerialiser()->serialiseToString( data, str );
					}
					if ( Field->getFlags() & ReFieldFlags::bcRFF_DEBUG_EDIT )
					{
						theClass[ "data" ][ fieldName ] = str;
					}
				}
				else
				{
					auto KeyType = Field->getKeyType();

					if ( KeyType == nullptr )
					{
						// Do something I guess
					}
					else
					{
					}
				}
				// Ignore null pointers, transients, and shallow copies.
				if ( !SrcFieldAccessor.isNullptr() &&
					!SrcFieldAccessor.isTransient() &&
					!SrcFieldAccessor.isShallowCopy() )
				{
					if ( SrcFieldAccessor.isPointerType() )
					{
						//gatherFieldPointer(SrcFieldAccessor);
					}
					else if ( SrcFieldAccessor.isContainerType() && SrcFieldAccessor.isContainerOfPointerValues() )
					{
						//gatherFieldContainer(SrcFieldAccessor);
					}
				}
			}
			pClass = pClass->getSuper();
			classes.append( theClass );
		}
	}
	root[ "classes" ] = ( classes );
	Output.setContents( root.toStyledString() );
}

void DsCoreImpl::cmdViewFunctions( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{

}


void DsCoreImpl::cmdJson( DsParameters params, BcHtmlNode& Output, std::string PostContent )
{
	std::string EntityId = "";

	EntityId = params[ 0 ];
	if ( !BcStrIsNumber( EntityId.c_str() ) )
	{
		Output.createChildNode( "" ).setContents( "Invalid resource Id" );
		Output.createChildNode( "br" );
		return;
	}
	BcU32 id = BcStrAtoi( EntityId.c_str() );

	ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResourceByUniqueId( id ) );

	if ( Resource == nullptr )
	{
		Output.createChildNode( "" ).setContents( "Invalid resource Id" );
		Output.createChildNode( "br" );
		return;
	}

	CsSerialiserPackageObjectCodec ObjectCodec( nullptr, ( BcU32 ) bcRFF_ALL, ( BcU32 ) bcRFF_TRANSIENT, ( BcU32 ) bcRFF_ALL );
	SeJsonWriter writer( &ObjectCodec );
	std::string output = writer.serialiseToString<CsResource>( Resource, Resource->getClass() );

	Output.setContents( output );
}
