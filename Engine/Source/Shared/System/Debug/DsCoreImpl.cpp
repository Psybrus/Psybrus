#include "System/Debug/DsCoreImpl.h"
#include "System/Debug/DsCoreLogging.h"
#include "System/Debug/DsImGui.h"
#include "System/Debug/DsTemplate.h"
#include "System/Debug/DsProfilerChromeTracing.h"
#include "System/Debug/DsProfilerRemotery.h"
#include "System/Debug/DsUtils.h"

#include "Base/BcBuildInfo.h"
#include "Base/BcFile.h"
#include "Base/BcHtml.h"
#include "Base/BcProfiler.h"

#include "Serialisation/SeJsonWriter.h"

#include "System/SysKernel.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnTexture.h"

#if !PLATFORM_WINPHONE && !PLATFORM_HTML5
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#endif

#include "Psybrus.h"

#include "Import/Img/Img.h"

#if COMPILER_MSVC
#pragma warning( push )
#pragma warning( disable : 4244 ) // '=': conversion from 'uint32_t' to 'uint8_t', possible loss of data
#pragma warning( disable : 4334 ) // '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
#endif

#include "Import/Img/gif.h"


#if COMPILER_MSVC
#pragma warning( pop )
#endif

//////////////////////////////////////////////////////////////////////////
// Screenshot utility.
namespace ScreenshotUtil
{
	size_t ScreenshotJobQueue( 0 );
	BcBool ScreenCapturing( BcFalse );
	GifWriter Writer;
	size_t FrameCounter( 0 );
	std::atomic< size_t > TotalFramesRemaining( 0 );

	void Init()
	{
		// Create job queue for screenshot encoding/saving.
		ScreenshotJobQueue = SysKernel::pImpl()->createJobQueue( 1, 1 );
	}

	void TakeScreenshot()
	{
		if( ScreenCapturing )
		{
			return;
		}

		RsCore::pImpl()->getContext( nullptr )->takeScreenshot(
			[]( RsScreenshot Screenshot )->BcBool
			{
				// Convert to image.
				const BcU32 W = Screenshot.Width_;
				const BcU32 H = Screenshot.Height_;
				const BcU32* ImageData = reinterpret_cast< const BcU32* >( Screenshot.Data_ );
				ImgImage* Image( new ImgImage() );
				Image->create( W, H, nullptr );
				Image->setPixels( reinterpret_cast< const ImgColour* >( ImageData ) );

				SysKernel::pImpl()->pushFunctionJob( ScreenshotJobQueue, 
					[ Image ]()->void
					{
						PSY_LOG( "Processing screenshot..." );

						// Solid alpha.
						for( BcU32 Y = 0; Y < Image->height(); ++Y )
						{
							for( BcU32 X = 0; X < Image->width(); ++X )
							{
								ImgColour Pixel = Image->getPixel( X, Y );
								Pixel.A_ = 0xff;
								Image->setPixel( X, Y, Pixel );
							}
						}

						// Save out image.
						auto Time = std::time( nullptr );
						auto LocalTime = *std::localtime( &Time );
						BcChar FileName[ 1024 ] = { 0 };
#if PLATFORM_ANDROID
						strftime( FileName, sizeof( FileName ) - 1, "/sdcard/Pictures/screenshot_%Y-%m-%d-%H-%M-%S.png", &LocalTime );
#else
						strftime( FileName, sizeof( FileName ) - 1, "screenshot_%Y-%m-%d-%H-%M-%S.png", &LocalTime );
#endif
						Img::save( FileName, Image );
						PSY_LOG( "Saved screenshot to %s", FileName );
						delete Image;
					} );

				return BcFalse;
			} );
	}

