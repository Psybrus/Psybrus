/////////////////////////////////////////////////////////////////////////////
// Name:        editors.cpp
// Purpose:     wxPropertyGrid editors
// Author:      Jaakko Salli
// Modified by:
// Created:     Apr-14-2007
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/object.h"
    #include "wx/hash.h"
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/window.h"
    #include "wx/panel.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/dcmemory.h"
    #include "wx/button.h"
    #include "wx/pen.h"
    #include "wx/brush.h"
    #include "wx/cursor.h"
    #include "wx/dialog.h"
    #include "wx/settings.h"
    #include "wx/msgdlg.h"
    #include "wx/choice.h"
    #include "wx/stattext.h"
    #include "wx/scrolwin.h"
    #include "wx/dirdlg.h"
    #include "wx/layout.h"
    #include "wx/sizer.h"
    #include "wx/textdlg.h"
    #include "wx/filedlg.h"
    #include "wx/statusbr.h"
    #include "wx/intl.h"
    #include "wx/frame.h"
#endif


#include "wx/timer.h"
#include "wx/dcbuffer.h"
#include "wx/bmpbuttn.h"


// This define is necessary to prevent macro clearing
#define __wxPG_SOURCE_FILE__

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/editors.h>
#include <wx/propgrid/props.h>

#ifdef __WXPYTHON__
    #include <wx/propgrid/advprops.h>
    #include <wx/propgrid/extras.h>
#endif

#if wxPG_USE_RENDERER_NATIVE
    #include <wx/renderer.h>
#endif

// How many pixels between textctrl and button
#ifdef __WXMAC__
    // Backported from wx2.9 by Julian Smart (old value was 8)
    #define wxPG_TEXTCTRL_AND_BUTTON_SPACING        4
#else
    #define wxPG_TEXTCTRL_AND_BUTTON_SPACING        2
#endif 

#define wxPG_BUTTON_SIZEDEC                         0

#if wxPG_USING_WXOWNERDRAWNCOMBOBOX
    #include <wx/odcombo.h>
#else
    #include <wx/propgrid/odcombo.h>
#endif

#ifdef __WXMSW__
    #include <wx/msw/private.h>
#endif

// -----------------------------------------------------------------------

#if defined(__WXMSW__)
    // tested
    #define wxPG_NAT_TEXTCTRL_BORDER_X          0 // Unremovable border of native textctrl.
    #define wxPG_NAT_TEXTCTRL_BORDER_Y          0 // Unremovable border of native textctrl.

    #define wxPG_NAT_BUTTON_BORDER_ANY          1
    #define wxPG_NAT_BUTTON_BORDER_X            1
    #define wxPG_NAT_BUTTON_BORDER_Y            1

    #define wxPG_CHECKMARK_XADJ                 1
    #define wxPG_CHECKMARK_YADJ                 (-1)
    #define wxPG_CHECKMARK_WADJ                 0
    #define wxPG_CHECKMARK_HADJ                 0
    #define wxPG_CHECKMARK_DEFLATE              0

    #define wxPG_TEXTCTRLYADJUST                (m_spacingy+0)

#elif defined(__WXGTK__)
    // tested
    #define wxPG_CHECKMARK_XADJ                 0
    #define wxPG_CHECKMARK_YADJ                 0
    #define wxPG_CHECKMARK_WADJ                 (-1)
    #define wxPG_CHECKMARK_HADJ                 (-1)
    #define wxPG_CHECKMARK_DEFLATE              3

    #define wxPG_NAT_TEXTCTRL_BORDER_X      3 // Unremovable border of native textctrl.
    #define wxPG_NAT_TEXTCTRL_BORDER_Y      3 // Unremovable border of native textctrl.

    #define wxPG_NAT_BUTTON_BORDER_ANY      1
    #define wxPG_NAT_BUTTON_BORDER_X        1
    #define wxPG_NAT_BUTTON_BORDER_Y        1

    #define wxPG_TEXTCTRLYADJUST            0

#elif defined(__WXMAC__)
    // *not* tested
    #define wxPG_CHECKMARK_XADJ                 0
    #define wxPG_CHECKMARK_YADJ                 0
    #define wxPG_CHECKMARK_WADJ                 0
    #define wxPG_CHECKMARK_HADJ                 0
    #define wxPG_CHECKMARK_DEFLATE              0

    #define wxPG_NAT_TEXTCTRL_BORDER_X      0 // Unremovable border of native textctrl.
    #define wxPG_NAT_TEXTCTRL_BORDER_Y      0 // Unremovable border of native textctrl.

    #define wxPG_NAT_BUTTON_BORDER_ANY      0
    #define wxPG_NAT_BUTTON_BORDER_X        0
    #define wxPG_NAT_BUTTON_BORDER_Y        0

    // Backported from wx2.9 by Julian Smart (old value was 3)
    #define wxPG_TEXTCTRLYADJUST            0

#else
    // defaults
    #define wxPG_CHECKMARK_XADJ                 0
    #define wxPG_CHECKMARK_YADJ                 0
    #define wxPG_CHECKMARK_WADJ                 0
    #define wxPG_CHECKMARK_HADJ                 0
    #define wxPG_CHECKMARK_DEFLATE              0

    #define wxPG_NAT_TEXTCTRL_BORDER_X      0 // Unremovable border of native textctrl.
    #define wxPG_NAT_TEXTCTRL_BORDER_Y      0 // Unremovable border of native textctrl.

    #define wxPG_NAT_BUTTON_BORDER_ANY      0
    #define wxPG_NAT_BUTTON_BORDER_X        0
    #define wxPG_NAT_BUTTON_BORDER_Y        0

    #define wxPG_TEXTCTRLYADJUST            0

#endif

#if (!wxPG_NAT_TEXTCTRL_BORDER_X && !wxPG_NAT_TEXTCTRL_BORDER_Y)
    #define wxPG_ENABLE_CLIPPER_WINDOW      0
#else
    #define wxPG_ENABLE_CLIPPER_WINDOW      1
#endif


// for odcombo
#ifdef __WXMAC__
    // Backported from wx2.9 by Julian Smart
    // required because wxComboCtrl reserves 3pixels for wxTextCtrl's
    // focus ring.
    #define wxPG_CHOICEXADJUST           -3
    #define wxPG_CHOICEYADJUST           -3
#else
    #define wxPG_CHOICEXADJUST           0
    #define wxPG_CHOICEYADJUST           0
#endif

//
// Number added to image width for SetCustomPaintWidth
// NOTE: Use different custom paint margin because of better textctrl spacing
#define ODCB_CUST_PAINT_MARGIN_RO            6
#define ODCB_CUST_PAINT_MARGIN               8

// Milliseconds to wait for two mouse-ups after focus inorder
// to trigger a double-click.
#define DOUBLE_CLICK_CONVERSION_TRESHOLD        500

// -----------------------------------------------------------------------
// wxPGEditor
// -----------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxPGEditor, wxObject)

wxPGEditor::~wxPGEditor()
{
}


/*wxPGCellRenderer* wxPGEditor::GetCellRenderer() const
{
    return &g_wxPGDefaultRenderer;
}*/

void wxPGEditor::DrawValue( wxDC& dc, const wxRect& rect, wxPGProperty* property, const wxString& text ) const
{
    if ( !property->IsValueUnspecified() )
        dc.DrawText( text, rect.x+wxPG_XBEFORETEXT, rect.y );
}

#ifdef __WXPYTHON__
bool wxPGEditor::GetValueFromControl( wxVariant&, wxPGProperty*, wxWindow* ) const
{
    return false;
}

wxPGVariantAndBool wxPGEditor::PyGetValueFromControl( wxPGProperty* property, wxWindow* ctrl ) const
{
    wxPGVariantAndBool vab;
    vab.m_result = GetValueFromControl(vab.m_value, property, ctrl);
    if ( vab.m_result )
        vab.m_valueValid = true;
    return vab;
}
#endif

void wxPGEditor::SetControlStringValue( wxPGProperty* WXUNUSED(property), wxWindow*, const wxString& ) const
{
}


void wxPGEditor::SetControlIntValue( wxPGProperty* WXUNUSED(property), wxWindow*, int ) const
{
}


int wxPGEditor::InsertItem( wxWindow*, const wxString&, int ) const
{
    return -1;
}


void wxPGEditor::DeleteItem( wxWindow*, int ) const
{
    return;
}


void wxPGEditor::OnFocus( wxPGProperty*, wxWindow* ) const
{
}


bool wxPGEditor::CanContainCustomImage() const
{
    return false;
}

// -----------------------------------------------------------------------
// wxPGClipperWindow
// -----------------------------------------------------------------------

#if wxPG_ENABLE_CLIPPER_WINDOW

//
// Clipper window is used to "remove" borders from controls
// which otherwise insist on having them despite of supplied
// wxNO_BORDER window style.
//
class wxPGClipperWindow : public wxWindow
{
    DECLARE_CLASS(wxPGClipperWindow)
public:

    wxPGClipperWindow()
        : wxWindow()
    {
        wxPGClipperWindow::Init();
    }

    wxPGClipperWindow(wxWindow* parent,
                      wxWindowID id,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize)
    {
        Init();
        Create(parent,id,pos,size);
    }

    void Create(wxWindow* parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize);

    virtual ~wxPGClipperWindow();

    virtual bool ProcessEvent(wxEvent& event);

    inline wxWindow* GetControl() const { return m_ctrl; }

    // This is called before wxControl is constructed.
    void GetControlRect( int xadj, int yadj, wxPoint& pt, wxSize& sz );

    // This is caleed after wxControl has been constructed.
    void SetControl( wxWindow* ctrl );

    virtual void Refresh( bool eraseBackground = true,
                          const wxRect *rect = (const wxRect *) NULL );
    virtual void SetFocus();

    virtual bool SetFont(const wxFont& font);

    virtual bool SetForegroundColour(const wxColour& col)
    {
        bool res = wxWindow::SetForegroundColour(col);
        if ( m_ctrl )
            m_ctrl->SetForegroundColour(col);
        return res;
    }

