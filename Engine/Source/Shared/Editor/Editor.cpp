#include "Editor/Editor.h"

#include "Editor/PanelComponentEditor.h"
#include "Editor/PanelSceneHierarchy.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Os/OsCore.h"

#include <vector>

namespace Editor
{	
	SceneContext SceneContext_;


	struct EditorHandle
	{
		BcU32 ID_ = 0;
		const char* Name_ = nullptr;
		MaVec3d PointA_;
		MaVec3d PointB_;
		bool IsLine_ = false;
		BcF32 Size_ = 32.0f;

		BcU32 CombinedID_ = 0;
	};
	std::vector< EditorHandle > EditorHandles_;


	struct EditorAction
	{
		BcU32 ID_ = 0;
		const char* Name_ = nullptr;
		ActionCallback Do_;
		ActionCallback Undo_;

		BcU32 CombinedID_ = 0;
	};
	EditorAction ActiveAction_;
	std::deque< EditorAction > UndoStack_;
	std::deque< EditorAction > RedoStack_;

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
				std::sort( EditorHandles_.begin(), EditorHandles_.end(), 
					[]( const EditorHandle& A, const EditorHandle& B )
					{
						return A.IsLine_ > B.IsLine_;
					} );

				MaVec2d MousePos( ImGui::GetIO().MousePos );