	void BeginCapture()
	{
		// Early out.
		if( ScreenshotUtil::ScreenCapturing == BcTrue || ScreenshotUtil::TotalFramesRemaining > 0 )
		{
			return;
		}

		// Begin the gif capture.
		auto Time = std::time( nullptr );
		auto LocalTime = *std::localtime( &Time );
		BcChar FileName[ 1024 ] = { 0 };
#if PLATFORM_ANDROID
		strftime( FileName, sizeof( FileName ) - 1, "/sdcard/Pictures/screencapture_%Y-%m-%d-%H-%M-%S.gif", &LocalTime );
#else
		strftime( FileName, sizeof( FileName ) - 1, "screencapture_%Y-%m-%d-%H-%M-%S.gif", &LocalTime );
#endif
		const BcU32 W = OsCore::pImpl()->getClient( 0 )->getWidth();
		const BcU32 H = OsCore::pImpl()->getClient( 0 )->getHeight();
		GifBegin( &Writer, FileName, W / 2, H / 2, 1, 8, false );

		// Mark capturing.
		ScreenCapturing = BcTrue;
		FrameCounter = 0;

		// 
		RsCore::pImpl()->getContext( nullptr )->takeScreenshot( 
			[]( RsScreenshot Screenshot )->BcBool
			{
				const BcBool IsScreenCapturing = ScreenCapturing;
				if( ( FrameCounter % 6 ) == 0 || IsScreenCapturing == BcFalse )
				{
					const BcU32 W = Screenshot.Width_;
					const BcU32 H = Screenshot.Height_;
					ImgImage* Image = new ImgImage();
					Image->create( W, H, nullptr );
					Image->setPixels( reinterpret_cast< ImgColour* >( Screenshot.Data_ ) );

					++TotalFramesRemaining;
					SysKernel::pImpl()->pushFunctionJob( ScreenshotJobQueue, 
						[ W, H, IsScreenCapturing, Image ]()->void
						{
							auto HalfImage = Image->resize( W / 2, H / 2, 1.0f );
							GifWriteFrame(
								&Writer,
								reinterpret_cast< const uint8_t* >( HalfImage->getImageData() ), 
								W / 2, H / 2, 0, 8, false );
							if( IsScreenCapturing == BcFalse )
							{
								GifEnd( &Writer );
							}

							delete Image;
							TotalFramesRemaining--;
						} );
				}
				++FrameCounter;
				return IsScreenCapturing;
			} );
	}

