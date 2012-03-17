/**************************************************************************
*
* File:		ScnGwenCanvas.cpp
* Author:	Neil Richardson 
* Ver/Date:	13/02/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnGwenCanvas.h"

#include "GwenRenderer.h"

#include "Gwen/Gwen.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Skins/TexturedBase.h"

#include "Gwen/UnitTest/UnitTest.h"


#include "CsCore.h"
#include "OsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnGwenCanvas );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
//virtual
void ScnGwenCanvas::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	
}

//////////////////////////////////////////////////////////////////////////
// import
//virtual

#if PSY_SERVER
BcBool ScnGwenCanvas::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& ImportMaterials( Object[ "materials" ] );
	const Json::Value& ImportFonts( Object[ "fonts" ] );
	const Json::Value& ImportControls( Object[ "controls" ] );
	
	THeader Header;
		
	// Import materials.
	ScnMaterialMap Materials;
	Json::Value::Members MaterialMembers = ImportMaterials.getMemberNames();
	ScnMaterialRef MaterialRef;
	for( BcU32 Idx = 0; Idx < MaterialMembers.size(); ++Idx )
	{
		const Json::Value& Material = ImportMaterials[ MaterialMembers[ Idx ] ];
			
		if( CsCore::pImpl()->importObject( Material, MaterialRef, DependancyList ) )
		{
			Materials[ MaterialMembers[ Idx ] ] = MaterialRef;
		}	
		else
		{
			BcPrintf( "ScnMaterial: Failed to import material.\n" );
		}
	}

	// Import fonts.
	ScnFontMap Fonts;
	Json::Value::Members FontMembers = ImportFonts.getMemberNames();
	ScnFontRef FontRef;
	for( BcU32 Idx = 0; Idx < FontMembers.size(); ++Idx )
	{
		const Json::Value& Font = ImportFonts[ FontMembers[ Idx ] ];
			
		if( CsCore::pImpl()->importObject( Font, FontRef, DependancyList ) )
		{
			Fonts[ FontMembers[ Idx ] ] = FontRef;
		}	
		else
		{
			BcPrintf( "ScnFont: Failed to import font.\n" );
		}
	}

	// Setup header.
	Header.NoofMaterials_ = Materials.size();
	Header.NoofFonts_ = Fonts.size();

	// Setup materials.
	BcStream MaterialStream;

	for( ScnMaterialMapIterator It( Materials.begin() ); It != Materials.end(); ++It )
	{
		const std::string& Alias( (*It).first );
		ScnMaterialRef Material( (*It).second );
		TMaterial ExportMaterial = 
		{
			pFile_->addString( Alias.c_str() ),
			pFile_->addString( (*Material->getName()).c_str() )
		};

		MaterialStream << ExportMaterial;
	}
	
	// Setup fonts.
	BcStream FontStream;
	for( ScnFontMapIterator It( Fonts.begin() ); It != Fonts.end(); ++It )
	{
		const std::string& Alias( (*It).first );
		ScnFontRef Font( (*It).second );
		TFont ExportFont = 
		{
			pFile_->addString( Alias.c_str() ),
			pFile_->addString( (*Font->getName()).c_str() )
		};

		FontStream << ExportFont;
	}

	// Setup controls.
	std::map< std::string, ControlType > ControlTypes;
	ControlTypes[ "base" ] = CT_BASE;
	ControlTypes[ "label" ] = CT_LABEL;
	ControlTypes[ "button" ] = CT_BUTTON;
	ControlTypes[ "unittest" ] = CT_UNITTEST;

	Header.NoofControls_ = 0;

	BcStream ControlsStream;
	Json::Value::Members ControlsMembers = ImportControls.getMemberNames();
	for( BcU32 Idx = 0; Idx < ControlsMembers.size(); ++Idx )
	{
		const BcChar* pName = ControlsMembers[ Idx ].c_str();
		const Json::Value& Control = ImportControls[ ControlsMembers[ Idx ] ];
		{
			// Grab type.
			const BcChar* pType = Control[ "type" ].asCString();
			if( ControlTypes.find( pType ) != ControlTypes.end() )
			{
				ControlType Type = ControlTypes[ pType ];

				// Grab all parameters from control, and specify default types.
				std::string Text( Control.get( "text", "N/A" ).asCString() );
				BcVec2d Position( Control.get( "position", "0, 0" ).asCString() );
				BcVec2d Size( Control.get( "size", "100, 20" ).asCString() );
				BcBool IsToggle( Control.get( "istoggle", false ).asBool() );
				BcBool ToggleState( Control.get( "togglestate", false ).asBool() );
				
				switch( Type )
				{
				case CT_LABEL:
					{
						TControlLabel Control;
						Control.Type_ = Type;
						Control.Size_ = sizeof( TControlLabel );
						Control.Name_ = pFile_->addString( pName );
						Control.PositionX_ = static_cast< BcS32 >( Position.x() );
						Control.PositionY_ = static_cast< BcS32 >( Position.y() );
						Control.SizeX_ = static_cast< BcS32 >( Size.x() );
						Control.SizeY_ = static_cast< BcS32 >( Size.y() );		
						Control.Text_ = pFile_->addString( Text.c_str() );

						ControlsStream << Control;
						Header.NoofControls_++;
					}
					break;
				case CT_BUTTON:
					{
						TControlButton Control;
						Control.Type_ = Type;
						Control.Size_ = sizeof( TControlButton );
						Control.Name_ = pFile_->addString( pName );
						Control.PositionX_ = static_cast< BcS32 >( Position.x() );
						Control.PositionY_ = static_cast< BcS32 >( Position.y() );
						Control.SizeX_ = static_cast< BcS32 >( Size.x() );
						Control.SizeY_ = static_cast< BcS32 >( Size.y() );		
						Control.Text_ = pFile_->addString( Text.c_str() );
						Control.IsToggle_ = IsToggle;
						Control.ToggleState_ = ToggleState;

						ControlsStream << Control;
						Header.NoofControls_++;
					}
					break;
				case CT_UNITTEST:
					{
						TControlUnitTest Control;
						Control.Type_ = Type;
						Control.Size_ = sizeof( TControlUnitTest );
						Control.Name_ = pFile_->addString( pName );
						Control.PositionX_ = static_cast< BcS32 >( Position.x() );
						Control.PositionY_ = static_cast< BcS32 >( Position.y() );

						ControlsStream << Control;
						Header.NoofControls_++;
					}
					break;
				}
			}
		}		
	}

	// Write chunks.
	pFile_->addChunk( BcHash( "header" ), &Header, sizeof( Header ) );
	pFile_->addChunk( BcHash( "materials" ), MaterialStream.pData(), MaterialStream.dataSize() );
	pFile_->addChunk( BcHash( "fonts" ), FontStream.pData(), FontStream.dataSize() );
	pFile_->addChunk( BcHash( "controls" ), ControlsStream.pData(), ControlsStream.dataSize() );

	
	return BcTrue;
}
#endif

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnGwenCanvas::initialise()
{
	pHeader_ = NULL;
	pMaterials_ = NULL;
	pFonts_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnGwenCanvas::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnGwenCanvas::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnGwenCanvas::isReady()
{
	BcBool IsReady = pHeader_ != NULL && pMaterials_ != NULL && pFonts_ != NULL;

	if( IsReady )
	{
		for( BcU32 Idx = 0; Idx < pHeader_->NoofMaterials_; ++Idx )
		{
			IsReady &= Materials_[ Idx ].isReady();
		}

		for( BcU32 Idx = 0; Idx < pHeader_->NoofFonts_; ++Idx )
		{
			IsReady &= Fonts_[ Idx ].isReady();
		}
	}
	
	return IsReady;
}

//////////////////////////////////////////////////////////////////////////
// getMaterialByAlias
ScnMaterialRef ScnGwenCanvas::getMaterialByAlias( const BcChar* pAlias )
{
	for( BcU32 Idx = 0; Idx < pHeader_->NoofMaterials_; ++Idx )
	{
		if( BcStrCompare( pAlias, getString( pMaterials_[ Idx ].MaterialAlias_ ) ) )
		{
			return Materials_[ Idx ];
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getFontByAlias
ScnFontRef ScnGwenCanvas::getFontByAlias( const BcChar* pAlias )
{
	for( BcU32 Idx = 0; Idx < pHeader_->NoofFonts_; ++Idx )
	{
		if( BcStrCompare( pAlias, getString( pFonts_[ Idx ].FontAlias_ ) ) )
		{
			return Fonts_[ Idx ];
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
//virtual
void ScnGwenCanvas::fileReady()
{
	getChunk( 0 );
	getChunk( 1 );
	getChunk( 2 );
	getChunk( 3 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void ScnGwenCanvas::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		Materials_.reserve( pHeader_->NoofMaterials_ );
		Fonts_.reserve( pHeader_->NoofFonts_ );
	}
	else if( pChunk->ID_ == BcHash( "materials" ) )
	{
		pMaterials_ = (TMaterial*)pData;
		ScnMaterialRef Material;
		for( BcU32 Idx = 0; Idx < pHeader_->NoofMaterials_; ++Idx )
		{
			const TMaterial& MaterialLookup( pMaterials_[ Idx ] );
			const BcChar* pMaterialName( getString( MaterialLookup.MaterialName_ ) );
			BcBool Result = CsCore::pImpl()->requestResource( pMaterialName, Material );
			BcAssertMsg( Result, "ScnGwenCanvas: Failed to import material \"%s\"", pMaterialName );
			Materials_.push_back( Material );
		}
	}
	else if( pChunk->ID_ == BcHash( "fonts" ) )
	{
		pFonts_ = (TFont*)pData;
		ScnFontRef Font;
		for( BcU32 Idx = 0; Idx < pHeader_->NoofFonts_; ++Idx )
		{
			const TFont& FontLookup( pFonts_[ Idx ] );
			const BcChar* pFontName( getString( FontLookup.FontName_ ) );
			BcBool Result = CsCore::pImpl()->requestResource( pFontName, Font );
			BcAssertMsg( Result, "ScnGwenCanvas: Failed to import font \"%s\"", pFontName );
			Fonts_.push_back( Font );	
		}
	}
	else if( pChunk->ID_ == BcHash( "controls" ) )
	{
		pControls_ = (TControlBase*)pData;
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnGwenCanvasComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
//virtual
void ScnGwenCanvasComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnGwenCanvasComponent::initialise( ScnGwenCanvasRef Parent )
{
	Parent_ = Parent;
	pGwenRenderer_ = NULL;
	pGwenSkinMaterial_ = NULL;
	pGwenCanvas_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnGwenCanvasComponent::update( BcReal tick )
{
	// Get client size.
	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	pGwenCanvas_->SetSize( pClient->getWidth(), pClient->getHeight() );

	// Render the canvas.
	pGwenCanvas_->RenderCanvas();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnGwenCanvasComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Create a new Gwen renderer.
	BcAssert( pGwenRenderer_ == NULL );
	pGwenRenderer_ = new GwenRenderer( ScnEntityRef( Parent ), this );
	pGwenRenderer_->Init();

	// Create a new Gwen skin.
	BcAssert( pGwenSkinMaterial_ == NULL );
	pGwenSkinMaterial_ = new Gwen::Skin::TexturedBase();
	pGwenSkinMaterial_->SetRender( pGwenRenderer_ );
	pGwenSkinMaterial_->Init( "skin" );

	// Load the default font.
	pGwenRenderer_->LoadFont( pGwenSkinMaterial_->GetDefaultFont() );

	// Create a new Gwen canvas.
	BcAssert( pGwenCanvas_ == NULL );
	pGwenCanvas_ = new Gwen::Controls::Canvas( pGwenSkinMaterial_ );
	pGwenCanvas_->SetSize( 800, 600 );
	pGwenCanvas_->SetDrawBackground( false );

	// Create controls.
	createControls();
	
	//
	OsEventInputKeyboard::Delegate OnKeyDown = OsEventInputKeyboard::Delegate::bind< ScnGwenCanvasComponent, &ScnGwenCanvasComponent::onKeyDown >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyDown );
	OsEventInputKeyboard::Delegate OnKeyUp = OsEventInputKeyboard::Delegate::bind< ScnGwenCanvasComponent, &ScnGwenCanvasComponent::onKeyUp >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyUp );

	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< ScnGwenCanvasComponent, &ScnGwenCanvasComponent::onMouseDown >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< ScnGwenCanvasComponent, &ScnGwenCanvasComponent::onMouseUp >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< ScnGwenCanvasComponent, &ScnGwenCanvasComponent::onMouseMove >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );

	//
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnGwenCanvasComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	// Destroy old Gwen canvas.
	BcAssert( pGwenCanvas_ != NULL );
	delete pGwenCanvas_;
	pGwenCanvas_ = NULL;

	// Destroy old Gwen skin.
	BcAssert( pGwenSkinMaterial_ != NULL );
	delete pGwenSkinMaterial_;
	pGwenSkinMaterial_ = NULL;
		
	// Destroy old Gwen renderer.
	BcAssert( pGwenRenderer_ != NULL );
	delete pGwenRenderer_;
	pGwenRenderer_ = NULL;
	
	//
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// createMaterialComponentByAlias
BcBool ScnGwenCanvasComponent::createMaterialComponentByAlias( const BcChar* pAlias, ScnMaterialComponentRef& Component )
{
	ScnMaterialRef Material = Parent_->getMaterialByAlias( pAlias );
	if( Material.isValid() )
	{
		return CsCore::pImpl()->createResource( BcName::INVALID, Component, Material, BcErrorCode );
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// createFontComponentByAlias
BcBool ScnGwenCanvasComponent::createFontComponentByAlias( const BcChar* pAlias, ScnFontComponentRef& Component )
{
	ScnMaterialRef Material = Parent_->getMaterialByAlias( pAlias );
	ScnFontRef Font = Parent_->getFontByAlias( pAlias );
	if( Material.isValid() && Font.isValid() )
	{
		return CsCore::pImpl()->createResource( BcName::INVALID, Component, Font, Material );
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// onKeyDown
eEvtReturn ScnGwenCanvasComponent::onKeyDown( EvtID id, const OsEventInputKeyboard& Event)
{
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onKeyUp
eEvtReturn ScnGwenCanvasComponent::onKeyUp( EvtID id, const OsEventInputKeyboard& Event)
{
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn ScnGwenCanvasComponent::onMouseDown( EvtID id, const OsEventInputMouse& Event )
{
	pGwenCanvas_->InputMouseButton( Event.ButtonCode_, true );

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseUp
eEvtReturn ScnGwenCanvasComponent::onMouseUp( EvtID id, const OsEventInputMouse& Event )
{
	pGwenCanvas_->InputMouseButton( Event.ButtonCode_, false );

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseMove
eEvtReturn ScnGwenCanvasComponent::onMouseMove( EvtID id, const OsEventInputMouse& Event )
{
	pGwenCanvas_->InputMouseMoved( Event.MouseX_, Event.MouseY_, Event.MouseDX_, Event.MouseDY_ );
	
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// createControls
void ScnGwenCanvasComponent::createControls()
{
	ScnGwenCanvas::THeader* pHeader = Parent_->pHeader_;
	ScnGwenCanvas::TControlBase* pControl = Parent_->pControls_;
		
	for( BcU32 Idx = 0; Idx < pHeader->NoofControls_; ++Idx )
	{
		switch( pControl->Type_ )
		{
		case ScnGwenCanvas::CT_LABEL:
			{
				ScnGwenCanvas::TControlLabel* pControlDef = static_cast< ScnGwenCanvas::TControlLabel* >( pControl );
				Gwen::Controls::Label* pLabel = new Gwen::Controls::Label( pGwenCanvas_ );
				pLabel->SetPos( pControlDef->PositionX_, pControlDef->PositionY_ );
				pLabel->SetSize( pControlDef->SizeX_, pControlDef->SizeY_ );
				pLabel->SetText( Parent_->getString( pControlDef->Text_ ) );
				Controls_.push_back( pLabel );
			}
			break;

		case ScnGwenCanvas::CT_BUTTON:
			{
				ScnGwenCanvas::TControlButton* pControlDef = static_cast< ScnGwenCanvas::TControlButton* >( pControl );
				Gwen::Controls::Button* pButton = new Gwen::Controls::Button( pGwenCanvas_ );
				pButton->SetPos( pControlDef->PositionX_, pControlDef->PositionY_ );
				pButton->SetSize( pControlDef->SizeX_, pControlDef->SizeY_ );
				pButton->SetText( Parent_->getString( pControlDef->Text_ ) );
				pButton->SetIsToggle( pControlDef->IsToggle_ ? true : false );
				pButton->SetToggleState( pControlDef->ToggleState_ ? true : false );

				Controls_.push_back( pButton );
			}
			break;

		case ScnGwenCanvas::CT_UNITTEST:
			{
				ScnGwenCanvas::TControlUnitTest* pControlDef = static_cast< ScnGwenCanvas::TControlUnitTest* >( pControl );
				UnitTest* pUnit = new UnitTest( pGwenCanvas_ );
				pUnit->SetPos( pControlDef->PositionX_, pControlDef->PositionY_ );
				
				Controls_.push_back( pUnit );
			}
			break;
		}
		
		// Advance to next control.
		pControl = reinterpret_cast< ScnGwenCanvas::TControlBase* >( reinterpret_cast< BcU8* >( pControl ) + pControl->Size_ );
	}
}