				for( const auto& Handle : EditorHandles_ )
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
									Action( Handle.CombinedID_, "Select",
										[ Handle, ButtonIdx, MousePos ]()
										{
											SelectedResult_.WasClicked_ = true;
											SelectedResult_.SelectedID_ = Handle.CombinedID_;
											SelectedResult_.ButtonClicked_ = ButtonIdx;
											SelectedResult_.ScreenPosition_ = MousePos;
											SelectedResult_.WorldPosition_ = Handle.PointA_;
										},
										[ SelectedResult = SelectedResult_ ]()
										{
											SelectedResult_ = SelectedResult;
										} );	
								}
							}
						}
					}
					else
					{
						bool MouseOver = false;
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
									Action( Handle.CombinedID_, "Select",
										[ Handle, ButtonIdx, MousePos, T ]()
										{
											SelectedResult_.WasClicked_ = true;
											SelectedResult_.SelectedID_ = Handle.CombinedID_;
											SelectedResult_.ButtonClicked_ = ButtonIdx;
											SelectedResult_.ScreenPosition_ = MousePos;
											SelectedResult_.WorldPosition_.lerp( Handle.PointA_, Handle.PointB_, BcClamp( T, 0.0f, 1.0f ) );
										},
										[ SelectedResult = SelectedResult_ ]()
										{
											SelectedResult_ = SelectedResult;
										} );	
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
			PanelComponentEditor::Register( SceneContext_ );
			PanelSceneHierarchy::Register( SceneContext_ );

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
					EditorHandles_.clear();
					return evtRET_PASS;
				} );
		}

		// Subscribe to F2 & F3 for screenshot
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN,
			[]( EvtID ID, const EvtBaseEvent& Event )
			{
				const auto& KeyEvent = Event.get< OsEventInputKeyboard >();
				if( KeyEvent.KeyCode_ == 'Z' )
				{
					UndoAction();
				}
				if( KeyEvent.KeyCode_ == 'Y' )
				{
					RedoAction();
				}
				return evtRET_PASS;
			} );
	}

	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d Position, BcF32 Size )
	{
		EditorHandle EditorHandle;
		EditorHandle.ID_ = ID;
		EditorHandle.Name_ = Name;
		EditorHandle.PointA_ = Position;
		EditorHandle.PointB_ = Position;
		EditorHandle.Size_ = Size;
		EditorHandle.IsLine_ = false;
		EditorHandle.CombinedID_ = BcHash::GenerateCRC32( EditorHandle.CombinedID_, &ID, sizeof( ID ) );
		EditorHandle.CombinedID_ = BcHash::GenerateCRC32( EditorHandle.CombinedID_, Name, BcStrLength( Name ) );

		EditorHandles_.push_back( EditorHandle );

		if( EditorHandle.CombinedID_ == SelectedResult_.SelectedID_ )
		{
			return SelectedResult_;
		}

		return HandleResult();
	}

	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d PointA, const MaVec3d PointB, BcF32 Size )
	{
		EditorHandle EditorHandle;
		EditorHandle.ID_ = ID;
		EditorHandle.Name_ = Name;
		EditorHandle.PointA_ = PointA;
		EditorHandle.PointB_ = PointB;
		EditorHandle.Size_ = Size;
		EditorHandle.IsLine_ = true;
		EditorHandle.CombinedID_ = BcHash::GenerateCRC32( EditorHandle.CombinedID_, &ID, sizeof( ID ) );
		EditorHandle.CombinedID_ = BcHash::GenerateCRC32( EditorHandle.CombinedID_, Name, BcStrLength( Name ) );

		EditorHandles_.push_back( EditorHandle );

		if( EditorHandle.CombinedID_ == SelectedResult_.SelectedID_ )
		{
			return SelectedResult_;
		}

		return HandleResult();
	}

	void DeselectHandle()
	{
		Action( SelectedResult_.SelectedID_, "Deselect",
			[]()
			{
				SelectedResult_.SelectedID_ = 0;
				SelectedResult_.WasClicked_ = false;
			},
			[ SelectedResult = SelectedResult_ ]()
			{
				SelectedResult_ = SelectedResult;
			} );	
	}

	void Action( BcU32 ID, const char* Name, ActionCallback Do, ActionCallback Undo, bool Commit )
	{
		BcU32 CombinedID = 0;
		CombinedID = BcHash::GenerateCRC32( CombinedID, &ID, sizeof( ID ) );
		CombinedID = BcHash::GenerateCRC32( CombinedID, Name, BcStrLength( Name ) );

		if( CombinedID != ActiveAction_.CombinedID_ )
		{
			if( ActiveAction_.CombinedID_ != 0 )
			{
				CommitAction();
			}

			ActiveAction_.Undo_ = Undo;
		}

		ActiveAction_.Do_ = Do;
		ActiveAction_.ID_ = ID;
		ActiveAction_.Name_ = Name;
		ActiveAction_.CombinedID_ = CombinedID;

		Do();
		RedoStack_.clear();

		if( Commit )
		{
			CommitAction();
		}
	}

	void CancelAction()
	{
		PSY_LOGSCOPEDCATEGORY( Editor );
		if( ActiveAction_ .CombinedID_ != 0 )
		{
			PSY_LOG( "CancelAction: %u, %s", ActiveAction_.ID_, ActiveAction_.Name_ );
		}

		if( ActiveAction_.Undo_ )
		{
			ActiveAction_.Undo_();
		}

		ActiveAction_ = EditorAction();
	}

	void CommitAction()
	{
		PSY_LOGSCOPEDCATEGORY( Editor );
		if( ActiveAction_ .CombinedID_ != 0 )
		{
			PSY_LOG( "CommitAction: %u, %s", ActiveAction_.ID_, ActiveAction_.Name_ );
		}

		if( ActiveAction_.CombinedID_ != 0 )
		{
			UndoStack_.push_back( ActiveAction_ );
			ActiveAction_ = EditorAction();
		}
	}

	void UndoAction()
	{
		PSY_LOG( "UndoAction: %u", UndoStack_.size() );
		CommitAction();
		if( UndoStack_.size() > 0 )
		{
			ActiveAction_ = UndoStack_.back();
			RedoStack_.push_back( ActiveAction_ );
			UndoStack_.pop_back();
			CancelAction();
		}
	}

	void RedoAction()
	{
		PSY_LOG( "RedoAction: %u", RedoStack_.size() );
		CommitAction();
		if( RedoStack_.size() > 0 )
		{
			ActiveAction_ = RedoStack_.back();
			RedoStack_.pop_back();
			ActiveAction_.Do_();
			CommitAction();
		}

	}
}