    virtual bool SetBackgroundColour(const wxColour& col)
    {
        bool res = wxWindow::SetBackgroundColour(col);
        if ( m_ctrl )
            m_ctrl->SetBackgroundColour(col);
        return res;
    }

    inline int GetXClip() const { return m_xadj; }

    inline int GetYClip() const { return m_yadj; }

protected:
    wxWindow*       m_ctrl;

    int             m_xadj; // Horizontal border clip.

    int             m_yadj; // Vertical border clip.

private:
    void Init ()
    {
        m_ctrl = (wxWindow*) NULL;
    }
};


IMPLEMENT_CLASS(wxPGClipperWindow,wxWindow)


// This is called before wxControl is constructed.
void wxPGClipperWindow::GetControlRect( int xadj, int yadj, wxPoint& pt, wxSize& sz )
{
    m_xadj = xadj;
    m_yadj = yadj;
    pt.x = -xadj;
    pt.y = -yadj;
    wxSize own_size = GetSize();
    sz.x = own_size.x+(xadj*2);
    sz.y = own_size.y+(yadj*2);
}


// This is caleed after wxControl has been constructed.
void wxPGClipperWindow::SetControl( wxWindow* ctrl )
{
    m_ctrl = ctrl;

    // GTK requires this.
    ctrl->SetSizeHints(3,3);

    // Correct size of this window to match the child.
    wxSize sz = GetSize();
    wxSize chsz = ctrl->GetSize();

    int hei_adj = chsz.y - (sz.y+(m_yadj*2));
    if ( hei_adj )
        SetSize(sz.x,chsz.y-(m_yadj*2));

}


void wxPGClipperWindow::Refresh( bool eraseBackground, const wxRect *rect )
{
    wxWindow::Refresh(false,rect);
    if ( m_ctrl )
        m_ctrl->Refresh(eraseBackground);
}


// Pass focus to control
void wxPGClipperWindow::SetFocus()
{
    if ( m_ctrl )
        m_ctrl->SetFocus();
    else
        wxWindow::SetFocus();
}


bool wxPGClipperWindow::SetFont(const wxFont& font)
{
    bool res = wxWindow::SetFont(font);
    if ( m_ctrl )
        return m_ctrl->SetFont(font);
    return res;
}


void wxPGClipperWindow::Create(wxWindow* parent,
                               wxWindowID id,
                               const wxPoint& pos,
                               const wxSize& size )
{
    wxWindow::Create(parent,id,pos,size);
}


wxPGClipperWindow::~wxPGClipperWindow()
{
}


bool wxPGClipperWindow::ProcessEvent(wxEvent& event)
{
    if ( event.GetEventType() == wxEVT_SIZE )
    {
        if ( m_ctrl )
        {
            // Maintain correct size relationship.
            wxSize sz = GetSize();
            m_ctrl->SetSize(sz.x+(m_xadj*2),sz.y+(m_yadj*2));
            event.Skip();
            return false;
        }
    }
    return wxWindow::ProcessEvent(event);
}

#endif // wxPG_ENABLE_CLIPPER_WINDOW

/*wxWindow* wxPropertyGrid::GetActualEditorControl( wxWindow* ctrl )
{
#if wxPG_ENABLE_CLIPPER_WINDOW
    // Pass real control instead of clipper window
    if ( ctrl->IsKindOf(CLASSINFO(wxPGClipperWindow)) )
    {
        return ((wxPGClipperWindow*)ctrl)->GetControl();
    }
#else
    return ctrl;
#endif
}*/

// -----------------------------------------------------------------------
// wxPGTextCtrlEditor
// -----------------------------------------------------------------------

// Clipper window support macro (depending on whether it is used
// for this editor or not)
#if wxPG_NAT_TEXTCTRL_BORDER_X || wxPG_NAT_TEXTCTRL_BORDER_Y
    #define wxPG_NAT_TEXTCTRL_BORDER_ANY    1
#else
    #define wxPG_NAT_TEXTCTRL_BORDER_ANY    0
#endif


WX_PG_IMPLEMENT_EDITOR_CLASS(TextCtrl,wxPGTextCtrlEditor,wxPGEditor)


wxPGWindowList wxPGTextCtrlEditor::CreateControls( wxPropertyGrid* propGrid,
                                                   wxPGProperty* property,
                                                   const wxPoint& pos,
                                                   const wxSize& sz ) const
{
    wxString text;

    //
    // If has children, and limited editing is specified, then don't create.
    if ( (property->GetFlags() & wxPG_PROP_NOEDITOR) &&
         property->GetChildCount() )
        return (wxWindow*) NULL;

    if ( !property->IsValueUnspecified() )
    {
        int flags = property->HasFlag(wxPG_PROP_READONLY) ? 
            0 : wxPG_EDITABLE_VALUE;
        text = property->GetValueString(flags);
    }
    else
    {
        text = propGrid->GetUnspecifiedValueText();
    }

    int flags = 0;
    if ( (property->GetFlags() & wxPG_PROP_PASSWORD) &&
         property->IsKindOf(WX_PG_CLASSINFO(wxStringProperty)) )
        flags |= wxTE_PASSWORD;

    wxWindow* wnd = propGrid->GenerateEditorTextCtrl(pos,sz,text,(wxWindow*)NULL,flags,
                                                     property->GetMaxLength());

    return wnd;
}

#if 0
void wxPGTextCtrlEditor::DrawValue( wxDC& dc, wxPGProperty* property, const wxRect& rect ) const
{
    if ( !property->IsValueUnspecified() )
    {
        wxString drawStr = property->GetDisplayedString();

        // Code below should no longer be needed, as the obfuscation
        // is now done in GetValueAsString.
        /*if ( (property->GetFlags() & wxPG_PROP_PASSWORD) &&
             property->IsKindOf(WX_PG_CLASSINFO(wxStringProperty)) )
        {
            size_t a = drawStr.length();
            drawStr.Empty();
            drawStr.Append(wxT('*'),a);
        }*/
        dc.DrawText( drawStr, rect.x+wxPG_XBEFORETEXT, rect.y );
    }
}
#endif

void wxPGTextCtrlEditor::UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const
{
    wxTextCtrl* tc = wxStaticCast(ctrl, wxTextCtrl);

    wxString s;

    if ( tc->HasFlag(wxTE_PASSWORD) )
        s = property->GetValueAsString(wxPG_FULL_VALUE);
    else
        s = property->GetDisplayedString();

    wxPropertyGrid* pg = property->GetGrid();

    pg->SetupTextCtrlValue(s);
    tc->SetValue(s);    

    // Must always fix indentation, just in case
#if defined(__WXMSW__) && !defined(__WXWINCE__)
        ::SendMessage(GetHwndOf(tc), EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
#endif
}


// Provided so that, for example, ComboBox editor can use the same code
// (multiple inheritance would get way too messy).
bool wxPGTextCtrlEditor::OnTextCtrlEvent( wxPropertyGrid* propGrid,
                                          wxPGProperty* WXUNUSED(property),
                                          wxWindow* ctrl,
                                          wxEvent& event )
{
    if ( !ctrl )
        return false;

    if ( event.GetEventType() == wxEVT_COMMAND_TEXT_ENTER )
    {
        if ( propGrid->IsEditorsValueModified() )
        {
            return true;
        }
    }
    else if ( event.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED )
    {
        //
        // Pass this event outside wxPropertyGrid so that,
        // if necessary, program can tell when user is editing
        // a textctrl.
        // FIXME: Is it safe to change event id in the middle of event
        //        processing (seems to work, but...)?
        event.Skip();
        event.SetId(propGrid->GetId());

        propGrid->EditorsValueWasModified();
    }
    return false;
}


bool wxPGTextCtrlEditor::OnEvent( wxPropertyGrid* propGrid,
                                  wxPGProperty* property,
                                  wxWindow* ctrl,
                                  wxEvent& event ) const
{
    return wxPGTextCtrlEditor::OnTextCtrlEvent(propGrid,property,ctrl,event);
}


bool wxPGTextCtrlEditor::GetTextCtrlValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl )
{
    wxTextCtrl* tc = wxStaticCast(ctrl, wxTextCtrl);
    wxString textVal = tc->GetValue();

    if ( property->UsesAutoUnspecified() && !textVal.length() )
    {
        variant.MakeNull();
        return true;
    }

    bool res = property->ActualStringToValue(variant, textVal, wxPG_EDITABLE_VALUE);

    // Changing unspecified always causes event (returning
    // true here should be enough to trigger it).
    // TODO: Move to propgrid.cpp
    if ( !res && variant.IsNull() )
        res = true;

    return res;
}


bool wxPGTextCtrlEditor::GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl ) const
{
    return wxPGTextCtrlEditor::GetTextCtrlValueFromControl(variant, property, ctrl);
}


void wxPGTextCtrlEditor::SetValueToUnspecified( wxPGProperty* property, wxWindow* ctrl ) const
{
    wxTextCtrl* tc = wxStaticCast(ctrl, wxTextCtrl);

    wxPropertyGrid* pg = property->GetGrid();
    wxASSERT(pg);  // Really, property grid should exist if editor does
    if ( pg )
    {
        wxString tcText = pg->GetUnspecifiedValueText();
        pg->SetupTextCtrlValue(tcText);
        tc->SetValue(tcText);
    }
}


void wxPGTextCtrlEditor::SetControlStringValue( wxPGProperty* property, wxWindow* ctrl, const wxString& txt ) const
{
    wxTextCtrl* tc = wxStaticCast(ctrl, wxTextCtrl);

    wxPropertyGrid* pg = property->GetGrid();
    wxASSERT(pg);  // Really, property grid should exist if editor does
    if ( pg )
        tc->SetValue(txt);
}

