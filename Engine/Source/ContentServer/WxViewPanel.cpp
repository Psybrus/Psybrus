/**************************************************************************
*
* File:		WxViewPanel.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		View panel.
*		
*
*
* 
**************************************************************************/

#include "WxViewPanel.h"

// Test render delegate.
void doRender( BcU32 Width, BcU32 Height )
{
	static int bleh = 1;
	static ScnMaterialRef MaterialRef;
	static ScnMaterialComponentRef MaterialComponentRef;
	static ScnCanvasComponentRef Canvas;
	if( MaterialRef.isValid() == BcFalse )
	{
		CsCore::pImpl()->importResource< ScnMaterial >( "EngineContent/default.material", MaterialRef );
		return;
	}
	if( MaterialRef.isReady() && MaterialComponentRef.isValid() == BcFalse )
	{
		CsCore::pImpl()->createResource( "MaterialComponent", MaterialComponentRef, MaterialRef, scnSPF_DEFAULT );
		CsCore::pImpl()->createResource( "canvas", Canvas, 4096, MaterialComponentRef );
		return;
	}

	if( Canvas.isReady() )
	{
		RsViewport Viewport( 0, 0, Width, Height );
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( NULL );
		pFrame->setRenderTarget( NULL );
		pFrame->setViewport( Viewport );
		Canvas->clear();
		Canvas->drawLine( BcVec2d( -1.0f, -1.0f ), BcVec2d( 1.0f, 1.0f ), RsColour::WHITE, 0 );
		Canvas->render( pFrame, 0 );
		RsCore::pImpl()->queueFrame( pFrame );
	}
}


