#include "Editor/Editor.h"

#include "Editor/EdPanelComponentEditor.h"
#include "Editor/EdPanelSceneHierarchy.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include <vector>

namespace Editor
{	
	EdSceneContext SceneContext_;


	struct EditableHandle
	{
		BcU32 ID_ = 0;
		const char* Name_ = nullptr;
		MaVec3d PointA_;
		MaVec3d PointB_;
		bool IsLine_ = false;
		BcF32 Size_ = 32.0f;

		BcU32 CombinedID_ = 0;
	};
	std::vector< EditableHandle > EditableHandles_;

	HandleResult SelectedResult_;

	// View callback.
	class ViewCallback : public ScnViewCallback
	{
	public:
		ViewCallback()
		{
		}

		void onViewDrawPreRender( ScnRenderContext& RenderContext ) override
		{
		}

		void onViewDrawPostRender( ScnRenderContext& RenderContext ) override
		{
			// Intercept transparent passes to render what we need.
			if( BcContainsAllFlags( RenderContext.View_->getPasses(), RsRenderSortPassFlags::TRANSPARENT ) )
			{				
				// TODO: Multiple view support.
				ImGui::Begin( "#editor", nullptr, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus );
				auto DrawList = ImGui::GetWindowDrawList();
				ImGui::End();

				// Sort all lines under other handles.
				std::sort( EditableHandles_.begin(), EditableHandles_.end(), 
					[]( const EditableHandle& A, const EditableHandle& B )
					{
						return A.IsLine_ > B.IsLine_;
					} );

				MaVec2d MousePos( ImGui::GetIO().MousePos );

				for( const auto& Handle : EditableHandles_ )
				{
					const BcF32 HandleSize = std::max( 4.0f, Handle.Size_ );
					const auto ScreenA = ( RenderContext.View_->getScreenPosition( Handle.PointA_ ) +
						MaVec2d( ImGui::GetIO().DisplaySize ) * 0.5f );
					const auto ScreenB = ( RenderContext.View_->getScreenPosition( Handle.PointB_ ) +
						MaVec2d( ImGui::GetIO().DisplaySize ) * 0.5f );

					if( Handle.IsLine_ == false )
					{
						bool MouseOver = false;
						const auto Distance = ( ScreenA - MousePos ).magnitude();
						if( Distance < HandleSize )
						{
							MouseOver = true;
						}

						DrawList->AddCircleFilled( ScreenA, HandleSize, MouseOver ? RsColour::GREEN.asRGBA() : RsColour::WHITE.asRGBA() );
						DrawList->AddCircle( ScreenA, HandleSize, RsColour::BLACK.asRGBA() );

						if( !ImGui::IsPosHoveringAnyWindow( MousePos ) )
						{
							for( int ButtonIdx = 0; ButtonIdx < 3; ++ButtonIdx )
							{
								if( ImGui::GetIO().MouseClicked[ ButtonIdx ] && MouseOver )
								{
									SelectedResult_.WasClicked_ = true;
									SelectedResult_.SelectedID_ = Handle.CombinedID_;
									SelectedResult_.ButtonClicked_ = ButtonIdx;
									SelectedResult_.ScreenPosition_ = MousePos;
									SelectedResult_.WorldPosition_ = Handle.PointA_;
								}
							}
						}
					}
					else
					{
						bool MouseOver = false;
						MaVec2d MousePos( ImGui::GetIO().MousePos );
						MaVec2d AP = MousePos - ScreenA;
						MaVec2d AB = ScreenB - ScreenA;
						BcF32 AB2 = AB.dot( AB );
						BcF32 APAB = AB.dot( AP );
						BcF32 T = BcClamp( APAB / AB2, 0.0f, 1.0f );
						MaVec2d PointOnLine = ( ScreenA + AB * T );
						BcF32 PositionToLine = ( MousePos - PointOnLine ).magnitudeSquared();
						if( PositionToLine < HandleSize )
						{
							MouseOver = true;
						}

						DrawList->AddLine( ScreenA, ScreenB, RsColour::BLACK.asRGBA(), HandleSize );
						DrawList->AddLine( ScreenA, ScreenB, MouseOver ? RsColour::GREEN.asRGBA() : RsColour::WHITE.asRGBA(), HandleSize - 2.0f );

						if( !ImGui::IsPosHoveringAnyWindow( MousePos ) )
						{
							for( int ButtonIdx = 0; ButtonIdx < 3; ++ButtonIdx )
							{
								if( ImGui::GetIO().MouseClicked[ ButtonIdx ] && MouseOver )
								{
									SelectedResult_.WasClicked_ = true;
									SelectedResult_.SelectedID_ = Handle.CombinedID_;
									SelectedResult_.ButtonClicked_ = ButtonIdx;
									SelectedResult_.ScreenPosition_ = MousePos;
									SelectedResult_.WorldPosition_.lerp( Handle.PointA_, Handle.PointB_, BcClamp( T, 0.0f, 1.0f ) );
								}
							}
						}
					}
				}
			}
		}
	};