void wxPGTextCtrlEditor_OnFocus( wxPGProperty* property,
                                 wxTextCtrl*tc )
{
    // Make sure there is correct text (instead of unspecified value
    // indicator or inline help)
    int flags = property->HasFlag(wxPG_PROP_READONLY) ? 
        0 : wxPG_EDITABLE_VALUE;
    wxString correctText = property->GetValueString(flags);

    if ( tc->GetValue() != correctText )
    {
        property->GetGrid()->SetupTextCtrlValue(correctText);
        tc->SetValue(correctText);
    }

    tc->SetSelection(-1,-1);
}

void wxPGTextCtrlEditor::OnFocus( wxPGProperty* property,
                                  wxWindow* wnd ) const
{
    wxTextCtrl* tc = wxStaticCast(wnd, wxTextCtrl);
    wxPGTextCtrlEditor_OnFocus(property, tc);
}

wxPGTextCtrlEditor::~wxPGTextCtrlEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(TextCtrl) = NULL;
}

// -----------------------------------------------------------------------
// wxPGChoiceEditor
// -----------------------------------------------------------------------


WX_PG_IMPLEMENT_EDITOR_CLASS(Choice,wxPGChoiceEditor,wxPGEditor)


// This is a special enhanced double-click processor class.
// In essence, it allows for double-clicks for which the
// first click "created" the control.
class wxPGDoubleClickProcessor : public wxEvtHandler
{
public:

    wxPGDoubleClickProcessor( wxPGOwnerDrawnComboBox* combo )
        : wxEvtHandler()
    {
        m_timeLastMouseUp = 0;
        m_combo = combo;
        m_downReceived = false;
    }

protected:

    void OnMouseEvent( wxMouseEvent& event )
    {
        wxLongLong t = ::wxGetLocalTimeMillis();
        int evtType = event.GetEventType();

        if ( m_combo->HasFlag(wxPGCC_DCLICK_CYCLES) &&
             !m_combo->IsPopupShown() )
        {
            // Just check that it is in the text area
            wxPoint pt = event.GetPosition();
            if ( m_combo->GetTextRect().wxPGRectContains(pt) )
            {
                if ( evtType == wxEVT_LEFT_DOWN )
                {
                    // Set value to avoid up-events without corresponding downs
                    m_downReceived = true;
                }
                else if ( evtType == wxEVT_LEFT_DCLICK )
                {
                    // We'll make our own double-clicks
                    event.SetEventType(0);
                    return;
                }
                else if ( evtType == wxEVT_LEFT_UP )
                {
                    if ( m_downReceived || m_timeLastMouseUp == 1 )
                    {
                        wxLongLong timeFromLastUp = (t-m_timeLastMouseUp);

                        if ( timeFromLastUp < DOUBLE_CLICK_CONVERSION_TRESHOLD )
                        {
                            event.SetEventType(wxEVT_LEFT_DCLICK);
                            m_timeLastMouseUp = 1;
                        }
                        else
                        {
                            m_timeLastMouseUp = t;
                        }
                    }
                }
            }
        }

        event.Skip();
    }

    void OnSetFocus( wxFocusEvent& event )
    {
        m_timeLastMouseUp = ::wxGetLocalTimeMillis();
        event.Skip();
    }

private:
    wxLongLong                  m_timeLastMouseUp;
    wxPGOwnerDrawnComboBox*     m_combo;
    bool                        m_downReceived;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxPGDoubleClickProcessor, wxEvtHandler)
    EVT_MOUSE_EVENTS(wxPGDoubleClickProcessor::OnMouseEvent)
    EVT_SET_FOCUS(wxPGDoubleClickProcessor::OnSetFocus)
END_EVENT_TABLE()



class wxPGComboBox : public wxPGOwnerDrawnComboBox
{
public:

    wxPGComboBox()
        : wxPGOwnerDrawnComboBox()
    {
        m_dclickProcessor = (wxPGDoubleClickProcessor*) NULL;
        m_sizeEventCalled = false;
    }

    ~wxPGComboBox()
    {
        if ( m_dclickProcessor )
        {
            RemoveEventHandler(m_dclickProcessor);
            delete m_dclickProcessor;
        }
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayString& choices,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxT("wxOwnerDrawnComboBox"))
    {
        if ( !wxPGOwnerDrawnComboBox::Create( parent,
                                              id,
                                              value,
                                              pos,
                                              size,
                                              choices,
                                              style,
                                              validator,
                                              name ) )
            return false;

        m_dclickProcessor = new wxPGDoubleClickProcessor(this);

        PushEventHandler(m_dclickProcessor);

        return true;
    }

#if wxPG_USING_WXOWNERDRAWNCOMBOBOX
    virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const
#else
    virtual bool OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const
#endif
    {
        wxPropertyGrid* pg = GetGrid();
        pg->OnComboItemPaint((wxPGCustomComboControl*)this,item,dc,(wxRect&)rect,flags);
#if !wxPG_USING_WXOWNERDRAWNCOMBOBOX
        return true;
#endif    
    }
    virtual wxCoord OnMeasureItem( size_t item ) const
    {
        wxPropertyGrid* pg = GetGrid();
        wxRect rect;
        rect.x = -1;
        rect.width = 0;
        pg->OnComboItemPaint((wxPGCustomComboControl*)this,item,*((wxDC*)NULL),rect,0);
        return rect.height;
    }

    wxPropertyGrid* GetGrid() const
    {
        wxPropertyGrid* pg = wxDynamicCast(GetParent()->GetParent(),wxPropertyGrid);
        wxASSERT(pg);
        return pg;
    }

    virtual wxCoord OnMeasureItemWidth( size_t item ) const
    {
        wxPropertyGrid* pg = GetGrid();
        wxRect rect;
        rect.x = -1;
        rect.width = -1;
        pg->OnComboItemPaint((wxPGCustomComboControl*)this,item,*((wxDC*)NULL),rect,0);
        return rect.width;
    }

    virtual void PositionTextCtrl( int WXUNUSED(textCtrlXAdjust), int WXUNUSED(textCtrlYAdjust) )
    {
        wxPropertyGrid* pg = GetGrid();
        wxPGOwnerDrawnComboBox::PositionTextCtrl(
            wxPG_TEXTCTRLXADJUST - (wxPG_XBEFOREWIDGET+wxPG_CONTROL_MARGIN+1) - 1,
            pg->GetSpacingY() + 2
        );
    }

private:
    wxPGDoubleClickProcessor*   m_dclickProcessor;
    bool                        m_sizeEventCalled;
};


void wxPropertyGrid::OnComboItemPaint( wxPGCustomComboControl* pCc,
                                       int item,
                                       wxDC& dc,
                                       wxRect& rect,
                                       int flags )
{
    wxPGComboBox* pCb = (wxPGComboBox*)pCc;

    // Sanity check
    wxASSERT( IsKindOf(CLASSINFO(wxPropertyGrid)) );

    wxPGProperty* p = GetSelection();
    wxString text;

    const wxPGChoices* pChoices = &p->GetChoices();
    const wxPGCommonValue* comVal = NULL;
    int choiceCount = p->GetChoiceCount();
    int comVals = p->GetDisplayedCommonValueCount();
    int comValIndex = -1;
    if ( item >= choiceCount && comVals > 0 )
    {
        comValIndex = item - choiceCount;
        comVal = GetCommonValue(comValIndex);
        if ( !p->IsValueUnspecified() )
            text = comVal->GetLabel();
    }
    else
    {
        if ( !(flags & wxPGCC_PAINTING_CONTROL) )
        {
            text = pCb->GetString(item);
        }
        else
        {
            if ( !p->IsValueUnspecified() )
                text = p->GetValueString(0);
        }
    }

    if ( item < 0 )
        return;

#if !wxPG_USING_WXOWNERDRAWNCOMBOBOX
    // Add wxPGCC_PAINTING_SELECTED
    if ( !(flags & wxPGCC_PAINTING_CONTROL) &&
         wxDynamicCast(pCb->GetPopup()->GetControl(),wxVListBox)->GetSelection() == item )
         flags |= wxPGCC_PAINTING_SELECTED;
#endif

    wxSize cis;

    const wxBitmap* itemBitmap = NULL;

    if ( item >= 0 && pChoices && pChoices->Item(item).GetBitmap().Ok() && comValIndex == -1 )
        itemBitmap = &pChoices->Item(item).GetBitmap();

    //
    // Decide what custom image size to use
    if ( itemBitmap )
    {
        cis.x = itemBitmap->GetWidth();
        cis.y = itemBitmap->GetHeight();
    }
    else
    {
        cis = GetImageSize(p, item);
    }

    if ( rect.x < 0 )
    {
        // Default measure behaviour (no flexible, custom paint image only)
        if ( rect.width < 0 )
        {
            wxCoord x, y;
            GetTextExtent(text, &x, &y, 0, 0);
            rect.width = cis.x + wxCC_CUSTOM_IMAGE_MARGIN1 + wxCC_CUSTOM_IMAGE_MARGIN2 + 9 + x;
        }

        rect.height = cis.y + 2;
        return;
    }

    wxPGPaintData paintdata;
    paintdata.m_parent = NULL;
    paintdata.m_choiceItem = item;

    // This is by the current (1.0.0b) spec - if painting control, item is -1
    if ( (flags & wxPGCC_PAINTING_CONTROL) )
        paintdata.m_choiceItem = -1;

    if ( &dc )
        dc.SetBrush(*wxWHITE_BRUSH);

    wxPGCellRenderer* renderer = NULL;
    const wxPGCell* cell = NULL;

    if ( rect.x >= 0 )
    {
        //
        // DrawItem call

        wxPoint pt(rect.x + wxPG_CONTROL_MARGIN - wxPG_CHOICEXADJUST - 1,
                   rect.y + 1);

        int renderFlags = 0;

        if ( flags & wxPGCC_PAINTING_CONTROL )
        {
            renderFlags |= wxPGCellRenderer::Control;
        }
        else
        {
            // For consistency, always use normal font when drawing drop down
            // items
            dc.SetFont(GetFont());
        }

        if ( flags & wxPGCC_PAINTING_SELECTED )
            renderFlags |= wxPGCellRenderer::Selected;

        if ( cis.x > 0 && (p->HasFlag(wxPG_PROP_CUSTOMIMAGE) || !(flags & wxPGCC_PAINTING_CONTROL)) &&
             ( !p->m_valueBitmap || item == pCb->GetSelection() ) &&
             ( item >= 0 || (flags & wxPGCC_PAINTING_CONTROL) ) &&
             !itemBitmap
           )
        {
            pt.x += wxCC_CUSTOM_IMAGE_MARGIN1;
            wxRect r(pt.x,pt.y,cis.x,cis.y);

            if ( flags & wxPGCC_PAINTING_CONTROL )
            {
                //r.width = cis.x;
                r.height = wxPG_STD_CUST_IMAGE_HEIGHT(m_lineHeight);
            }

            paintdata.m_drawnWidth = r.width;

            dc.SetPen(m_colPropFore);
            if ( comValIndex >= 0 )
            {
                const wxPGCommonValue* cv = GetCommonValue(comValIndex);
                wxPGCellRenderer* renderer = cv->GetRenderer();
                r.width = rect.width;
                renderer->Render( dc, r, this, p, m_selColumn, comValIndex, renderFlags );
                return;
            }
            else if ( item >= 0 )
            {
                p->OnCustomPaint( dc, r, paintdata );
            }
            else
            {
                dc.DrawRectangle( r );
            }

            pt.x += paintdata.m_drawnWidth + wxCC_CUSTOM_IMAGE_MARGIN2 - 1;
        }
        else
        {
            // TODO: This aligns text so that it seems to be horizontally
            //       on the same line as property values. Not really
            //       sure if its needed, but seems to not cause any harm.
            pt.x -= 1;

            if ( (flags & wxPGCC_PAINTING_CONTROL) )
            {
                if ( p->IsValueUnspecified() )
                    cell = &m_unspecifiedAppearance;
                else if ( item < 0 )
                    item = pCb->GetSelection();
            }

            if ( p->IsValueUnspecified() && item < 0 )
            {
                cell = &m_unspecifiedAppearance;
            }

            if ( pChoices && item >= 0 && comValIndex < 0 )
            {
                cell = &pChoices->Item(item);
                renderer = wxPGGlobalVars->m_defaultRenderer;
                int imageOffset = renderer->PreDrawCell(dc, rect, *cell,
                                                        renderFlags );
                if ( imageOffset )
                    imageOffset += wxCC_CUSTOM_IMAGE_MARGIN1 +
                                   wxCC_CUSTOM_IMAGE_MARGIN2;
                pt.x += imageOffset;
            }
        }

        //
        // Draw text
        //

        pt.y += (rect.height-m_fontHeight)/2 - 1;

        pt.x += 1;

        dc.DrawText( text, pt.x + wxPG_XBEFORETEXT, pt.y );

        if ( renderer )
            renderer->PostDrawCell(dc, this, *cell, renderFlags);
    }
    else
    {
        //
        // MeasureItem call

        p->OnCustomPaint( dc, rect, paintdata );
        rect.height = paintdata.m_drawnHeight + 2;
        rect.width = cis.x + wxCC_CUSTOM_IMAGE_MARGIN1 + wxCC_CUSTOM_IMAGE_MARGIN2 + 9;
    }
}

