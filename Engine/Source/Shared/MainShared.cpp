/**************************************************************************
*
* File:		MainShared.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "MainShared.h"

BcU32 GResolutionWidth = 1280;
BcU32 GResolutionHeight = 720;

#include "Events/EvtBinding.h"
#include "Events/EvtEvent.h"
#include "Events/EvtPublisher.h"
#include "Events/EvtProxyBuffered.h"

#include "Math/MaMat3d.h"
#include "Math/MaMat4d.h"

#include "System/SysKernel.h"

#include "System/Content/CsCore.h"
#include "System/Debug/DsCore.h"
#include "System/File/FsCore.h"
#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"
#include "System/Sound/SsCore.h"
#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/SysKernel.h"

#include "Editor/Editor.h"

#include "Base/BcFile.h"
#include "Base/BcProfiler.h"

#define SEARCH_FOR_CORRECT_PATH ( PLATFORM_WINDOWS | PLATFORM_LINUX | PLATFORM_OSX )

#include <chrono>

//////////////////////////////////////////////////////////////////////////
// Unit test setup.
#if !PSY_PRODUCTION
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <reporters/catch_reporter_teamcity.hpp>

int MainUnitTests()
{
	BcAssertScopedHandler AssertHandler(
		[]( const BcChar* Message, const BcChar* File, int Line )
		{
			BcPrintBacktrace( BcBacktrace() );
			std::array< char, 256 > Buffer;
			BcSPrintf( Buffer.data(), Buffer.size(), "Caught assertion: \"%s\" in %s on line %u.\n", Message, File, Line );
			FAIL( Buffer.data() );
			return BcFalse;
		} );

	auto RetVal = Catch::Session().run( GCommandLine_.getArgc(), GCommandLine_.getArgv() );

#if PLATFORM_WINDOWS
	if( RetVal != 0 && ::IsDebuggerPresent() )
	{
		BcBreakpoint;
	}
#endif

	return RetVal;
}

#endif // !PSY_PRODUCTION
 
//////////////////////////////////////////////////////////////////////////
// onCsCoreOpened
#if !PLATFORM_ANDROID && !PLATFORM_HTML5
eEvtReturn onCsCoreOpened( EvtID ID, const EvtBaseEvent& Event )
{
	// Register reflection.
	extern void AutoGenRegisterReflection();
	AutoGenRegisterReflection();

	return evtRET_REMOVE;
}
#endif

//////////////////////////////////////////////////////////////////////////
// onDsCoreOpened
eEvtReturn onDsCoreOpened( EvtID ID, const EvtBaseEvent& Event )
{
	struct EngineLog :
		public BcLogListener
	{
		EngineLog()
		{
		}

		EngineLog( const EngineLog& )
		{
		}

		void operator()( BcU32 Handle )
		{
			ImGui::SetNextWindowSize( ImVec2( 500, 400 ), ImGuiSetCond_FirstUseEver );
			if( ImGui::Button( "Clear" ) )
			{
				Buf_.clear(); 
				LineOffsets_.clear(); 
			}
			ImGui::SameLine();
			bool Copy = ImGui::Button( "Copy" );
			ImGui::SameLine();
			Filter_.Draw( "Filter", -100.0f );
			ImGui::Separator();
			ImGui::BeginChild( "scrolling" );
			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 1 ) );
			if( Copy )
			{
				ImGui::LogToClipboard();
			}

			if( Filter_.IsActive() )
			{
				const char* BufBegin = Buf_.begin();
				const char* Line = BufBegin;
				for( int LineNo = 0; Line != NULL; LineNo++ )
				{
					const char* LineEnd = (LineNo < LineOffsets_.Size ) ? BufBegin + LineOffsets_[ LineNo ] : NULL;
					if( Filter_.PassFilter( Line, LineEnd ) )
					{
						ImGui::TextUnformatted( Line, LineEnd );
					}
					Line = LineEnd && LineEnd[1] ? LineEnd + 1 : NULL;
				}
			}
			else
			{
				ImGui::TextUnformatted( Buf_.begin() );
			}

			if( ScrollToBottom_ )
			{
				ImGui::SetScrollHere(1.0f);
			}
			ScrollToBottom_ = false;
			ImGui::PopStyleVar();
			ImGui::EndChild();
		}

		void onLog( const BcLogEntry& Entry ) override
		{
			int OldSize = Buf_.size();
			Buf_.append( "%s\n", Entry.Text_.c_str() );

			for( int NewSize = Buf_.size(); OldSize < NewSize; OldSize++ )
			{
				if( Buf_[ OldSize ] == '\n' )
				{
					LineOffsets_.push_back( OldSize );
				}
			}
			ScrollToBottom_ = true;
		}
		
		ImGuiTextBuffer Buf_;
		ImGuiTextFilter Filter_;
		ImVector<int> LineOffsets_;        // Index to lines offset
		bool ScrollToBottom_ = false;
	};

	DsCore::pImpl()->registerPanel(
		"Engine", "ImGui Demo", nullptr, []( BcU32 )->void
		{
			ImGui::ShowTestWindow();
		} );

	DsCore::pImpl()->registerPanel( 
		"Engine", "Log", "Ctrl+Alt+L", EngineLog() );

	DsCore::pImpl()->registerPanel( 
		"Engine", "Stat Overlay", "Ctrl+Alt+S", []( BcU32 )->void
		{
			static BcF64 GameTimeTotal = 0.0f;
			static BcF64 RenderTimeTotal = 0.0f;
			static BcF64 GPUTimeTotal = 0.0f;
			static BcF64 FrameTimeTotal = 0.0f;
			static BcF64 GameTimeAccum = 0.0f;
			static BcF64 RenderTimeAccum = 0.0f;
			static BcF64 GPUTimeAccum = 0.0f;
			static BcF64 FrameTimeAccum = 0.0f;
			static int CaptureAmount = 60;
			static int CaptureAccum = 0;

			const BcU32 NOOF_POINTS = 128;
			static std::array< BcF32, NOOF_POINTS > GameTimeGraphPoints = { 0.0f };
			static std::array< BcF32, NOOF_POINTS > RenderTimeGraphPoints = { 0.0f };
			static std::array< BcF32, NOOF_POINTS > GPUTimeGraphPoints = { 0.0f };
			static std::array< BcF32, NOOF_POINTS > FrameTimeGraphPoints = { 0.0f };
			static int GraphPointIdx = 0;

			static BcF32 GraphScale = 18.0f;

			GameTimeAccum += SysKernel::pImpl()->getGameThreadTime();
			RenderTimeAccum += RsCore::pImpl()->getFrameTime();
			GPUTimeAccum += ScnViewProcessor::pImpl()->getFrameTime();
			FrameTimeAccum += SysKernel::pImpl()->getFrameTime();
			++CaptureAccum;
			if( CaptureAccum >= CaptureAmount )
			{
				GameTimeTotal = GameTimeAccum / BcF32( CaptureAccum );
				RenderTimeTotal = RenderTimeAccum / BcF32( CaptureAccum );
				GPUTimeTotal = GPUTimeAccum / BcF32( CaptureAccum );
				FrameTimeTotal = FrameTimeAccum / BcF32( CaptureAccum );
				GameTimeAccum = 0.0f;
				RenderTimeAccum = 0.0f;
				GPUTimeAccum = 0.0f;
				FrameTimeAccum = 0.0f;
				CaptureAccum = 0;
			}

			GameTimeGraphPoints[ GraphPointIdx ] = (BcF32)SysKernel::pImpl()->getGameThreadTime() * 1000.0f;
			RenderTimeGraphPoints[ GraphPointIdx ] = (BcF32)RsCore::pImpl()->getFrameTime() * 1000.0f;
			GPUTimeGraphPoints[ GraphPointIdx ] = (BcF32)ScnViewProcessor::pImpl()->getFrameTime() * 1000.0f;
			FrameTimeGraphPoints[ GraphPointIdx ] = (BcF32)SysKernel::pImpl()->getFrameTime() * 1000.0f;
			
			GraphScale = std::max( GraphScale, 33.0f );
			BcF32 TargetGraphScale = 33.0f;
			for( BcU32 Idx = 0; Idx < NOOF_POINTS; ++Idx )
			{
				TargetGraphScale = std::max( GraphScale, GameTimeGraphPoints[ Idx ] );
				TargetGraphScale = std::max( GraphScale, RenderTimeGraphPoints[ Idx ] );
				TargetGraphScale = std::max( GraphScale, FrameTimeGraphPoints[ Idx ] );
			}
			if( TargetGraphScale > 33.0f )
			{
				while( GraphScale < TargetGraphScale )
				{
					GraphScale *= 2.0f;
				}
			}
			else
			{
				GraphScale = TargetGraphScale;
			}


			GraphPointIdx = ( GraphPointIdx + 1 ) % GameTimeGraphPoints.size();

			BcF32 Scale = 1.0f;
			auto Client = OsCore::pImpl()->getClient( 0 );
			if( Client )
			{
				Scale = BcMax( 1.0f, BcFloor( BcF32( Client->getDPI() ) / 96.0f ) ) ;
			}

			ImGui::Text( "Worker count: %u", 
				(BcU32)SysKernel::pImpl()->workerCount() );

			if( ImGui::TreeNode( "Game", "Game time: %.2fms (%.2fms avg.)", 
				SysKernel::pImpl()->getGameThreadTime() * 1000.0f, GameTimeTotal * 1000.0f ) )
			{
				ImGui::PlotLines( "", GameTimeGraphPoints.data(), (int)GameTimeGraphPoints.size(), GraphPointIdx, nullptr, 0.0f, GraphScale, MaVec2d( 256.0f, 64.0f ) * Scale );
				ImGui::TreePop();
			}

			if( ImGui::TreeNode( "Render", "Render time: %.2fms (%.2fms avg.)", 
				RsCore::pImpl()->getFrameTime() * 1000.0f, RenderTimeTotal * 1000.0f ) )
			{
				ImGui::PlotLines( "", RenderTimeGraphPoints.data(), (int)RenderTimeGraphPoints.size(), GraphPointIdx, nullptr, 0.0f, GraphScale, MaVec2d( 256.0f, 64.0f ) * Scale );
				ImGui::TreePop();
			}

			if( ImGui::TreeNode( "GPU", "GPU time: %.2fms (%.2fms avg.)", 
				ScnViewProcessor::pImpl()->getFrameTime() * 1000.0f, GPUTimeTotal * 1000.0f ) )
			{
				ImGui::PlotLines( "", GPUTimeGraphPoints.data(), (int)GPUTimeGraphPoints.size(), GraphPointIdx, nullptr, 0.0f, GraphScale, MaVec2d( 256.0f, 64.0f ) * Scale );
				ImGui::TreePop();
			}

			if( ImGui::TreeNode( "Frame", "Frame time: %.2fms (%.2fms avg.)", 
				SysKernel::pImpl()->getFrameTime() * 1000.0f, FrameTimeTotal * 1000.0f ) )
			{
				ImGui::PlotLines( "", FrameTimeGraphPoints.data(), (int)FrameTimeGraphPoints.size(), GraphPointIdx, nullptr, 0.0f, GraphScale, MaVec2d( 256.0f, 64.0f ) * Scale );
				ImGui::TreePop();
			}
		} );

	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// OnQuit
eEvtReturn onQuit( EvtID ID, const EvtBaseEvent& Event )
{
	SysKernel::pImpl()->stop();

	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// MainShared
void MainShared()
{
#ifdef SearchPath
#undef SearchPath
#endif

#if SEARCH_FOR_CORRECT_PATH
	// Change to executable root.
	BcFileSystemChangeDirectory( BcPath( GExePath_ ).getParent().c_str() );

	bool FoundRoot = false;
	bool Success = true;
	while( !FoundRoot && Success )
	{
		// Check for Content or PackedContent.
		if( BcFileSystemExists( "Content" ) || BcFileSystemExists( "PackedContent" ) )
		{
			FoundRoot = true;
		}
#if !PSY_PRODUCTION
		else if( BcFileSystemExists( "Dist" ) )
		{
			FoundRoot = true;
			BcFileSystemChangeDirectory( "Dist" );
		}
#endif
		else
		{
			Success = BcFileSystemChangeDirectory( ".." );
		}
	}

	if( !Success )
	{
		PSY_LOG( "ERROR: Can't find working directory containing PackedContent." );
		exit(1);
	}
#endif

	if( GCommandLine_.hasArg( '\0', "norender" ) )
	{
		GPsySetupParams.Flags_ &= ~psySF_RENDER;
	}

	if( GCommandLine_.hasArg( '\0', "nosound" ) )
	{
		GPsySetupParams.Flags_ &= ~psySF_SOUND;
	}

	// Check if running as a server.
	const bool IsServer = GCommandLine_.hasArg( '\0', "server" );
	if( IsServer )
	{
		GPsySetupParams.Flags_ = psySF_SERVER;		
	}
	
	// Start debug system if not a production build.
#if !defined( PSY_PRODUCTION )
	SysKernel::pImpl()->startSystem( "DsCoreLogging" );
#endif

//#if !defined( PSY_PRODUCTION )
	SysKernel::pImpl()->startSystem( "DsCore" );
//#endif

	// Start workers.
	SysKernel::pImpl()->startWorkers();

	// Setup default system job queues.
	SysKernel::DEFAULT_JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( std::thread::hardware_concurrency(), 0 );

	// Start file system.
	SysKernel::pImpl()->startSystem( "FsCore" );

	// Start OS system.
	SysKernel::pImpl()->startSystem( "OsCore" );

	// Start render system.
	if( GPsySetupParams.Flags_ & psySF_RENDER )
	{
		SysKernel::pImpl()->startSystem( "RsCore" );
	}

	// Start sound system.
	if( GPsySetupParams.Flags_ & psySF_SOUND )
	{
		SysKernel::pImpl()->startSystem( "SsCore" );
	}

	// Start content system, depending on startup flags.
	SysKernel::pImpl()->startSystem( "CsCore" );

	// Start scene system.
	if( GPsySetupParams.Flags_ & psySF_SCENE )
	{
		SysKernel::pImpl()->startSystem( "ScnCore" );
	}

	// Log kernel information.
	PSY_LOG( "============================================================================\n" );
	PSY_LOG( "MainShared:\n" );
	PSY_LOG( " - Command line: %s\n", GCommandLine_.c_str() );
	PSY_LOG( " - Setup Flags: 0x%x\n", GPsySetupParams.Flags_ );
	PSY_LOG( " - Name: %s\n", GPsySetupParams.Name_.c_str() );
	PSY_LOG( " - Tick Rate: 1.0/%.1f\n", 1.0f / GPsySetupParams.TickRate_ );
	PSY_LOG( " - SysKernel::DEFAULT_JOB_QUEUE_ID: 0x%x\n", SysKernel::DEFAULT_JOB_QUEUE_ID );
	PSY_LOG( " - FsCore::JOB_QUEUE_ID: 0x%x\n", FsCore::JOB_QUEUE_ID );
	PSY_LOG( " - RsCore::JOB_QUEUE_ID: 0x%x\n", RsCore::JOB_QUEUE_ID );
	PSY_LOG( " - SsCore::JOB_QUEUE_ID: 0x%x\n", SsCore::JOB_QUEUE_ID );

#if !PLATFORM_ANDROID && !PLATFORM_HTML5
	// Setup callback for post CsCore open for resource registration.
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, onCsCoreOpened );
#endif

	// Setup callback for post DsCore open.
	if( DsCore::pImpl() )
	{
		DsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, onDsCoreOpened );
	}
	
	// Subscribe to quit.
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT, onQuit );

#if PSY_USE_PROFILER
	// Set name in profiler.
	if( BcProfiler::pImpl() )
	{
		BcProfiler::pImpl()->setCurrentThreadName( "Main Thread" );
	}
#endif

#if !PSY_PRODUCTION
	Editor::Init();
#endif
}