	ViewCallback ViewCallback_;

	void Init()
	{
		if( ScnCore::pImpl() )
		{
			// Register scene context for callbacks.
			ScnCore::pImpl()->addCallback( &SceneContext_ );

			// Register panels.
			EdPanelComponentEditor::Register( SceneContext_ );
			EdPanelSceneHierarchy::Register( SceneContext_ );

			// Register for view callback.
			ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,
				[]( EvtID, const EvtBaseEvent& )
				{
					ScnViewProcessor::pImpl()->registerViewCallback( &ViewCallback_ );
					return evtRET_REMOVE;
				} );

			// Register for next frame callback.
			ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,
				[]( EvtID, const EvtBaseEvent& )
				{
					EditableHandles_.clear();
					return evtRET_PASS;
				} );
			
		}
	}

	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d Position, BcF32 Size )
	{
		EditableHandle EditableHandle;
		EditableHandle.ID_ = ID;
		EditableHandle.Name_ = Name;
		EditableHandle.PointA_ = Position;
		EditableHandle.PointB_ = Position;
		EditableHandle.Size_ = Size;
		EditableHandle.IsLine_ = false;
		EditableHandle.CombinedID_ = BcHash::GenerateCRC32( EditableHandle.CombinedID_, &ID, sizeof( ID ) );
		EditableHandle.CombinedID_ = BcHash::GenerateCRC32( EditableHandle.CombinedID_, Name, BcStrLength( Name ) );

		EditableHandles_.push_back( EditableHandle );

		if( EditableHandle.CombinedID_ == SelectedResult_.SelectedID_ )
		{
			return SelectedResult_;
		}

		return HandleResult();
	}

	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d PointA, const MaVec3d PointB, BcF32 Size )
	{
		EditableHandle EditableHandle;
		EditableHandle.ID_ = ID;
		EditableHandle.Name_ = Name;
		EditableHandle.PointA_ = PointA;
		EditableHandle.PointB_ = PointB;
		EditableHandle.Size_ = Size;
		EditableHandle.IsLine_ = true;
		EditableHandle.CombinedID_ = BcHash::GenerateCRC32( EditableHandle.CombinedID_, &ID, sizeof( ID ) );
		EditableHandle.CombinedID_ = BcHash::GenerateCRC32( EditableHandle.CombinedID_, Name, BcStrLength( Name ) );

		EditableHandles_.push_back( EditableHandle );

		if( EditableHandle.CombinedID_ == SelectedResult_.SelectedID_ )
		{
			return SelectedResult_;
		}

		return HandleResult();
	}

	void DeselectHandle()
	{
		SelectedResult_.SelectedID_ = 0;
		SelectedResult_.WasClicked_ = false;
	}
}