bool wxPGChoiceEditor_SetCustomPaintWidth( wxPropertyGrid* propGrid,
                                           wxPGComboBox* cb,
                                           wxPGProperty* property,
                                           int cmnVal )
{
    // Must return true if value was not a common value
    int custPaintMargin;

    //
    // Use different custom paint margin because of better textctrl spacing
    if ( cb->HasFlag(wxCB_READONLY) )
        custPaintMargin = ODCB_CUST_PAINT_MARGIN_RO;
    else
        custPaintMargin = ODCB_CUST_PAINT_MARGIN;

    if ( property->IsValueUnspecified() )
    {
        cb->SetCustomPaintWidth( 0 );
        return true;
    }

    if ( cmnVal >= 0 )
    {
        // Yes, a common value is being selected
        property->SetCommonValue( cmnVal );
        wxSize imageSize = propGrid->GetCommonValue(cmnVal)->
                            GetRenderer()->GetImageSize(property, 1, cmnVal);
        if ( imageSize.x ) imageSize.x += custPaintMargin;
        cb->SetCustomPaintWidth( imageSize.x );
        return false;
    }
    else
    {
        wxSize imageSize = propGrid->GetImageSize(property, -1);
        if ( imageSize.x ) imageSize.x += custPaintMargin;
        cb->SetCustomPaintWidth( imageSize.x );
        return true;
    }
}

// CreateControls calls this with CB_READONLY in extraStyle
wxWindow* wxPGChoiceEditor::CreateControlsBase( wxPropertyGrid* propGrid,
                                                wxPGProperty* property,
                                                const wxPoint& pos,
                                                const wxSize& sz,
                                                long extraStyle ) const
{
    wxString        defString;

    // Since it is not possible (yet) to create a read-only combo box in
    // the same sense that wxTextCtrl is read-only, simply do not create
    // the control in this case.
    if ( property->HasFlag(wxPG_PROP_READONLY) )
        return NULL;

    // Get choices.
    int index = property->GetChoiceInfo( NULL );

    bool isUnspecified = property->IsValueUnspecified();

    if ( isUnspecified )
        index = -1;
    else
        defString = property->GetDisplayedString();

    const wxPGChoices& choices = property->GetChoices();

    wxArrayString labels = choices.GetLabels();

    wxPGComboBox* cb;

    wxPoint po(pos);
    wxSize si(sz);
    po.y += wxPG_CHOICEYADJUST;
    si.y -= (wxPG_CHOICEYADJUST*2);

    po.x += wxPG_CHOICEXADJUST;
    si.x -= wxPG_CHOICEXADJUST;
    wxWindow* ctrlParent = propGrid->GetPanel();

    int odcbFlags = extraStyle | wxNO_BORDER | wxPGCC_PROCESS_ENTER | wxPGCC_ALT_KEYS;

    if ( (property->GetFlags() & wxPG_PROP_USE_DCC) &&
         (property->IsKindOf(CLASSINFO(wxBoolProperty)) ) )
        odcbFlags |= wxPGCC_DCLICK_CYCLES;

    //
    // If common value specified, use appropriate index
    unsigned int cmnVals = property->GetDisplayedCommonValueCount();
    if ( cmnVals )
    {
        if ( !isUnspecified )
        {
            int cmnVal = property->GetCommonValue();
            if ( cmnVal >= 0 )
            {
                index = labels.size() + cmnVal;
            }
        }

        unsigned int i;
        for ( i=0; i<cmnVals; i++ )
            labels.Add(propGrid->GetCommonValueLabel(i));
    }

    cb = new wxPGComboBox();
#ifdef __WXMSW__
    cb->Hide();
#endif
    cb->Create(ctrlParent,
               wxPG_SUBID1,
               wxString(),
               po,
               si,
               labels,
               odcbFlags);

    //int extRight = propGrid->GetClientSize().x - (po.x+si.x);
    //int extRight =  - (po.x+si.x);

    cb->SetButtonPosition(si.y,0,wxRIGHT);
    //cb->SetPopupExtents( 1, extRight );
    cb->SetTextIndent(wxPG_XBEFORETEXT-1);

    wxPGChoiceEditor_SetCustomPaintWidth( propGrid, cb, property,
                                          property->GetCommonValue() );
    /*if ( property->GetFlags() & wxPG_PROP_CUSTOMIMAGE )
    {
        wxSize imageSize = propGrid->GetImageSize(property, index);
        if ( imageSize.x ) imageSize.x += ODCB_CUST_PAINT_MARGIN;
        cb->SetCustomPaintWidth( imageSize.x );
    }*/

    if ( index >= 0 && index < (int)cb->GetCount() )
    {
        cb->SetSelection( index );
        if ( defString.length() )
            cb->SetText( defString );
    }
    else if ( !(extraStyle & wxCB_READONLY) && defString.length() )
        cb->SetValue( defString );
    else
        cb->SetSelection( -1 );

#ifdef __WXMSW__
    cb->Show();
#endif

    return (wxWindow*) cb;
}


void wxPGChoiceEditor::UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const
{
    wxASSERT( ctrl );
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxASSERT( cb->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)));
    int ind = property->GetChoiceInfo( (wxPGChoiceInfo*)NULL );
    cb->SetSelection(ind);
}

wxPGWindowList wxPGChoiceEditor::CreateControls( wxPropertyGrid* propGrid, wxPGProperty* property,
        const wxPoint& pos, const wxSize& sz ) const
{
    return CreateControlsBase(propGrid,property,pos,sz,wxCB_READONLY);
}


int wxPGChoiceEditor::InsertItem( wxWindow* ctrl, const wxString& label, int index ) const
{
    wxASSERT( ctrl );
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxASSERT( cb->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)));

    if (index < 0)
        index = cb->GetCount();

    return cb->Insert(label,index);
}


void wxPGChoiceEditor::DeleteItem( wxWindow* ctrl, int index ) const
{
    wxASSERT( ctrl );
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxASSERT( cb->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)));

    cb->Delete(index);
}

bool wxPGChoiceEditor::OnEvent( wxPropertyGrid* propGrid, wxPGProperty* property,
    wxWindow* ctrl, wxEvent& event ) const
{
    if ( event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED )
    {
        wxPGComboBox* cb = (wxPGComboBox*)ctrl;
        int index = cb->GetSelection();
        int cmnValIndex = -1;
        int cmnVals = property->GetDisplayedCommonValueCount();
        int items = cb->GetCount();

        if ( index >= (items-cmnVals) )
        {
            // Yes, a common value is being selected
            cmnValIndex = index - (items-cmnVals);
            property->SetCommonValue( cmnValIndex );

            // Truly set value to unspecified?
            if ( propGrid->GetUnspecifiedCommonValue() == cmnValIndex )
            {
                if ( !property->IsValueUnspecified() )
                    propGrid->SetInternalFlag(wxPG_FL_VALUE_CHANGE_IN_EVENT);
                property->SetValueToUnspecified();
                if ( !cb->HasFlag(wxCB_READONLY) )
                    cb->GetTextCtrl()->SetValue(wxEmptyString);
                return false;
            }
        }
        return wxPGChoiceEditor_SetCustomPaintWidth( propGrid, cb, property,
                                                     cmnValIndex);
    }
    return false;
}