	void EndCapture()
	{
		ScreenCapturing = BcFalse;
	}
}

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
	PanelFunctions_(),
	PageFunctions_(),
	ButtonFunctions_(),
	NextHandle_( 0 )
{
	// Start profiler.
#if PSY_USE_PROFILER
	if( GCommandLine_.hasArg( 'p', "profile" ) )
	{
		new DsProfilerRemotery();
	}

#if PLATFORM_ANDROID
	//new DsProfilerChromeTracing();
#endif
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

	ShortcutLookup_[ 192 ] = "`";
	ShortcutLookup_[ '~' ] = "~";
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCoreImpl::~DsCoreImpl()
{
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void DsCoreImpl::open()
{
	IPAddresses_ = getIPAddresses();
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
	for ( int i = 0; i < IPAddresses_.size(); ++i )
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

#if !PSY_PRODUCTION
	if( ScnCore::pImpl() )
	{
		// Setup init/deinit hooks.
		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, this,
			[]( EvtID, const EvtBaseEvent& )
		{
			if( OsCore::pImpl()->getClient( 0 ) )
			{
				ImGui::Psybrus::Init();
				Debug::Init( 1024 * 64 );

			}
			return evtRET_REMOVE;
		} );

		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_CLOSE, this,
			[]( EvtID, const EvtBaseEvent& )
		{
			if( OsCore::pImpl()->getClient( 0 ) )
			{
				Debug::Shutdown();
				ImGui::Psybrus::Shutdown();
			}
			return evtRET_REMOVE;
		} );

		// Subscribe to F2 & F3 for screenshot
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, this,
			[ this ]( EvtID ID, const EvtBaseEvent& Event )
			{
				const auto& KeyEvent = Event.get< OsEventInputKeyboard >();
				if( KeyEvent.KeyCode_ == OsEventInputKeyboard::KEYCODE_F3 )
				{
					if( ScreenshotUtil::ScreenCapturing == BcFalse )
					{
						ScreenshotUtil::BeginCapture();
					}
					else
					{
						ScreenshotUtil::EndCapture();
					}
				}
				else if( KeyEvent.KeyCode_ == OsEventInputKeyboard::KEYCODE_F2 )
				{
					ScreenshotUtil::TakeScreenshot();
				}
				return evtRET_PASS;
			} );

		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, this,
			[ this ]( EvtID, const EvtBaseEvent& )
		{
			PSY_PROFILER_SECTION( "ImGui" );
			if( OsCore::pImpl()->getClient( 0 ) )
			{
				if( ImGui::Psybrus::NewFrame() )
				{
					// Setup window.
					MaVec2d ClientSize( (BcF32)OsCore::pImpl()->getClient( 0 )->getWidth(), (BcF32)OsCore::pImpl()->getClient( 0 )->getHeight() );
					ImGui::Begin( "Engine Internal", NULL, ClientSize, 0, 
						ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
						ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing );
					auto DrawList = ImGui::GetWindowDrawList();
					ImGui::End();

					// Draw stats.
					if( DrawStats_ )
					{
						auto Font = ImGui::GetWindowFont();
						auto TextHeightIncr = MaVec2d( 0.0f, Font->FontSize + 2.0f ) * ImGui::GetIO().FontGlobalScale;
						auto ShadowOff = MaVec2d( 1.0f, 1.0f );

						auto NumLines = 4.0f + 1.0f * IPAddresses_.size();
						auto TextPos = MaVec2d( 16.0f, ClientSize.y() - ( 16.0f + TextHeightIncr.y() * NumLines ) );

						ImGui::AddShadowedText( DrawList, TextPos, 0xffffffff, "Build: %s-%s-%s", 
								BUILD_ACTION,
								BUILD_TOOLCHAIN,
								BUILD_CONFIG );
						TextPos += TextHeightIncr;

						ImGui::AddShadowedText( DrawList, TextPos, 0xffffffff, "Version: %s", 
								BUILD_VERSION );
						TextPos += TextHeightIncr;

						ImGui::AddShadowedText( DrawList, TextPos, 0xffffffff, "Date/Time: %s %s", 
								BUILD_DATE, BUILD_TIME );
						TextPos += TextHeightIncr;
						
						for( const auto& IPAddress : IPAddresses_ )
						{
							ImGui::AddShadowedText( DrawList, TextPos, 0xffffffff, "IP: %s", IPAddress.c_str() );
							TextPos += TextHeightIncr;
						}

						auto ScreenshotProcessing = ScreenshotUtil::TotalFramesRemaining.load();
						if( ScreenshotProcessing > 0 )
						{
							ImGui::AddShadowedText( DrawList, TextPos, 0xff0000ff, "Capture frames remaining to process: %u", ScreenshotProcessing );
							TextPos += TextHeightIncr;
						}

						// TODO: Per view perhaps?
						float Size = 32 * ImGui::GetIO().FontGlobalScale;
						ImGui::SetNextWindowPos( MaVec2d( ClientSize.x() - Size * 2.0f, Size ) );
						if( ImGui::Begin( "Engine Internal 2", NULL, MaVec2d( Size,  Size ), 0.0f,
							ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
							ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing ) )
						{
							if( ImGui::Button( "..." ) )
							{
								ImGui::OpenPopup( "View Popup" );
							}
							if( ImGui::BeginPopup( "View Popup" ) )
							{
								if( ImGui::BeginMenu( "Debug Rendering" ) )
								{
									BcU32 CategoryMask = Debug::GetDrawCategoryMask();

									std::array< const char*, 32 > CategoryNames;
									std::array< BcU32, 32 > CategoryMasks;
									size_t NumCategories = Debug::GetDrawCategories( 
										CategoryNames.data(), CategoryMasks.data(), CategoryNames.size() );
									for( auto Idx = 0; Idx < NumCategories; ++Idx )
									{
										if( ImGui::MenuItem( CategoryNames[ Idx ], nullptr, 
											!!( CategoryMasks[ Idx ] & CategoryMask ), true) )
										{
											CategoryMask ^= CategoryMasks[ Idx ];
										}
									}
									Debug::SetDrawCategoryMask( CategoryMask );
									ImGui::EndMenu();
								}

								if( ImGui::BeginMenu( "Capture" ) )
								{
									if( !ScreenshotUtil::ScreenCapturing )
									{
										if( ImGui::MenuItem( "Screenshot", "F2" ) )
										{
											ScreenshotUtil::TakeScreenshot();
										}
									}

									if( ScreenshotUtil::ScreenCapturing == BcFalse && ScreenshotUtil::TotalFramesRemaining == 0 )
									{
										if( ImGui::MenuItem( "Begin Capture", "F3" ) )
										{
											ScreenshotUtil::BeginCapture();
										}
									}
									else
									{
										if( ImGui::MenuItem( "End Capture", "F3" ) )
										{
											ScreenshotUtil::EndCapture();
										}
									}

									ImGui::EndMenu();
								}

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
										if( ImGui::MenuItem( Panel.Name_.c_str(), Panel.Shortcut_.c_str(), Panel.IsVisible_ ) )
										{
											Panel.IsVisible_ = !Panel.IsVisible_; 
										}
									}
								}
								if( MenuOpen && !LastCategory.empty() )
								{
									ImGui::EndMenu();
								}

								ImGui::EndPopup();
							}
						}
						ImGui::End();
					}

					// Draw views.
					if( DrawViews_ )
					{
						for( const auto& ViewInfo : ViewInfos_ )
						{
							MaMat4d View = ViewInfo.View_;
							View.row3( MaVec4d( 0.0f, 0.0f, 0.0f, 1.0f ) );
							MaMat4d ClipTransform = View * ViewInfo.Proj_;

							DrawList->PushClipRect( 
								MaVec2d( (BcF32)ViewInfo.Viewport_.x(), (BcF32)ViewInfo.Viewport_.y() ),
								MaVec2d( (BcF32)ViewInfo.Viewport_.x(), (BcF32)ViewInfo.Viewport_.y() ) +
									MaVec2d( (BcF32)ViewInfo.Viewport_.width(), (BcF32)ViewInfo.Viewport_.height() ) );

							auto getScreenPos = [ & ]( MaVec4d WorldPos )
							{
								MaVec4d ScreenSpace = WorldPos * ClipTransform;
								MaVec2d ScreenPosition = MaVec2d( ScreenSpace.x() / ScreenSpace.w(), -ScreenSpace.y() / ScreenSpace.w() );

								BcF32 Aspect = (BcF32)ViewInfo.Viewport_.width() / (BcF32)ViewInfo.Viewport_.height();

								return MaVec2d( ( ScreenPosition.x() ), ( ScreenPosition.y() / Aspect ) );
							};

							MaMat4d InvView = View;
							InvView.inverse();
							MaVec4d Offset = MaVec4d( 0.0f, 0.0f, 4.0f, 1.0f ) * InvView;

							auto SC = getScreenPos( MaVec4d( 0.0f, 0.0f, 0.0f, 0.0f ) + Offset );
							auto SX = getScreenPos( MaVec4d( 1.0f, 0.0f, 0.0f, 0.0f ) + Offset );
							auto SY = getScreenPos( MaVec4d( 0.0f, 1.0f, 0.0f, 0.0f ) + Offset );
							auto SZ = getScreenPos( MaVec4d( 0.0f, 0.0f, 1.0f, 0.0f ) + Offset );

							auto S = 64.0f;
							MaVec2d SO = MaVec2d( ViewInfo.Viewport_.width() - S, ViewInfo.Viewport_.height() - S );
							SO += MaVec2d( (BcF32)ViewInfo.Viewport_.x(), (BcF32)ViewInfo.Viewport_.y() );
							SC = ( SC * S * 3.0f ) + SO;
							SX = ( SX * S * 3.0f ) + SO;
							SY = ( SY * S * 3.0f ) + SO;
							SZ = ( SZ * S * 3.0f ) + SO;

							DrawList->AddLine( SC, SX, 0xff0000ff, 4.0f );
							DrawList->AddLine( SC, SY, 0xff00ff00, 4.0f );
							DrawList->AddLine( SC, SZ, 0xffff0000, 4.0f );


							Debug::DrawViewOverlay( DrawList, ViewInfo.View_, ViewInfo.Proj_, ViewInfo.Viewport_, nullptr );

							DrawList->PopClipRect();
						}
					}

					for ( auto& Panel : PanelFunctions_ )
					{
						if( Panel.IsVisible_ )
						{
							if ( ImGui::Begin( Panel.Name_.c_str() ) )
							{
								Panel.Function_( Panel.Handle_ );
							}
							ImGui::End();
						}
					}
				}
			}
			ViewInfos_.clear();

			Debug::NextFrame();
			return evtRET_PASS;
		} );
	}

	// Scale for DPI.
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )
	{
		auto Client = OsCore::pImpl()->getClient( 0 );
		if( Client )
		{
			auto Scale = BcMax( 1.0f, BcFloor( BcF32( Client->getDPI() ) / 96.0f ) ) ;
			auto& Style = ImGui::GetStyle();
			Style.GrabMinSize *= Scale;
			ImGui::GetIO().FontGlobalScale = BcMax( 1.0f, Scale * 0.75f );
		}

		return evtRET_PASS;
	} );

	// Setup toggle of debug panels.
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, this,
		[ this ]( EvtID, const EvtBaseEvent& InEvent )
	{
		const auto& Event = InEvent.get< OsEventInputKeyboard >();

		const bool CtrlModifier = BcContainsAllFlags( Event.Modifiers_, OsEventInputKeyboard::MODIFIER_CTRL );
		const bool AltModifier = BcContainsAllFlags( Event.Modifiers_, OsEventInputKeyboard::MODIFIER_ALT );
		const bool ShiftModifier = BcContainsAllFlags( Event.Modifiers_, OsEventInputKeyboard::MODIFIER_SHIFT );

		// Check if a shortcut combo has been pressed.
		if( ShortcutLookup_.find( Event.KeyCode_ ) != ShortcutLookup_.end() )
		{
			// Check panels.
			for ( auto& Panel : PanelFunctions_ )
			{
				if( Panel.Shortcut_.size() > 0 )
				{
					bool CtrlRequired = Panel.Shortcut_.find( "Ctrl" ) != std::string::npos;
					bool AltRequired = Panel.Shortcut_.find( "Alt" ) != std::string::npos;
					bool ShiftRequired = Panel.Shortcut_.find( "Shift" ) != std::string::npos;
				
					if( CtrlRequired == !!CtrlModifier &&
						AltRequired == !!AltModifier &&
						ShiftRequired == !!ShiftModifier )
					{
						std::string Shortcut = ShortcutLookup_[ Event.KeyCode_ ];
						if( CtrlRequired || AltRequired || ShiftRequired )
						{
							Shortcut = "+" + Shortcut;
						}

						if( Panel.Shortcut_.find( Shortcut ) == Panel.Shortcut_.length() - Shortcut.length() )
						{
							Panel.IsVisible_ = !Panel.IsVisible_;
							return evtRET_BLOCK;
						}
					}
				}
			}
		}

		return evtRET_PASS;
	} );

