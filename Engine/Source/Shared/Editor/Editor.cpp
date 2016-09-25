#include "Editor/Editor.h"

#include "Editor/PanelComponentEditor.h"
#include "Editor/PanelSceneHierarchy.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Debug/DsImGuiFieldEditor.h"

#include "System/Os/OsCore.h"

#include "Math/MaQuat.h"
#include "Math/MaMat4d.h"

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
		ObjectID Object_;
		ActionCallback Do_;
		ActionCallback Undo_;

		BcU32 CombinedID_ = 0;
	};
	EditorAction ActiveAction_;
	std::deque< EditorAction > UndoStack_;
	std::deque< EditorAction > RedoStack_;

	HandleResult SelectedResult_;

	std::unordered_map< ObjectID, void* > IDToObjectRegistry_;
	std::unordered_map< void*, ObjectID > ObjectToIDRegistry_;
	ObjectID CurrentID_ = 1;

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
				ImGui::Begin( "#editor", nullptr, ImGui::GetIO().DisplaySize, 0, 
					ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus );
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

						if( !ImGui::IsMouseHoveringAnyWindow() )
						{
							for( int ButtonIdx = 0; ButtonIdx < 3; ++ButtonIdx )
							{
								if( MouseOver )

								if( ImGui::GetIO().MouseClicked[ ButtonIdx ] && MouseOver )
								{
									Action( Handle.CombinedID_, "Select", 0,
										[ Handle, ButtonIdx, MousePos ]( ObjectID )
										{
											SelectedResult_.WasClicked_ = true;
											SelectedResult_.SelectedID_ = Handle.CombinedID_;
											SelectedResult_.ButtonClicked_ = ButtonIdx;
											SelectedResult_.ScreenPosition_ = MousePos;
											SelectedResult_.WorldPosition_ = Handle.PointA_;
										},
										[ SelectedResult = SelectedResult_ ]( ObjectID )
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
									Action( Handle.CombinedID_, "Select", 0,
										[ Handle, ButtonIdx, MousePos, T ]( ObjectID )
										{
											SelectedResult_.WasClicked_ = true;
											SelectedResult_.SelectedID_ = Handle.CombinedID_;
											SelectedResult_.ButtonClicked_ = ButtonIdx;
											SelectedResult_.ScreenPosition_ = MousePos;
											SelectedResult_.WorldPosition_.lerp( Handle.PointA_, Handle.PointB_, BcClamp( T, 0.0f, 1.0f ) );
										},
										[ SelectedResult = SelectedResult_ ]( ObjectID )
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

	void SetupFieldEditorAttributes()
	{
		// Add some custom editors.
		ReManager::GetClass( ReTypeTraits< BcBool >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< bool >::Name() )->addAttribute(
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

		ReManager::GetClass(ReTypeTraits< BcU8 >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcS8 >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcU16 >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcS16 >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcU32 >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcS32 >::Name() )->addAttribute(
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



		ReManager::GetClass( ReTypeTraits< size_t >::Name() )->addAttribute(
			new DsImGuiFieldEditor(
			[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				size_t* Value = ( size_t* ) Object;
				if( *Value >= 0 && *Value <= INT_MAX )
				{
					int ValueInt = int( *Value );
					if ( ImGui::InputInt( Name.c_str(), &ValueInt, 1, 100 ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
					{
						*Value = ( size_t ) ValueInt;
					}
				}
			} ) );

		ReManager::GetClass( ReTypeTraits< BcF32 >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcF64 >::Name() )->addAttribute(
			new DsImGuiFieldEditor(
			[ ]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				BcF64* Value = ( BcF64* ) Object;
				float ValueF32 = float( *Value );
				if ( ImGui::InputFloat( Name.c_str(), &ValueF32, 0.1f, 1.0f ) && ( Flags & bcRFF_CONST ) == bcRFF_NONE )
				{
					*Value = ValueF32;
				}
			} ) );

		ReManager::GetClass( ReTypeTraits< std::string >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< BcName >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< MaVec2d >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< MaVec3d >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< MaVec4d >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< MaAABB >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< MaQuat >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< MaMat4d >::Name() )->addAttribute(
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

		ReManager::GetClass( ReTypeTraits< RsColour >::Name() )->addAttribute(
			new DsImGuiFieldEditor(
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
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

		ReManager::GetClass( ReTypeTraits< ReObject >::Name() )->addAttribute(
			new DsImGuiFieldEditor(
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* ObjectData, const ReClass* Class, ReFieldFlags Flags )
			{
				ObjectEditor( ThisFieldEditor, ObjectData, Class, Flags );
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

			// Setup field editors.
			SetupFieldEditorAttributes();
		}

		// Subscribe to F2 & F3 for screenshot
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN,
			[]( EvtID ID, const EvtBaseEvent& Event )
			{
				const auto& KeyEvent = Event.get< OsEventInputKeyboard >();
				if( KeyEvent.KeyCode_ == 'Z' && BcContainsAnyFlags( KeyEvent.Modifiers_, OsEventInputKeyboard::MODIFIER_CTRL ) )
				{
					UndoAction();
				}
				if( KeyEvent.KeyCode_ == 'Y' && BcContainsAnyFlags( KeyEvent.Modifiers_, OsEventInputKeyboard::MODIFIER_CTRL ) )
				{
					RedoAction();
				}
				return evtRET_PASS;
			} );
	}

	void ObjectEditor( DsImGuiFieldEditor* ThisFieldEditor, void* Data, const ReClass* Class, BcU32 Flags )
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
								ImGui::ScopedID ScopedID2( Value );
								if( FieldType->hasBaseClass( ReObject::StaticGetClass() ) )
								{
									if ( ImGui::TreeNode( Value, "%s", ( *Field->getName() ).c_str() ) )
									{
										FieldEditor->onEdit( " ", Value, UpperFieldType,
											ReFieldFlags( FieldAccessor.getFlags() | ( Flags & bcRFF_CONST ) ) );
										ImGui::TreePop();
									}
								}
								else
								{
									FieldEditor->onEdit( ( *Field->getName() ), Value, UpperFieldType,
										ReFieldFlags( FieldAccessor.getFlags() | ( Flags & bcRFF_CONST ) ) );
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
											ImGui::ScopedID ScopedID2( Idx );
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
											ImGui::ScopedID ScopedID2( Idx );
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
		Action( SelectedResult_.SelectedID_, "Deselect", 0,
			[]( ObjectID )
			{
				SelectedResult_.SelectedID_ = 0;
				SelectedResult_.WasClicked_ = false;
			},
			[ SelectedResult = SelectedResult_ ]( ObjectID )
			{
				SelectedResult_ = SelectedResult;
			} );	
	}

	void Action( BcU32 ID, const char* Name, ObjectID Object, ActionCallback Do, ActionCallback Undo, bool Commit )
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
		ActiveAction_.Object_ = Object;
		ActiveAction_.CombinedID_ = CombinedID;

		Do( Object );
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
			ActiveAction_.Undo_( ActiveAction_.Object_ );
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
		PSY_LOGSCOPEDCATEGORY( Editor );
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
		PSY_LOGSCOPEDCATEGORY( Editor );
		PSY_LOG( "RedoAction: %u", RedoStack_.size() );
		CommitAction();
		if( RedoStack_.size() > 0 )
		{
			ActiveAction_ = RedoStack_.back();
			RedoStack_.pop_back();
			ActiveAction_.Do_( ActiveAction_.Object_ );
			CommitAction();
		}
	}

	ObjectID GetObjectID( void* Object, ObjectID ID )
	{
		auto FoundIt = ObjectToIDRegistry_.find( Object );
		if( ID == 0 )
		{
			if( FoundIt != ObjectToIDRegistry_.end() )
			{
				return FoundIt->second;
			}

			BcAssertMsg( CurrentID_ != 0, "No more IDs." );
			if( CurrentID_ != 0 )
			{
				ID = CurrentID_++;
			}
		}
		if( FoundIt != ObjectToIDRegistry_.end() )
		{
			BcAssertMsg( FoundIt->second == ID, "Object already registered with different ID." );
		}
		IDToObjectRegistry_[ ID ] = Object;
		return ID;
	}

	void RemoveObjectID( void* Object, ObjectID ID )
	{
		if( ID != 0 )
		{
			auto FoundIt = IDToObjectRegistry_.find( ID );
			if( FoundIt != IDToObjectRegistry_.end() )
			{
				BcAssertMsg( FoundIt->second != Object, "ID and object don't match." );
				IDToObjectRegistry_.erase( FoundIt );
				ObjectToIDRegistry_.erase( ObjectToIDRegistry_.find( Object ) );
			}
		}
		else
		{
			for( auto It = IDToObjectRegistry_.begin(); It != IDToObjectRegistry_.end(); ++It )
			{
				if( It->second == Object )
				{
					IDToObjectRegistry_.erase( It );
					ObjectToIDRegistry_.erase( ObjectToIDRegistry_.find( Object ) );
					break;
				}
			}
		}
	}

	void* GetObject( ObjectID ID )
	{
		auto FoundIt = IDToObjectRegistry_.find( ID );
		if( FoundIt != IDToObjectRegistry_.end() )
		{
			return FoundIt->second;
		}
		return nullptr;
	}
}