bool wxPGChoiceEditor::GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;

    int index = cb->GetSelection();

    if ( index != property->GetChoiceInfo( (wxPGChoiceInfo*) NULL ) ||
        // Changing unspecified always causes event (returning
        // true here should be enough to trigger it).
         property->IsValueUnspecified()
       )
    {
        return property->ActualIntToValue( variant, index, 0 );
    }
    return false;
}


void wxPGChoiceEditor::SetControlStringValue( wxPGProperty* WXUNUSED(property), wxWindow* ctrl, const wxString& txt ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxASSERT( cb );
    cb->SetValue(txt);
}


void wxPGChoiceEditor::SetControlIntValue( wxPGProperty* WXUNUSED(property), wxWindow* ctrl, int value ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxASSERT( cb );
    cb->SetSelection(value);
}


void wxPGChoiceEditor::SetValueToUnspecified( wxPGProperty* property,
                                              wxWindow* ctrl ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;

    if ( !cb->HasFlag(wxCB_READONLY) )
    {
        wxPropertyGrid* pg = property->GetGrid();
        if ( pg )
        {
            wxString tcText = pg->GetUnspecifiedValueText();
            pg->SetupTextCtrlValue(tcText);
            cb->SetValue(tcText);
        }
    }
    else
    {
        cb->SetSelection(-1);
    }
}


bool wxPGChoiceEditor::CanContainCustomImage() const
{
    return true;
}


wxPGChoiceEditor::~wxPGChoiceEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(Choice) = NULL;
}

// -----------------------------------------------------------------------
// wxPGComboBoxEditor
// -----------------------------------------------------------------------


WX_PG_IMPLEMENT_EDITOR_CLASS(ComboBox,wxPGComboBoxEditor,wxPGChoiceEditor)


void wxPGComboBoxEditor::UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    cb->SetValue(property->GetValueString(wxPG_EDITABLE_VALUE));

    // TODO: If string matches any selection, then select that.
}


wxPGWindowList wxPGComboBoxEditor::CreateControls( wxPropertyGrid* propGrid,
                                                   wxPGProperty* property,
                                                   const wxPoint& pos,
                                                   const wxSize& sz ) const
{
    return CreateControlsBase(propGrid,property,pos,sz,0);
}


bool wxPGComboBoxEditor::OnEvent( wxPropertyGrid* propGrid,
                                  wxPGProperty* property,
                                  wxWindow* ctrl,
                                  wxEvent& event ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*) NULL;
    wxWindow* textCtrl = (wxWindow*) NULL;

    if ( ctrl )
    {
        cb = (wxPGOwnerDrawnComboBox*)ctrl;
        textCtrl = cb->GetTextCtrl();
    }

    if ( wxPGTextCtrlEditor::OnTextCtrlEvent(propGrid,property,textCtrl,event) )
        return true;

    return wxPGChoiceEditor::OnEvent(propGrid,property,ctrl,event);
}


bool wxPGComboBoxEditor::GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxString textVal = cb->GetValue();

    if ( property->UsesAutoUnspecified() && !textVal.length() )
    {
        variant.MakeNull();
        return true;
    }

    bool res = property->ActualStringToValue(variant, textVal, wxPG_EDITABLE_VALUE);

    // Changing unspecified always causes event (returning
    // true here should be enough to trigger it).
    if ( !res && variant.IsNull() )
        res = true;

    return res;
}

void wxPGComboBoxEditor::OnFocus( wxPGProperty* property,
                                  wxWindow* ctrl ) const
{
    wxPGOwnerDrawnComboBox* cb = (wxPGOwnerDrawnComboBox*)ctrl;
    wxPGTextCtrlEditor_OnFocus(property, cb->GetTextCtrl());
}

wxPGComboBoxEditor::~wxPGComboBoxEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(ComboBox) = NULL;
}

// -----------------------------------------------------------------------
// wxPGChoiceAndButtonEditor
// -----------------------------------------------------------------------


// This simpler implement_editor macro doesn't define class body.
WX_PG_IMPLEMENT_EDITOR_CLASS(ChoiceAndButton,wxPGChoiceAndButtonEditor,wxPGChoiceEditor)


wxPGWindowList wxPGChoiceAndButtonEditor::CreateControls( wxPropertyGrid* propGrid,
                                                          wxPGProperty* property,
                                                          const wxPoint& pos,
                                                          const wxSize& sz ) const
{
    // Use one two units smaller to match size of the combo's dropbutton.
    // (normally a bigger button is used because it looks better)
    int bt_wid = sz.y;
    bt_wid -= 2;
    wxSize bt_sz(bt_wid,bt_wid);

    // Position of button.
    wxPoint bt_pos(pos.x+sz.x-bt_sz.x,pos.y);
#ifdef __WXMAC__
    bt_pos.y -= 1;
#else
    bt_pos.y += 1;
#endif

    wxWindow* bt = propGrid->GenerateEditorButton( bt_pos, bt_sz );

    // Size of choice.
    wxSize ch_sz(sz.x-bt->GetSize().x,sz.y);

#ifdef __WXMAC__
    ch_sz.x -= wxPG_TEXTCTRL_AND_BUTTON_SPACING;
#endif

    wxWindow* ch = wxPG_EDITOR(Choice)->CreateControls(propGrid,property,
        pos,ch_sz).m_primary;

#ifdef __WXMSW__
    bt->Show();
#endif

    return wxPGWindowList(ch, bt);
}

wxPGChoiceAndButtonEditor::~wxPGChoiceAndButtonEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(ChoiceAndButton) = NULL;
}

// -----------------------------------------------------------------------
// wxPGTextCtrlAndButtonEditor
// -----------------------------------------------------------------------


// This simpler implement_editor macro doesn't define class body.
WX_PG_IMPLEMENT_EDITOR_CLASS(TextCtrlAndButton,wxPGTextCtrlAndButtonEditor,wxPGTextCtrlEditor)


wxPGWindowList wxPGTextCtrlAndButtonEditor::CreateControls( wxPropertyGrid* propGrid,
                                                            wxPGProperty* property,
                                                            const wxPoint& pos,
                                                            const wxSize& sz ) const
{
    wxWindow* wnd2;
    wxWindow* wnd = propGrid->GenerateEditorTextCtrlAndButton( pos, sz, &wnd2,
        property->GetFlags() & wxPG_PROP_NOEDITOR, property);

    return wxPGWindowList(wnd, wnd2);
}

wxPGTextCtrlAndButtonEditor::~wxPGTextCtrlAndButtonEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(TextCtrlAndButton) = NULL;
}

// -----------------------------------------------------------------------
// wxPGCheckBoxEditor
// -----------------------------------------------------------------------

#if wxPG_INCLUDE_CHECKBOX

WX_PG_IMPLEMENT_EDITOR_CLASS(CheckBox,wxPGCheckBoxEditor,wxPGEditor)


// Check box state flags
enum
{
    wxSCB_STATE_UNCHECKED   = 0,
    wxSCB_STATE_CHECKED     = 1,
    wxSCB_STATE_BOLD        = 2,
    wxSCB_STATE_UNSPECIFIED = 4
};

const int wxSCB_SETVALUE_CYCLE = 2;


static void DrawSimpleCheckBox( wxDC& dc, const wxRect& rect, int box_hei,
                                int state )
{
    // Box rectangle.
    wxRect r(rect.x+wxPG_XBEFORETEXT,rect.y+((rect.height-box_hei)/2),
             box_hei,box_hei);
    wxColour useCol = dc.GetTextForeground();

    // Draw check mark first because it is likely to overdraw the
    // surrounding rectangle.
    if ( state & wxSCB_STATE_CHECKED )
    {
        wxRect r2(r.x+wxPG_CHECKMARK_XADJ,
                  r.y+wxPG_CHECKMARK_YADJ,
                  r.width+wxPG_CHECKMARK_WADJ,
                  r.height+wxPG_CHECKMARK_HADJ);
    #if wxPG_CHECKMARK_DEFLATE
        r2.Deflate(wxPG_CHECKMARK_DEFLATE);
    #endif
        dc.DrawCheckMark(r2);

        // This would draw a simple cross check mark.
        // dc.DrawLine(r.x,r.y,r.x+r.width-1,r.y+r.height-1);
        // dc.DrawLine(r.x,r.y+r.height-1,r.x+r.width-1,r.y);

    }

    if ( !(state & wxSCB_STATE_BOLD) )
    {
        // Pen for thin rectangle.
        dc.SetPen(useCol);
    }
    else
    {
        // Pen for bold rectangle.
        wxPen linepen(useCol,2,wxSOLID);
        linepen.SetJoin(wxJOIN_MITER); // This prevents round edges.
        dc.SetPen(linepen);
        r.x++;
        r.y++;
        r.width--;
        r.height--;
    }

    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    dc.DrawRectangle(r);
    dc.SetPen(*wxTRANSPARENT_PEN);
}

//
// Real simple custom-drawn checkbox-without-label class.
//
class wxSimpleCheckBox : public wxControl
{
public:

    void SetValue( int value );

    wxSimpleCheckBox( wxWindow* parent,
                      wxWindowID id,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize )
        : wxControl(parent,id,pos,size,wxNO_BORDER|wxWANTS_CHARS)
    {
        // Due to SetOwnFont stuff necessary for GTK+ 1.2, we need to have this
        SetFont( parent->GetFont() );

        m_state = wxSCB_STATE_UNCHECKED;
        wxPropertyGrid* pg = (wxPropertyGrid*) parent->GetParent();
        wxASSERT( pg->IsKindOf(CLASSINFO(wxPropertyGrid)) );
        m_boxHeight = pg->GetFontHeight();
        SetBackgroundStyle( wxBG_STYLE_COLOUR );
    }