#endif // !PSY_PRODUCTION
	// Start screenshot util.
	ScreenshotUtil::Init();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void DsCoreImpl::update()
{
#if USE_WEBBY
	for ( unsigned int Idx = 0; Idx < Servers_.size(); ++Idx )
	{
		PSY_PROFILER_SECTION( "WebbyServerUpdate" );

		WebbyServerUpdate( Servers_[ Idx ] );
	}
#endif // USE_WEBBY
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void DsCoreImpl::close()
{
	if( OsCore::pImpl() )
	{
		OsCore::pImpl()->unsubscribeAll( this );
	}

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

#endif // USE_WEBBY

std::vector< std::string > DsCoreImpl::getIPAddresses()
{
	std::vector< std::string > result;
#if !PLATFORM_WINPHONE && !PLATFORM_HTML5
	RakNet::RakPeerInterface* peer = NULL;
	peer = RakNet::RakPeerInterface::GetInstance();

	short port = 1337;

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

//////////////////////////////////////////////////////////////////////////
// registerPanel
BcU32 DsCoreImpl::registerPanel( const char* Category, const char* Name, const char* Shortcut, std::function< void( BcU32 )> Func )
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;
	PanelFunctions_.emplace_back( Category, Name, Shortcut, std::move( Func ), Handle );
	std::sort( PanelFunctions_.begin(), PanelFunctions_.end(), 
		[]( const DsPanelDefinition& A, const DsPanelDefinition& B ) 
		{
			return A.Category_ < B.Category_;
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
// addViewOverlay
void DsCoreImpl::addViewOverlay( const MaMat4d& View, const MaMat4d& Proj, const RsViewport& Viewport )
{
	ViewInfo ViewInfo = { View, Proj, Viewport };
	ViewInfos_.push_back( ViewInfo );
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
				BcHtmlNode row2 = tbl.createChildNode( "tr" );
				ReFieldAccessor SrcFieldAccessor( Resource, pClass->getField( Idx ) );
				auto Field = pClass->getField( Idx );

				row2.createChildNode( "td" ).setContents( Field->getName().getValue() );
				row2.createChildNode( "td" ).setContents( Field->getType()->getName().getValue() );
				BcHtmlNode fValue = row2.createChildNode( "td" );

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
#if 0
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
#endif
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