//////////////////////////////////////////////////////////////////////////
// Event table.
BEGIN_EVENT_TABLE( WxViewPanel, wxPanel )
	EVT_SIZE(				WxViewPanel::OnSize )
	EVT_PAINT(				WxViewPanel::OnPaint )
	EVT_KEY_DOWN(			WxViewPanel::OnKeyDown )
	EVT_KEY_UP(				WxViewPanel::OnKeyUp )
	EVT_MOTION(				WxViewPanel::OnMouseMove )
	EVT_LEFT_DOWN(			WxViewPanel::OnMouseDown )
	EVT_LEFT_UP(			WxViewPanel::OnMouseUp )
	EVT_MIDDLE_DOWN(		WxViewPanel::OnMouseDown )
	EVT_MIDDLE_UP(			WxViewPanel::OnMouseUp )
	EVT_RIGHT_DOWN(			WxViewPanel::OnMouseDown )
	EVT_RIGHT_UP(			WxViewPanel::OnMouseUp )
	EVT_MOUSEWHEEL(			WxViewPanel::OnMouseWheel )
	EVT_LEAVE_WINDOW(		WxViewPanel::OnLeaveWindow )
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////////////
// Ctor
WxViewPanel::WxViewPanel( wxWindow* pParent ):
	wxPanel( pParent, wxID_ANY, wxDefaultPosition, wxSize( 32, 32 ), wxTAB_TRAVERSAL )
{
#if PLATFORM_WINDOWS
	// Start renderer if we haven't got one.
	if( RsCore::pImpl() == NULL )
	{
		// Start renderer.
		SysKernel::pImpl()->startSystem( "RsCore" );

		// Create a rendering context for this view.
		pContext_ = RsCore::pImpl()->getContext( this );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
WxViewPanel::~WxViewPanel()
{
	
}

//////////////////////////////////////////////////////////////////////////
// OnSize
//virtual
BcHandle WxViewPanel::getDeviceHandle()
{
	return (BcHandle)::GetDC( (HWND)GetHandle() );
}

//////////////////////////////////////////////////////////////////////////
// OnSize
//virtual
BcU32 WxViewPanel::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// OnSize
//virtual
BcU32 WxViewPanel::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// OnSize
void WxViewPanel::OnSize( wxSizeEvent& Event )
{
	// Update renderer resolution.
	Width_ = BcMin( (BcU32)2048, (BcU32)Event.GetSize().GetWidth() );
	Height_ = BcMin( (BcU32)2048, (BcU32)Event.GetSize().GetHeight() );
}

//////////////////////////////////////////////////////////////////////////
// OnPaint
void WxViewPanel::OnPaint( wxPaintEvent& Event )
{
	SysKernel::pImpl()->enqueueCallback< void(*)(BcU32, BcU32) >( BcDelegate< void(*)(BcU32,BcU32) >::bind< doRender >(), Width_, Height_ );

	//
	Event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OnKeyDown
void WxViewPanel::OnKeyDown( wxKeyEvent& Event )
{
	/*
	BcU32 ModifierFlags = ( Event.AltDown() ? EdUICommand::MF_ALT : 0 ) | ( Event.ControlDown() ? EdUICommand::MF_CTRL : 0 ) | ( Event.ShiftDown() ? EdUICommand::MF_SHIFT : 0 );
	BcU32 ValueFlags = Event.GetKeyCode();
	*/
	Event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OnKeyUp
void WxViewPanel::OnKeyUp( wxKeyEvent& Event )
{
	/*
	BcU32 ModifierFlags = ( Event.AltDown() ? EdUICommand::MF_ALT : 0 ) | ( Event.ControlDown() ? EdUICommand::MF_CTRL : 0 ) | ( Event.ShiftDown() ? EdUICommand::MF_SHIFT : 0 );
	BcU32 ValueFlags = Event.GetKeyCode();
	*/

	Event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OnMouseMove
void WxViewPanel::OnMouseMove( wxMouseEvent& Event )
{
	SysKernel::pImpl()->enqueueCallback< void(*)(BcU32, BcU32) >( BcDelegate< void(*)(BcU32,BcU32) >::bind< doRender >(), Width_, Height_ );

	/*
	BcU32 ModifierFlags = ( Event.AltDown() ? EdUICommand::MF_ALT : 0 ) | ( Event.ControlDown() ? EdUICommand::MF_CTRL : 0 ) | ( Event.ShiftDown() ? EdUICommand::MF_SHIFT : 0 );
	BcU32 ValueFlags = ( Event.LeftIsDown() ? EdUICommand::BTN_LEFT : 0 ) | ( Event.MiddleIsDown() ? EdUICommand::BTN_MIDDLE : 0 ) | ( Event.RightIsDown() ? EdUICommand::BTN_RIGHT : 0 );
	*/

	Event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OnMouseLeftDn
void WxViewPanel::OnMouseDown( wxMouseEvent& Event )
{
	/*
	BcU32 ModifierFlags = ( Event.AltDown() ? EdUICommand::MF_ALT : 0 ) | ( Event.ControlDown() ? EdUICommand::MF_CTRL : 0 ) | ( Event.ShiftDown() ? EdUICommand::MF_SHIFT : 0 );
	BcU32 ValueFlags = ( Event.LeftDown() ? EdUICommand::BTN_LEFT : 0 ) | ( Event.MiddleDown() ? EdUICommand::BTN_MIDDLE : 0 ) | ( Event.RightDown() ? EdUICommand::BTN_RIGHT : 0 );
	*/

	Event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OnMouseMiddleUp
void WxViewPanel::OnMouseUp( wxMouseEvent& Event )
{
	/*
	BcU32 ModifierFlags = ( Event.AltDown() ? EdUICommand::MF_ALT : 0 ) | ( Event.ControlDown() ? EdUICommand::MF_CTRL : 0 ) | ( Event.ShiftDown() ? EdUICommand::MF_SHIFT : 0 );
	BcU32 ValueFlags = ( Event.LeftUp() ? EdUICommand::BTN_LEFT : 0 ) | ( Event.MiddleUp() ? EdUICommand::BTN_MIDDLE : 0 ) | ( Event.RightUp() ? EdUICommand::BTN_RIGHT : 0 );
	*/

	Event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OnMouseWheel
void WxViewPanel::OnMouseWheel( wxMouseEvent& Event )
{
	/*
	BcU32 ModifierFlags = ( Event.AltDown() ? EdUICommand::MF_ALT : 0 ) | ( Event.ControlDown() ? EdUICommand::MF_CTRL : 0 ) | ( Event.ShiftDown() ? EdUICommand::MF_SHIFT : 0 );
	BcU32 ValueFlags = ( Event.LeftIsDown() ? EdUICommand::BTN_LEFT : 0 ) | ( Event.MiddleIsDown() ? EdUICommand::BTN_MIDDLE : 0 ) | ( Event.RightIsDown() ? EdUICommand::BTN_RIGHT : 0 );
	*/

	// Check the event is in bounds.
	if( Event.GetX() >= 0 && Event.GetX() < (int)Width_ && Event.GetY() >= 0 && Event.GetY() < (int)Height_ )
	{
		Event.Skip();
	}
}

//////////////////////////////////////////////////////////////////////////
// OnLeaveWindow
void WxViewPanel::OnLeaveWindow( wxMouseEvent& Event )
{
	Event.Skip();
}