    virtual ~wxSimpleCheckBox();

    virtual bool ProcessEvent(wxEvent& event);

    int m_state;
    int m_boxHeight;

    static wxBitmap* ms_doubleBuffer;

};

wxSimpleCheckBox::~wxSimpleCheckBox()
{
    delete ms_doubleBuffer;
    ms_doubleBuffer = NULL;
}


wxBitmap* wxSimpleCheckBox::ms_doubleBuffer = (wxBitmap*) NULL;

void wxSimpleCheckBox::SetValue( int value )
{
    if ( value == wxSCB_SETVALUE_CYCLE )
    {
        if ( m_state & wxSCB_STATE_CHECKED )
            m_state &= ~wxSCB_STATE_CHECKED;
        else
            m_state |= wxSCB_STATE_CHECKED;
    }
    else
    {
        m_state = value;
    }
    Refresh();

    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED,GetParent()->GetId());

    wxPropertyGrid* propGrid = (wxPropertyGrid*) GetParent()->GetParent();
    wxASSERT( propGrid->IsKindOf(CLASSINFO(wxPropertyGrid)) );
    propGrid->OnCustomEditorEvent(evt);
}


bool wxSimpleCheckBox::ProcessEvent(wxEvent& event)
{
    wxPropertyGrid* propGrid = (wxPropertyGrid*) GetParent()->GetParent();
    wxASSERT( propGrid->IsKindOf(CLASSINFO(wxPropertyGrid)) );

    if ( event.GetEventType() == wxEVT_NAVIGATION_KEY )
    {
        //wxLogDebug(wxT("wxEVT_NAVIGATION_KEY"));
        //SetFocusFromKbd();
        //event.Skip();
        //return wxControl::ProcessEvent(event);
    }
    else
    if ( ( (event.GetEventType() == wxEVT_LEFT_DOWN || event.GetEventType() == wxEVT_LEFT_DCLICK)
          && ((wxMouseEvent&)event).m_x > (wxPG_XBEFORETEXT-2)
          && ((wxMouseEvent&)event).m_x <= (wxPG_XBEFORETEXT-2+m_boxHeight) )
       )
    {
        SetValue(wxSCB_SETVALUE_CYCLE);
        return true;
    }
    else if ( event.GetEventType() == wxEVT_PAINT )
    {
        wxSize clientSize = GetClientSize();
        wxPaintDC dc(this);

        /*
        // Buffered paint DC doesn't seem to do much good
        if ( !ms_doubleBuffer ||
             clientSize.x > ms_doubleBuffer->GetWidth() ||
             clientSize.y > ms_doubleBuffer->GetHeight() )
        {
            delete ms_doubleBuffer;
            ms_doubleBuffer = new wxBitmap(clientSize.x+25,clientSize.y+25);
        }

        wxBufferedPaintDC dc(this,*ms_doubleBuffer);
        */

        wxRect rect(0,0,clientSize.x,clientSize.y);
        //rect.x -= 1;
        rect.y += 1;
        rect.width += 1;

        m_boxHeight = propGrid->GetFontHeight();

        wxColour bgcol = GetBackgroundColour();
        dc.SetBrush( bgcol );
        dc.SetPen( bgcol );
        dc.DrawRectangle( rect );

        dc.SetTextForeground(GetForegroundColour());

        int state = m_state;
        if ( !(state & wxSCB_STATE_UNSPECIFIED) &&
             GetFont().GetWeight() == wxBOLD )
            state |= wxSCB_STATE_BOLD;

        DrawSimpleCheckBox(dc, rect, m_boxHeight, state);

        return true;
    }
    else if ( event.GetEventType() == wxEVT_SIZE ||
              event.GetEventType() == wxEVT_SET_FOCUS ||
              event.GetEventType() == wxEVT_KILL_FOCUS
            )
    {
        Refresh();
    }
    else if ( event.GetEventType() == wxEVT_KEY_DOWN )
    {
        wxKeyEvent& keyEv = (wxKeyEvent&) event;

        if ( keyEv.GetKeyCode() == WXK_TAB )
        {
            propGrid->SendNavigationKeyEvent( keyEv.ShiftDown()?0:1 );
            return true;
        }
        else
        if ( keyEv.GetKeyCode() == WXK_SPACE )
        {
            SetValue(wxSCB_SETVALUE_CYCLE);
            return true;
        }
    }
    return wxControl::ProcessEvent(event);
}


wxPGWindowList wxPGCheckBoxEditor::CreateControls( wxPropertyGrid* propGrid,
                                                   wxPGProperty* property,
                                                   const wxPoint& pos,
                                                   const wxSize& size ) const
{
    if ( property->HasFlag(wxPG_PROP_READONLY) )
        return NULL;

    wxPoint pt = pos;
    pt.x -= wxPG_XBEFOREWIDGET;
    wxSize sz = size;
    sz.x = propGrid->GetFontHeight() + (wxPG_XBEFOREWIDGET*2) + 4;

    wxSimpleCheckBox* cb = new wxSimpleCheckBox(propGrid->GetPanel(),wxPG_SUBID1,pt,sz);

    cb->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    cb->Connect( wxPG_SUBID1, wxEVT_LEFT_DOWN,
            (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
            &wxPropertyGrid::OnCustomEditorEvent, NULL, propGrid );

    cb->Connect( wxPG_SUBID1, wxEVT_LEFT_DCLICK,
            (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
            &wxPropertyGrid::OnCustomEditorEvent, NULL, propGrid );

    if ( property->IsValueUnspecified() )
    {
        cb->m_state = wxSCB_STATE_UNSPECIFIED;
    }
    else
    {
        if ( property->GetChoiceInfo((wxPGChoiceInfo*)NULL) )
            cb->m_state = wxSCB_STATE_CHECKED;

        // If mouse cursor was on the item, toggle the value now.
        if ( propGrid->GetInternalFlags() & wxPG_FL_ACTIVATION_BY_CLICK )
        {
            wxPoint pt = cb->ScreenToClient(::wxGetMousePosition());
            if ( pt.x <= (wxPG_XBEFORETEXT-2+cb->m_boxHeight) )
            {
                if ( cb->m_state & wxSCB_STATE_CHECKED )
                    cb->m_state &= ~wxSCB_STATE_CHECKED;
                else
                    cb->m_state |= wxSCB_STATE_CHECKED;

                // Makes sure wxPG_EVT_CHANGING etc. is sent for this initial
                // click 
                propGrid->ChangePropertyValue(property,
                                              wxPGVariant_Bool(cb->m_state));
            }
        }
    }

    propGrid->SetInternalFlag( wxPG_FL_FIXED_WIDTH_EDITOR );

    return cb;
}

void wxPGCheckBoxEditor::DrawValue( wxDC& dc, const wxRect& rect,
                                    wxPGProperty* property,
                                    const wxString& WXUNUSED(text) ) const
{
    int state = wxSCB_STATE_UNCHECKED;

    if ( !property->IsValueUnspecified() )
    {
        state = property->GetChoiceInfo((wxPGChoiceInfo*)NULL);
        if ( dc.GetFont().GetWeight() == wxBOLD )
            state |= wxSCB_STATE_BOLD;
    }
    else
    {
        state |= wxSCB_STATE_UNSPECIFIED;
    }

    DrawSimpleCheckBox(dc, rect, dc.GetCharHeight(), state);
}

void wxPGCheckBoxEditor::UpdateControl( wxPGProperty* property,
                                        wxWindow* ctrl ) const
{
    wxSimpleCheckBox* cb = (wxSimpleCheckBox*) ctrl;
    wxASSERT( cb );

    if ( !property->IsValueUnspecified() )
        cb->m_state = property->GetChoiceInfo((wxPGChoiceInfo*)NULL);
    else
        cb->m_state = wxSCB_STATE_UNSPECIFIED;

    cb->Refresh();
}

bool wxPGCheckBoxEditor::OnEvent( wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property),
    wxWindow* WXUNUSED(ctrl), wxEvent& event ) const
{
    if ( event.GetEventType() == wxEVT_COMMAND_CHECKBOX_CLICKED )
    {
        return true;
    }
    return false;
}


bool wxPGCheckBoxEditor::GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl ) const
{
    wxSimpleCheckBox* cb = (wxSimpleCheckBox*)ctrl;

    int index = cb->m_state;

    if ( index != property->GetChoiceInfo( (wxPGChoiceInfo*) NULL ) ||
         // Changing unspecified always causes event (returning
         // true here should be enough to trigger it).
         property->IsValueUnspecified()
       )
    {
        return property->ActualIntToValue(variant, index, 0);
    }
    return false;
}


void wxPGCheckBoxEditor::SetControlIntValue( wxPGProperty* WXUNUSED(property), wxWindow* ctrl, int value ) const
{
    if ( value != 0 ) value = 1;
    ((wxSimpleCheckBox*)ctrl)->m_state = value;
    ctrl->Refresh();
}


void wxPGCheckBoxEditor::SetValueToUnspecified( wxPGProperty* WXUNUSED(property), wxWindow* ctrl ) const
{
    ((wxSimpleCheckBox*)ctrl)->m_state = wxSCB_STATE_UNSPECIFIED;
    ctrl->Refresh();
}

wxPGCheckBoxEditor::~wxPGCheckBoxEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(CheckBox) = NULL;
}

#endif // wxPG_INCLUDE_CHECKBOX

// -----------------------------------------------------------------------

