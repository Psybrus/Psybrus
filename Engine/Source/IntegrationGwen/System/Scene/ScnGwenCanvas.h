/**************************************************************************
*
* File:		ScnGwenCanvas.h
* Author:	Neil Richardson 
* Ver/Date:	13/02/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNGWENCANVAS_H__
#define __SCNGWENCANVAS_H__

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnFont.h"

#include "System/Os/OsEvents.h"

//////////////////////////////////////////////////////////////////////////
// Gwen Forward Declarations.
namespace Gwen
{
	namespace Controls
	{
		class Canvas;
		class Base;
	}

	namespace Skin
	{
		class TexturedBase;
	}
}

//////////////////////////////////////////////////////////////////////////
// References.
typedef CsResourceRef< class ScnGwenCanvas > ScnGwenCanvasRef;
typedef CsResourceRef< class ScnGwenCanvasComponent > ScnGwenCanvasComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnGwenCanvas
class ScnGwenCanvas:
	public CsResource
{
public:
	enum ControlType
	{
		CT_BASE = 0,			// Base control.
		CT_LABEL,				// Label.
		CT_BUTTON,				// Button.
		CT_UNITTEST,			// Unit test.

		//
		CT_MAX,
		CT_INVALID = BcErrorCode
	};

public:
	DECLARE_RESOURCE( CsResource, ScnGwenCanvas );
	
#ifdef PSY_IMPORT_PIPELINE
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );

#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	ScnMaterialRef						getMaterialByAlias( const BcChar* pAlias );
	ScnFontRef							getFontByAlias( const BcChar* pAlias );

private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );

private:
	friend class ScnGwenCanvasComponent;

	struct THeader
	{
		BcU32							NoofMaterials_;
		BcU32							NoofFonts_;
		BcU32							NoofControls_;
	};

	struct TMaterial
	{
		BcU32							MaterialAlias_;
		BcU32							MaterialName_;
	};

	struct TFont
	{
		BcU32							FontAlias_;
		BcU32							FontName_;
	};

	struct TControlBase
	{
		ControlType		Type_;
		BcU32			Size_;
		BcU32			Name_;
	};

	struct TControlLabel: TControlBase
	{
		BcS32			PositionX_;
		BcS32			PositionY_;
		BcS32			SizeX_;
		BcS32			SizeY_;
		BcU32			Text_;
	};

	struct TControlButton: TControlLabel
	{
		BcBool			IsToggle_;
		BcBool			ToggleState_;
	};

	struct TControlUnitTest: TControlBase
	{
		BcS32			PositionX_;
		BcS32			PositionY_;
		
	};
	
	THeader*							pHeader_;
	TMaterial*							pMaterials_;
	TFont*								pFonts_;
	TControlBase*						pControls_;

	std::vector< ScnMaterialRef >		Materials_;
	std::vector< ScnFontRef >			Fonts_;
};

//////////////////////////////////////////////////////////////////////////
// ScnGwenCanvasComponent
class ScnGwenCanvasComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnGwenCanvasComponent );
	
	virtual void						initialise( ScnGwenCanvasRef Parent );

	virtual void						update( BcReal tick );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcBool								createMaterialComponentByAlias( const BcChar* pAlias, ScnMaterialComponentRef& Component );
	BcBool								createFontComponentByAlias( const BcChar* pAlias, ScnFontComponentRef& Component );
	
	// NEILO TEMP: Should have input events we subscribe to, and game pushes in.
	eEvtReturn onKeyDown( EvtID id, const OsEventInputKeyboard& Event );
	eEvtReturn onKeyUp( EvtID id, const OsEventInputKeyboard& Event );
	eEvtReturn onMouseDown( EvtID id, const OsEventInputMouse& Event );
	eEvtReturn onMouseUp( EvtID id, const OsEventInputMouse& Event );
	eEvtReturn onMouseMove( EvtID id, const OsEventInputMouse& Event );
	
private:
	void									createControls();

private:
	ScnGwenCanvasRef						Parent_;
	class GwenRenderer*						pGwenRenderer_;
	class Gwen::Skin::TexturedBase*			pGwenSkinMaterial_;
	class Gwen::Controls::Canvas*			pGwenCanvas_;

	std::vector< ScnMaterialComponentRef >	MaterialComponents_;
	std::vector< ScnFontComponentRef >		FontComponents_;
	
	std::vector< Gwen::Controls::Base* >	Controls_;
};

#endif