wxWindow* wxPropertyGrid::GetEditorControl() const
{
    wxWindow* ctrl = m_wndEditor;

    if ( !ctrl )
        return ctrl;

    // If it's clipper window, return its child instead
#if wxPG_ENABLE_CLIPPER_WINDOW
    if ( ctrl->IsKindOf(CLASSINFO(wxPGClipperWindow)) )
    {
        return ((wxPGClipperWindow*)ctrl)->GetControl();
    }
#endif

    return ctrl;
}

// -----------------------------------------------------------------------

wxTextCtrl* wxPropertyGrid::GetLabelEditor() const
{
    wxWindow* tcWnd = m_labelEditor;

    if ( !tcWnd )
        return NULL;

#if wxPG_ENABLE_CLIPPER_WINDOW
    if ( tcWnd->IsKindOf(CLASSINFO(wxPGClipperWindow)) )
    {
        tcWnd = ((wxPGClipperWindow*)tcWnd)->GetControl();
    }
#endif

    return wxStaticCast(tcWnd, wxTextCtrl);
}

// -----------------------------------------------------------------------

void wxPropertyGrid::CorrectEditorWidgetSizeX()
{
    int secWid = 0;

    // Use fixed selColumn 1 for main editor widgets
    int newSplitterx = m_pState->DoGetSplitterPosition(0);
    int newWidth = newSplitterx + m_pState->m_colWidths[1];

    if ( m_wndEditor2 )
    {
        // if width change occurred, move secondary wnd by that amount
        wxRect r = m_wndEditor2->GetRect();
        secWid = r.width;
        r.x = newWidth - secWid;

        m_wndEditor2->SetSize( r );

        // if primary is textctrl, then we have to add some extra space
#ifdef __WXMAC__
        if ( m_wndEditor )
#else
        if ( m_wndEditor && m_wndEditor->IsKindOf(CLASSINFO(wxTextCtrl)) )
#endif
            secWid += wxPG_TEXTCTRL_AND_BUTTON_SPACING;
    }

    if ( m_wndEditor )
    {
        wxRect r = m_wndEditor->GetRect();

        r.x = newSplitterx+m_ctrlXAdjust;

        if ( !(m_iFlags & wxPG_FL_FIXED_WIDTH_EDITOR) )
            r.width = newWidth - r.x - secWid;

        m_wndEditor->SetSize(r);
    }

    if ( m_wndEditor2 )
        m_wndEditor2->Refresh();
}

// -----------------------------------------------------------------------

void wxPropertyGrid::CorrectEditorWidgetPosY()
{
    wxPGProperty* selected = GetSelection();

    if ( selected )
    {
        if ( m_labelEditor )
        {
            wxRect r = GetEditorWidgetRect(selected, m_selColumn);
            wxPoint pos = m_labelEditor->GetPosition();

            // Calculate y offset
            int offset = pos.y % m_lineHeight;

            m_labelEditor->Move(pos.x, r.y + offset);
        }

        if ( m_wndEditor || m_wndEditor2 ) 
        {
            wxRect r = GetEditorWidgetRect(selected, 1);

            if ( m_wndEditor )
            {
                wxPoint pos = m_wndEditor->GetPosition();

                // Calculate y offset
                int offset = pos.y % m_lineHeight;

                m_wndEditor->Move(pos.x, r.y + offset);
            }

            if ( m_wndEditor2 )
            {
                wxPoint pos = m_wndEditor2->GetPosition();

                m_wndEditor2->Move(pos.x, r.y);
            }
        }
    }
}

// -----------------------------------------------------------------------

bool wxPropertyGrid::AdjustPosForClipperWindow( wxWindow* topCtrlWnd, int* x, int* y )
{
#if wxPG_ENABLE_CLIPPER_WINDOW
    // Take clipper window into account
    if (topCtrlWnd->GetPosition().x < 1 &&
        !topCtrlWnd->IsKindOf(CLASSINFO(wxPGClipperWindow)))
    {
        topCtrlWnd = topCtrlWnd->GetParent();
        wxASSERT( topCtrlWnd->IsKindOf(CLASSINFO(wxPGClipperWindow)) );
        *x -= ((wxPGClipperWindow*)topCtrlWnd)->GetXClip();
        *y -= ((wxPGClipperWindow*)topCtrlWnd)->GetYClip();
        return true;
    }
#else
    wxUnusedVar(topCtrlWnd);
    wxUnusedVar(x);
    wxUnusedVar(y);
#endif
    return false;
}

// -----------------------------------------------------------------------

// Fixes position of wxTextCtrl-like control (wxSpinCtrl usually
// fits into that category as well).
void wxPropertyGrid::FixPosForTextCtrl( wxWindow* ctrl,
                                        unsigned int forColumn,
                                        const wxPoint& offset )
{
    // Center the control vertically
    wxRect finalPos = ctrl->GetRect();
    int y_adj = (m_lineHeight - finalPos.height)/2 + wxPG_TEXTCTRLYADJUST;

    // Prevent over-sized control
    int sz_dec = (y_adj + finalPos.height) - m_lineHeight;
    if ( sz_dec < 0 ) sz_dec = 0;

    finalPos.y += y_adj;
    finalPos.height -= (y_adj+sz_dec);

    int textCtrlXAdjust = wxPG_TEXTCTRLXADJUST;

    if ( forColumn != 1 )
        textCtrlXAdjust -= 3;  // magic number!

    finalPos.x += textCtrlXAdjust;
    finalPos.width -= textCtrlXAdjust;

    finalPos.x += offset.x;
    finalPos.y += offset.y;

    ctrl->SetSize(finalPos);
}

// -----------------------------------------------------------------------

wxWindow* wxPropertyGrid::GenerateEditorTextCtrl( const wxPoint& pos,
                                                  const wxSize& sz,
                                                  const wxString& value,
                                                  wxWindow* secondary,
                                                  int extraStyle,
                                                  int maxLen,
                                                  unsigned int forColumn )
{
    wxPGProperty* selected = GetSelection();
    wxASSERT(selected);

    int tcFlags = wxTE_PROCESS_ENTER | extraStyle;

    if ( selected->HasFlag(wxPG_PROP_READONLY) && forColumn == 1 )
        tcFlags |= wxTE_READONLY;

    wxPoint p(pos.x,pos.y);
    wxSize s(sz.x,sz.y);

   // Need to reduce width of text control on Mac
#if defined(__WXMAC__)
   s.x -= 8;
#endif

    // For label editors, trim the size to allow better splitter grabbing
    if ( forColumn != 1 )
        s.x -= 2;

     // Take button into acccount
    if ( secondary )
    {
        s.x -= (secondary->GetSize().x + wxPG_TEXTCTRL_AND_BUTTON_SPACING);
        m_iFlags &= ~(wxPG_FL_PRIMARY_FILLS_ENTIRE);
    }

    // If the height is significantly higher, then use border, and fill the rect exactly.
    bool hasSpecialSize = false;

    if ( (sz.y - m_lineHeight) > 5 )
        hasSpecialSize = true;

#if wxPG_NAT_TEXTCTRL_BORDER_ANY

    // Create clipper window
    wxPGClipperWindow* wnd = new wxPGClipperWindow();
#if defined(__WXMSW__)
    wnd->Hide();
#endif
    wnd->Create(GetPanel(),wxPG_SUBID1,p,s);

    // This generates rect of the control inside the clipper window
    if ( !hasSpecialSize )
        wnd->GetControlRect(wxPG_NAT_TEXTCTRL_BORDER_X, wxPG_NAT_TEXTCTRL_BORDER_Y, p, s);
    else
        wnd->GetControlRect(0, 0, p, s);

    wxWindow* ctrlParent = wnd;

#else

    wxWindow* ctrlParent = GetPanel();

    if ( !hasSpecialSize )
        tcFlags |= wxNO_BORDER;

#endif

    wxTextCtrl* tc = new wxTextCtrl();

#if defined(__WXMSW__) && !wxPG_NAT_TEXTCTRL_BORDER_ANY
    tc->Hide();
#endif
    SetupTextCtrlValue(value);
    tc->Create(ctrlParent,wxPG_SUBID1,value, p, s,tcFlags);

#if wxPG_NAT_TEXTCTRL_BORDER_ANY
    wxWindow* ed = wnd;
    wnd->SetControl(tc);
#else
    wxWindow* ed = tc;
#endif

    // Center the control vertically
    if ( !hasSpecialSize )
        FixPosForTextCtrl(ed, forColumn);

    if ( forColumn != 1 )
    {
        if ( tc != ed )
        {
            ed->SetBackgroundColour(m_colSelBack);
            ed->SetForegroundColour(m_colSelFore);
        }
        tc->SetBackgroundColour(m_colSelBack);
        tc->SetForegroundColour(m_colSelFore);
    }

#ifdef __WXMSW__
    ed->Show();
    if ( secondary )
        secondary->Show();
#endif

    // Set maximum length
    if ( maxLen > 0 )
        tc->SetMaxLength( maxLen );

    return (wxWindow*) ed;
}

// -----------------------------------------------------------------------

wxWindow* wxPropertyGrid::GenerateEditorButton( const wxPoint& pos, const wxSize& sz )
{
    wxPGProperty* selected = GetSelection();
    wxASSERT(selected);

#ifdef __WXMAC__
   // Decorations are chunky on Mac, and we can't make the button square, so
   // do things a bit differently on this platform.

   wxPoint p(pos.x+sz.x,
             pos.y+wxPG_BUTTON_SIZEDEC-wxPG_NAT_BUTTON_BORDER_Y);
   wxSize s(25, -1);

   wxButton* but = new wxButton();
   but->Create(GetPanel(),wxPG_SUBID2,wxT("..."),p,s,wxWANTS_CHARS);

   // Now that we know the size, move to the correct position
   p.x = pos.x + sz.x - but->GetSize().x - 2;
   but->Move(p);

#else 
    wxSize s(sz.y-(wxPG_BUTTON_SIZEDEC*2)+(wxPG_NAT_BUTTON_BORDER_Y*2),
        sz.y-(wxPG_BUTTON_SIZEDEC*2)+(wxPG_NAT_BUTTON_BORDER_Y*2));

    // Reduce button width to lineheight
    if ( s.x > m_lineHeight )
        s.x = m_lineHeight;

#ifdef __WXGTK__
    // On wxGTK, take fixed button margins into account
    if ( s.x < 25 )
        s.x = 25;
#endif

    wxPoint p(pos.x+sz.x-s.x,
        pos.y+wxPG_BUTTON_SIZEDEC-wxPG_NAT_BUTTON_BORDER_Y);

    wxButton* but = new wxButton();
  #ifdef __WXMSW__
    but->Hide();
  #endif
    but->Create(GetPanel(),wxPG_SUBID2,wxT("..."),p,s,wxWANTS_CHARS);

  #ifdef __WXGTK__
    wxFont font = GetFont();
    font.SetPointSize(font.GetPointSize()-2);
    but->SetFont(font);
  #else
    but->SetFont(GetFont());
  #endif
#endif

    if ( selected->HasFlag(wxPG_PROP_READONLY) )
        but->Disable();

    return but;
}

// -----------------------------------------------------------------------

wxWindow* wxPropertyGrid::GenerateEditorTextCtrlAndButton( const wxPoint& pos,
                                                           const wxSize& sz,
                                                           wxWindow** psecondary,
                                                           int limitedEditing,
                                                           wxPGProperty* property )
{
    wxButton* but = (wxButton*)GenerateEditorButton(pos,sz);
    *psecondary = (wxWindow*)but;

    if ( limitedEditing )
    {
    #ifdef __WXMSW__
        // There is button Show in GenerateEditorTextCtrl as well
        but->Show();
    #endif
        return (wxWindow*) NULL;
    }

    wxString text;

    if ( !property->IsValueUnspecified() )
        text = property->GetValueString(property->HasFlag(wxPG_PROP_READONLY)?0:wxPG_EDITABLE_VALUE);

    return GenerateEditorTextCtrl(pos,sz,text,but,property->m_maxLen);
}

// -----------------------------------------------------------------------

void wxPropertyGrid::SetEditorAppearance( const wxPGCell& cell )
{
    wxWindow* editor = GetEditorControl();
    if ( !editor )
        return;

    // Get old editor appearance
    const wxPGCell& oCell = m_editorAppearance;
    wxPGProperty* property = GetSelection();

    wxTextCtrl* tc = GetEditorTextCtrl();

    wxPGComboBox* cb;
    if ( editor->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)) )
        cb = (wxPGComboBox*) editor;
    else
        cb = NULL;

    if ( tc || cb )
    {
        wxString tcText;
        bool changeText = false;

        if ( cell.HasText() && !IsEditorFocused() )
        {
            tcText = cell.GetText();
            changeText = true;
        }
        else if ( oCell.HasText() )
        {
            tcText = GetSelection()->GetValueString(
                property->HasFlag(wxPG_PROP_READONLY)?0:wxPG_EDITABLE_VALUE);
            changeText = true;
        }

        if ( changeText )
        {
            if ( tc )
            {
                // The next line prevents spurious EVT_TEXT from being
                // received.
                SetupTextCtrlValue(tcText);
                tc->SetValue(tcText);
            }
            else
            {
                cb->SetText(tcText);
            }
        }
    }

    // We used to obtain wxVisualAttributes via
    // editor->GetDefaultAttributes() here, but that is not
    // very consistently implemented in wx2.8, so it is safer
    // to just use colours from wxSystemSettings etc. 

    const wxColour& fgCol = cell.GetFgCol();
    if ( wxGDI_IS_OK(fgCol) )
    {
        editor->SetForegroundColour(fgCol);

        // Set for wxTextCtrl separately to work around bug in wx2.8
        // that may not be fixable due to ABI compatibility issues.
        if ( tc && tc != editor )
            tc->SetForegroundColour(fgCol);
    }
    else if ( wxGDI_IS_OK(oCell.GetFgCol()) )
    {
        wxColour vColFg =
            wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
        editor->SetForegroundColour(vColFg);
        if ( tc && tc != editor )
            tc->SetForegroundColour(vColFg);
    }

    const wxColour& bgCol = cell.GetBgCol();
    if ( wxGDI_IS_OK(bgCol) )
    {
        editor->SetBackgroundColour(bgCol);
        if ( tc && tc != editor )
            tc->SetBackgroundColour(bgCol);
    }
    else if ( wxGDI_IS_OK(oCell.GetBgCol()) )
    {
        wxColour vColBg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
        editor->SetBackgroundColour(vColBg);
        if ( tc && tc != editor )
            tc->SetBackgroundColour(vColBg);
    }

    const wxFont& font = cell.GetFont();
    if ( wxGDI_IS_OK(font) )
    {
        editor->SetFont(font);
        if ( tc && tc != editor )
            tc->SetFont(font);
    }
    else if ( wxGDI_IS_OK(oCell.GetFont()) )
    {
        wxFont vFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
        editor->SetFont(vFont);
        if ( tc && tc != editor )
            tc->SetFont(vFont);
    }

    m_editorAppearance.Assign(cell);
}

// -----------------------------------------------------------------------

wxTextCtrl* wxPropertyGrid::GetEditorTextCtrl() const
{
    wxWindow* wnd = GetEditorControl();

    if ( !wnd )
        return NULL;

    if ( wnd->IsKindOf(CLASSINFO(wxTextCtrl)) )
        return wxStaticCast(wnd, wxTextCtrl);

    if ( wnd->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)) )
    {
        wxPGOwnerDrawnComboBox* cb = wxStaticCast(wnd, wxPGOwnerDrawnComboBox);
        return cb->GetTextCtrl();
    }

    return NULL;
}

// -----------------------------------------------------------------------

#if defined(__WXMSW__) && !defined(__WXWINCE__)

bool wxPG_TextCtrl_SetMargins(wxWindow* tc, const wxPoint& margins)
{
    ::SendMessage(GetHwndOf(tc),
                  EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                  MAKELONG(margins.x, margins.x));
    return true;
}

/*#elif defined(__WXGTK20__)

//
// NOTE: For this to work we need to somehow include gtk devel
//       in the bake/makefile.
//

#include <gtk/gtk.h>

bool wxPG_TextCtrl_SetMargins(wxWindow* tc, const wxPoint& margins)
{
    //
    // NB: This code has been ported from wx2.9 SVN trunk
    //

  #if GTK_CHECK_VERSION(2,10,0)
    GtkEntry *entry = NULL;

    entry = GTK_ENTRY( m_widget );

    if ( !entry )
        return false;

    const GtkBorder* oldBorder = gtk_entry_get_inner_border(entry);
    GtkBorder* newBorder;

    if ( oldBorder )
    {
        newBorder = gtk_border_copy(oldBorder);
    }
    else
    {
    #if GTK_CHECK_VERSION(2,14,0)
        newBorder = gtk_border_new();
    #else
        newBorder = g_slice_new0(GtkBorder);
    #endif
        // Use some reasonable defaults for initial margins
        newBorder->left = 2;
        newBorder->right = 2;

        // These numbers seem to let the text remain vertically centered
        // in common use scenarios when margins.y == -1.
        newBorder->top = 3;
        newBorder->bottom = 3;
    }

    if ( margins.x != -1 )
        newBorder->left = (gint) margins.x;

    if ( margins.y != -1 )
        newBorder->top = (gint) margins.y;

    gtk_entry_set_inner_border(entry, newBorder);

  #if GTK_CHECK_VERSION(2,14,0)
    gtk_border_free(newBorder);
  #else
    g_slice_free(GtkBorder, newBorder);
  #endif

    return true;
  #else
    wxUnusedVar(tc);
    wxUnusedVar(margins);
    return false;
  #endif
}
*/
#else

bool wxPG_TextCtrl_SetMargins(wxWindow* tc, const wxPoint& margins)
{
    wxUnusedVar(tc);
    wxUnusedVar(margins);
    return false;
}

#endif

// -----------------------------------------------------------------------
// wxPGEditorDialogAdapter
// -----------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxPGEditorDialogAdapter, wxObject)

bool wxPGEditorDialogAdapter::ShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property )
{
    if ( !propGrid->EditorValidate() )
        return false;

    bool res = DoShowDialog( propGrid, property );

    if ( res )
    {
        propGrid->ValueChangeInEvent( m_value );
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------
// wxPGMultiButton
// -----------------------------------------------------------------------

wxPGMultiButton::wxPGMultiButton( wxPropertyGrid* pg, const wxSize& sz )
    : wxWindow( pg->GetPanel(), wxPG_SUBID2, wxPoint(-100,-100), wxSize(0, sz.y) ),
      m_fullEditorSize(sz), m_buttonsWidth(0)
{
    SetBackgroundColour(pg->GetCellBackgroundColour());
}

int wxPGMultiButton::GenId( int id ) const
{
    if ( id < -1 )
    {
        if ( m_buttons.size() )
            id = GetButton(m_buttons.size()-1)->GetId() + 1;
        else
            id = wxPG_SUBID2;
    }
    return id;
}

#if wxUSE_BMPBUTTON
void wxPGMultiButton::Add( const wxBitmap& bitmap, int id )
{
    id = GenId(id);
    wxSize sz = GetSize();
    wxButton* button = new wxBitmapButton( this, id, bitmap, wxPoint(sz.x, 0), wxSize(sz.y, sz.y) );
    m_buttons.push_back(button);
    int bw = button->GetSize().x;
    SetSize(wxSize(sz.x+bw,sz.y));
    m_buttonsWidth += bw;
}
#endif

void wxPGMultiButton::Add( const wxString& label, int id )
{
    id = GenId(id);
    wxSize sz = GetSize();
    wxButton* button = new wxButton( this, id, label, wxPoint(sz.x, 0), wxSize(sz.y, sz.y) );
    m_buttons.push_back(button);
    int bw = button->GetSize().x;
    SetSize(wxSize(sz.x+bw,sz.y));
    m_buttonsWidth += bw;
}

// -----------------------------------------------------------------------
