/////////////////////////////////////////////////////////////////////////////
// Name:        advprops.cpp
// Purpose:     wxPropertyGrid Advanced Properties (font, colour, etc.)
// Author:      Jaakko Salli
// Modified by:
// Created:     Sep-25-2004
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
    #include "wx/button.h"
    #include "wx/pen.h"
    #include "wx/brush.h"
    #include "wx/cursor.h"
    #include "wx/dialog.h"
    #include "wx/settings.h"
    #include "wx/msgdlg.h"
    #include "wx/choice.h"
    #include "wx/stattext.h"
    #include "wx/textctrl.h"
    #include "wx/scrolwin.h"
    #include "wx/dirdlg.h"
    #include "wx/combobox.h"
    #include "wx/layout.h"
    #include "wx/sizer.h"
    #include "wx/textdlg.h"
    #include "wx/filedlg.h"
    #include "wx/intl.h"
#endif

#define __wxPG_SOURCE_FILE__

#include <wx/propgrid/propgrid.h>

#if wxPG_INCLUDE_ADVPROPS

#include <wx/propgrid/propdev.h>

#include <wx/propgrid/advprops.h>

#ifdef __WXMSW__
    #include <wx/msw/private.h>
  #if wxCHECK_VERSION(2,9,0)
    #include <wx/msw/dc.h>
  #endif
#endif

#if wxPG_USING_WXOWNERDRAWNCOMBOBOX
    #include <wx/odcombo.h>
#else
    #include <wx/propgrid/odcombo.h>
#endif

// -----------------------------------------------------------------------

#if defined(__WXMSW__)
    #define wxPG_CAN_DRAW_CURSOR           1
#elif defined(__WXGTK__)
    #define wxPG_CAN_DRAW_CURSOR           0
#elif defined(__WXMAC__)
    #define wxPG_CAN_DRAW_CURSOR           0
#else
    #define wxPG_CAN_DRAW_CURSOR           0
#endif


// -----------------------------------------------------------------------
// Value type related
// -----------------------------------------------------------------------


// Implement dynamic class for type value.
IMPLEMENT_DYNAMIC_CLASS(wxColourPropertyValue, wxObject)

bool operator == (const wxColourPropertyValue& a, const wxColourPropertyValue& b)
{
    return ( ( a.m_colour == b.m_colour ) && (a.m_type == b.m_type) );
}

bool operator == (const wxArrayInt& array1, const wxArrayInt& array2)
{
    if ( array1.GetCount() != array2.GetCount() )
        return false;
    size_t i;
    for ( i=0; i<array1.GetCount(); i++ )
    {
        if ( array1[i] != array2[i] )
            return false;
    }
    return true;
}

// -----------------------------------------------------------------------
// wxSpinCtrl-based property editor
// -----------------------------------------------------------------------

#if wxUSE_SPINBTN

#ifdef __WXMSW__
  #define IS_MOTION_SPIN_SUPPORTED  1
#else
  #define IS_MOTION_SPIN_SUPPORTED  0
#endif

#if IS_MOTION_SPIN_SUPPORTED
//
// This class implements ability to rapidly change "spin" value
// by moving mouse when one of the spin buttons is depressed.
class wxPGSpinButton : public wxSpinButton
{
public:
    wxPGSpinButton() : wxSpinButton()
    {
        m_bLeftDown = false;
        m_hasCapture = false;
        m_spins = 1;

        Connect( wxEVT_LEFT_DOWN,
                 (wxObjectEventFunction) (wxEventFunction)
                 (wxMouseEventFunction)&wxPGSpinButton::OnMouseEvent );
        Connect( wxEVT_LEFT_UP,
                 (wxObjectEventFunction) (wxEventFunction)
                 (wxMouseEventFunction)&wxPGSpinButton::OnMouseEvent );
        Connect( wxEVT_MOTION,
                 (wxObjectEventFunction) (wxEventFunction)
                 (wxMouseEventFunction)&wxPGSpinButton::OnMouseEvent );
#if wxCHECK_VERSION(2,8,0)
        Connect( wxEVT_MOUSE_CAPTURE_LOST,
                 (wxObjectEventFunction) (wxEventFunction)
                 (wxMouseCaptureLostEventFunction)&wxPGSpinButton::OnMouseCaptureLost );
#endif
    }


    int GetSpins() const
    {
        return m_spins;
    }

private:
    wxPoint m_ptPosition;

    // Having a separate spins variable allows us to handle validation etc. for
    // multiple spin events at once (with quick mouse movements there could be
    // hundreds of 'spins' being done at once). Technically things like this
    // should be stored in event (wxSpinEvent in this case), but there probably
    // isn't anything there that can be reliably reused.
    int     m_spins;

    bool    m_bLeftDown;

    // SpinButton seems to be a special for mouse capture, so we may need track
    // privately whether mouse is actually captured.
    bool    m_hasCapture;

    void Capture()
    {
        if ( !m_hasCapture )
        {
            CaptureMouse();
            m_hasCapture = true;
        }

        SetCursor(wxCURSOR_SIZENS);
    }
    void Release()
    {
        m_bLeftDown = false;

        if ( m_hasCapture )
        {
            ReleaseMouse();
            m_hasCapture = false;
        }

        wxWindow *parent = GetParent();
        if ( parent )
            SetCursor(parent->GetCursor());
        else
            SetCursor(wxNullCursor);
    }

    void OnMouseEvent(wxMouseEvent& event)
    {
        if ( event.GetEventType() == wxEVT_LEFT_DOWN )
        {
            m_bLeftDown = true;
            m_ptPosition = event.GetPosition();
        }
        else if ( event.GetEventType() == wxEVT_LEFT_UP )
        {
            Release();
            m_bLeftDown = false;
        }
        else if ( event.GetEventType() == wxEVT_MOTION )
        {
            if ( m_bLeftDown )
            {
                int dy = m_ptPosition.y - event.GetPosition().y;
                if ( dy )
                {
                    Capture();
                    m_ptPosition = event.GetPosition();

                    wxSpinEvent evtscroll( (dy >= 0) ? wxEVT_SCROLL_LINEUP :
                                                       wxEVT_SCROLL_LINEDOWN,
                                           GetId() );
                    evtscroll.SetEventObject(this);

                    wxASSERT( m_spins == 1 );

                    m_spins = abs(dy);
                    GetEventHandler()->ProcessEvent(evtscroll);
                    m_spins = 1;
                }
            }
        }

        event.Skip();
    }
#if wxCHECK_VERSION(2,8,0)
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
    {
        Release();
    }
#endif
};

#endif // IS_MOTION_SPIN_SUPPORTED


// This macro also defines global wxPGEditor_SpinCtrl for storing
// the singleton class instance.
WX_PG_IMPLEMENT_EDITOR_CLASS(SpinCtrl,wxPGSpinCtrlEditor,wxPGEditor)


// Trivial destructor.
wxPGSpinCtrlEditor::~wxPGSpinCtrlEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(SpinCtrl) = NULL;
}

// Create controls and initialize event handling.
wxPGWindowList wxPGSpinCtrlEditor::CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property,
                                                   const wxPoint& pos, const wxSize& sz ) const
{
    const int margin = 1;
    wxSize butSz(18, sz.y);
    wxSize tcSz(sz.x - butSz.x - margin, sz.y);
    wxPoint butPos(pos.x + tcSz.x + margin, pos.y);

    wxSpinButton* wnd2;

#if IS_MOTION_SPIN_SUPPORTED
    if ( property->GetAttributeAsLong(wxT("MotionSpin"), 0) )
    {
        wnd2 = new wxPGSpinButton();
    }
    else
#endif
    {
        wnd2 = new wxSpinButton();
    }

#ifdef __WXMSW__
    wnd2->Hide();
#endif
    wnd2->Create( propgrid->GetPanel(), wxPG_SUBID2, butPos, butSz, wxSP_VERTICAL );

    wnd2->SetRange( INT_MIN, INT_MAX );
    wnd2->SetValue( 0 );

    propgrid->Connect( wxPG_SUBID2, wxEVT_SCROLL_LINEUP,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent, NULL, propgrid );
    propgrid->Connect( wxPG_SUBID2, wxEVT_SCROLL_LINEDOWN,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent, NULL, propgrid );
    propgrid->Connect( wxPG_SUBID1, wxEVT_KEY_DOWN,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent, NULL, propgrid );

    // Let's add validator to make sure only numbers can be entered
    wxTextValidator validator(wxFILTER_NUMERIC, &m_tempString);

    wxTextCtrl* wnd1 = (wxTextCtrl*) wxPGTextCtrlEditor::CreateControls( propgrid, property, pos, tcSz ).m_primary;
    wnd1->SetValidator(validator);

    return wxPGWindowList(wnd1, wnd2);
}

// Control's events are redirected here
bool wxPGSpinCtrlEditor::OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
                                  wxWindow* wnd, wxEvent& event ) const
{
    int evtType = event.GetEventType();
    int keycode = -1;
    int spins = 1;
    bool bigStep = false;

    if ( evtType == wxEVT_KEY_DOWN )
    {
        wxKeyEvent& keyEvent = (wxKeyEvent&)event;
        keycode = keyEvent.GetKeyCode();

        if ( keycode == WXK_UP )
            evtType = wxEVT_SCROLL_LINEUP;
        else if ( keycode == WXK_DOWN )
            evtType = wxEVT_SCROLL_LINEDOWN;
        else if ( keycode == WXK_PAGEUP )
        {
            evtType = wxEVT_SCROLL_LINEUP;
            bigStep = true;
        }
        else if ( keycode == WXK_PAGEDOWN )
        {
            evtType = wxEVT_SCROLL_LINEDOWN;
            bigStep = true;
        }
    }

    if ( evtType == wxEVT_SCROLL_LINEUP || evtType == wxEVT_SCROLL_LINEDOWN )
    {
    #if IS_MOTION_SPIN_SUPPORTED
        if ( property->GetAttributeAsLong(wxT("MotionSpin"), 0) )
        {
            wxPGSpinButton* spinButton =
                (wxPGSpinButton*) propgrid->GetEditorControlSecondary();

            if ( spinButton )
                spins = spinButton->GetSpins();
        }
    #endif

        wxString s;
        // Can't use wnd since it might be clipper window
        wxTextCtrl* tc = wxDynamicCast(propgrid->GetEditorControl(), wxTextCtrl);

        if ( tc )
            s = tc->GetValue();
        else
            s = property->GetValueAsString(wxPG_FULL_VALUE);

        int mode = wxPG_PROPERTY_VALIDATION_SATURATE;

        if ( property->GetAttributeAsLong(wxT("Wrap"), 0) )
            mode = wxPG_PROPERTY_VALIDATION_WRAP;

        if ( property->GetValueType() == wxT("double") )
        {
            double v_d;
            double step = property->GetAttributeAsDouble(wxT("Step"), 1.0);

            // Try double
            if ( s.ToDouble(&v_d) )
            {
                if ( bigStep )
                    step *= 10.0;

                step *= (double) spins;

                if ( evtType == wxEVT_SCROLL_LINEUP ) v_d += step;
                else v_d -= step;

                // Min/Max check
                wxFloatProperty::DoValidation(property, v_d, NULL, mode);

                wxPropertyGrid::DoubleToString(s, v_d, 6, true, NULL);
            }
            else
            {
                return false;
            }
        }
        else
        {
            wxLongLong_t v_ll;
            wxLongLong_t step = property->GetAttributeAsLong(wxT("Step"), 1);

            // Try long
            if ( wxPGStringToLongLong(s, &v_ll, 10) )
            {
                if ( bigStep )
                    step *= 10;

                step *= spins;

                if ( evtType == wxEVT_SCROLL_LINEUP ) v_ll += step;
                else v_ll -= step;

                // Min/Max check
                wxIntProperty::DoValidation(property, v_ll, NULL, mode);

                s = wxLongLong(v_ll).ToString();
            }
            else
            {
                return false;
            }
        }

        if ( tc )
        {
            int ip = tc->GetInsertionPoint();
            int lp = tc->GetLastPosition();
            tc->SetValue(s);
            tc->SetInsertionPoint(ip+(tc->GetLastPosition()-lp));
        }

        return true;
    }

    return wxPGTextCtrlEditor::OnEvent(propgrid,property,wnd,event);
}

#endif // wxUSE_SPINBTN


// -----------------------------------------------------------------------
// wxDatePickerCtrl-based property editor
// -----------------------------------------------------------------------

#if wxUSE_DATEPICKCTRL


#include <wx/datectrl.h>
#include <wx/dateevt.h>

class wxPGDatePickerCtrlEditor : public wxPGEditor
{
    WX_PG_DECLARE_EDITOR_CLASS(wxPGDatePickerCtrlEditor)
public:
    virtual ~wxPGDatePickerCtrlEditor();

    wxPG_DECLARE_CREATECONTROLS

    virtual void UpdateControl( wxPGProperty* property, wxWindow* wnd ) const;
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd, wxEvent& event ) const;
    virtual bool GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* wnd ) const;
    virtual void SetValueToUnspecified( wxPGProperty* WXUNUSED(property), wxWindow* wnd ) const;
};


WX_PG_IMPLEMENT_EDITOR_CLASS(DatePickerCtrl,wxPGDatePickerCtrlEditor,wxPGEditor)


wxPGDatePickerCtrlEditor::~wxPGDatePickerCtrlEditor()
{
    // Reset the global pointer. Useful when wxPropertyGrid is accessed
    // from an external main loop.
    wxPG_EDITOR(DatePickerCtrl) = NULL;
}

wxPGWindowList wxPGDatePickerCtrlEditor::CreateControls( wxPropertyGrid* propgrid,
                                                         wxPGProperty* property,
                                                         const wxPoint& pos,
                                                         const wxSize& sz ) const
{
    wxCHECK_MSG( property->IsKindOf(WX_PG_CLASSINFO(wxDateProperty)),
                 NULL,
                 wxT("DatePickerCtrl editor can only be used with wxDateProperty or derivative.") );

    wxDateProperty* prop = wxDynamicCast(property, wxDateProperty);

    // Use two stage creation to allow cleaner display on wxMSW
    wxDatePickerCtrl* ctrl = new wxDatePickerCtrl();
#ifdef __WXMSW__
    ctrl->Hide();
    wxSize useSz = wxDefaultSize;
    useSz.x = sz.x;
#else
    wxSize useSz = sz;
#endif

    wxDateTime dateValue(wxInvalidDateTime);
    if ( prop->GetType() == wxT("datetime") )
        dateValue = prop->GetDateValue();

    ctrl->Create(propgrid->GetPanel(),
                 wxPG_SUBID1,
                 dateValue,
                 pos,
                 useSz,
                 prop->GetDatePickerStyle() | 
                 wxNO_BORDER);

    // Connect all required events to grid's OnCustomEditorEvent
    // (all relevenat wxTextCtrl, wxComboBox and wxButton events are
    // already connected)
    propgrid->Connect( wxPG_SUBID1, wxEVT_DATE_CHANGED,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent );

#ifdef __WXMSW__
    ctrl->Show();
#endif

    return ctrl;
}

// Copies value from property to control
void wxPGDatePickerCtrlEditor::UpdateControl( wxPGProperty* property, wxWindow* wnd ) const
{
    wxDatePickerCtrl* ctrl = (wxDatePickerCtrl*) wnd;
    wxASSERT( ctrl && ctrl->IsKindOf(CLASSINFO(wxDatePickerCtrl)) );

    wxDateTime dateValue(wxInvalidDateTime);
    if ( property->GetType() == wxT("datetime") )
        dateValue = property->GetValue().GetDateTime();

    // We assume that property's data type is 'int' (or something similar),
    // thus allowing us to get raw, unchecked value via DoGetValue.
    ctrl->SetValue( dateValue );
}

// Control's events are redirected here
bool wxPGDatePickerCtrlEditor::OnEvent( wxPropertyGrid* WXUNUSED(propgrid),
                                        wxPGProperty* WXUNUSED(property),
                                        wxWindow* WXUNUSED(wnd),
                                        wxEvent& event ) const
{
    if ( event.GetEventType() == wxEVT_DATE_CHANGED )
        return true;

    return false;
}

bool wxPGDatePickerCtrlEditor::GetValueFromControl( wxVariant& variant, wxPGProperty* WXUNUSED(property), wxWindow* wnd ) const
{
    wxDatePickerCtrl* ctrl = (wxDatePickerCtrl*) wnd;
    wxASSERT( ctrl && ctrl->IsKindOf(CLASSINFO(wxDatePickerCtrl)) );

    variant = ctrl->GetValue();

    return true;
}

void wxPGDatePickerCtrlEditor::SetValueToUnspecified( wxPGProperty* property,
                                                      wxWindow* wnd ) const
{
    wxDatePickerCtrl* ctrl = (wxDatePickerCtrl*) wnd;
    wxASSERT( ctrl && ctrl->IsKindOf(CLASSINFO(wxDatePickerCtrl)) );

    wxDateProperty* prop = wxDynamicCast(property, wxDateProperty);

    if ( prop )
    {
        int datePickerStyle = prop->GetDatePickerStyle();
        if ( datePickerStyle & wxDP_ALLOWNONE )
            ctrl->SetValue(wxInvalidDateTime);
    }
}

#endif // wxUSE_DATEPICKCTRL


// -----------------------------------------------------------------------
// wxFontProperty
// -----------------------------------------------------------------------

#include <wx/fontdlg.h>
#include <wx/fontenum.h>

static const wxChar* gs_fp_es_family_labels[] = {
    wxT("Default"), wxT("Decorative"),
    wxT("Roman"), wxT("Script"),
    wxT("Swiss"), wxT("Modern"),
#if wxCHECK_VERSION(2,8,0)
    wxT("Teletype"), wxT("Unknown"),
#endif
    (const wxChar*) NULL
};

static long gs_fp_es_family_values[] = {
    wxFONTFAMILY_DEFAULT, wxFONTFAMILY_DECORATIVE,
    wxFONTFAMILY_ROMAN, wxFONTFAMILY_SCRIPT,
    wxFONTFAMILY_SWISS, wxFONTFAMILY_MODERN,
#if wxCHECK_VERSION(2,8,0)
    wxFONTFAMILY_TELETYPE, wxFONTFAMILY_UNKNOWN
#endif
};

static const wxChar* gs_fp_es_style_labels[] = {
    wxT("Normal"),
    wxT("Slant"),
    wxT("Italic"),
    (const wxChar*) NULL
};

static long gs_fp_es_style_values[] = {
    wxNORMAL,
    wxSLANT,
    wxITALIC
};

static const wxChar* gs_fp_es_weight_labels[] = {
    wxT("Normal"),
    wxT("Light"),
    wxT("Bold"),
    (const wxChar*) NULL
};

static long gs_fp_es_weight_values[] = {
    wxNORMAL,
    wxLIGHT,
    wxBOLD
};

// Class body is in advprops.h


WX_PG_IMPLEMENT_PROPERTY_CLASS(wxFontProperty,wxPGProperty,
                               wxFont,const wxFont&,TextCtrlAndButton)


wxFontProperty::wxFontProperty( const wxString& label, const wxString& name,
                                const wxFont& value )
    : wxPGProperty(label,name)
{
    SetValue( wxFontToVariant(value) );

    // Initialize font family choices list
    if ( !wxPGGlobalVars->m_fontFamilyChoices )
    {
        wxFontEnumerator enumerator;
        enumerator.EnumerateFacenames();

#if wxMINOR_VERSION > 6
        wxArrayString faceNames = enumerator.GetFacenames();
#else
        wxArrayString& faceNames = *enumerator.GetFacenames();
#endif

        faceNames.Sort();

        wxPGGlobalVars->m_fontFamilyChoices = new wxPGChoices(faceNames);
    }

    wxString emptyString(wxEmptyString);

    wxFont& font = wxFontFromVariant(m_value);

    AddPrivateChild( new wxIntProperty( _("Point Size"), wxT("Point Size"),
                     (long)font.GetPointSize() ) );

    AddPrivateChild( new wxEnumProperty(_("Family"), wxT("Family"),
                     gs_fp_es_family_labels,gs_fp_es_family_values,
                     font.GetFamily()) );

    wxString faceName = font.GetFaceName();
    // If font was not in there, add it now
    if ( faceName.length() &&
         wxPGGlobalVars->m_fontFamilyChoices->Index(faceName) == wxNOT_FOUND )
        wxPGGlobalVars->m_fontFamilyChoices->AddAsSorted(faceName);

    wxPGProperty* p = new wxEnumProperty(_("Face Name"), wxT("Face Name"),
                                         *wxPGGlobalVars->m_fontFamilyChoices);

    p->SetValueFromString(faceName, wxPG_FULL_VALUE);

    AddPrivateChild( p );

    AddPrivateChild( new wxEnumProperty(_("Style"), wxT("Style"),
              gs_fp_es_style_labels,gs_fp_es_style_values,font.GetStyle()) );

    AddPrivateChild( new wxEnumProperty(_("Weight"), wxT("Weight"),
              gs_fp_es_weight_labels,gs_fp_es_weight_values,font.GetWeight()) );

    AddPrivateChild( new wxBoolProperty(_("Underlined"), wxT("Underlined"),
              font.GetUnderlined()) );
}

wxFontProperty::~wxFontProperty() { }

void wxFontProperty::OnSetValue()
{
    wxFont& font = wxFontFromVariant(m_value);
    wxASSERT(&font);

    wxFont font2;
    if ( !font.Ok() )
        font2 = *wxNORMAL_FONT;
    else
        font2 = font;

    m_value = wxFontToVariant(font2);
}

wxString wxFontProperty::GetValueAsString( int argFlags ) const
{
    return wxPGProperty::GetValueAsString(argFlags);
}

bool wxFontProperty::OnEvent( wxPropertyGrid* propgrid, wxWindow* WXUNUSED(primary),
                              wxEvent& event )
{
    if ( propgrid->IsMainButtonEvent(event) )
    {
        // Update value from last minute changes
        PrepareValueForDialogEditing(propgrid);

        wxFontData data;
        wxFont font;

        if ( m_value.GetType() == wxT("wxFont") )
            font = wxFontFromVariant(m_value);

        data.SetInitialFont( font );
        data.SetColour(*wxBLACK);

        wxFontDialog dlg(propgrid, data);
        if ( dlg.ShowModal() == wxID_OK )
        {
            propgrid->EditorsValueWasModified();

            wxVariant variant = wxFontToVariant(dlg.GetFontData().GetChosenFont());
            SetValueInEvent( variant );
            return true;
        }
    }
    return false;
}

void wxFontProperty::RefreshChildren()
{
    if ( !GetCount() ) return;
    const wxFont& font = wxFontFromVariant(m_value);
    Item(0)->SetValue( (long)font.GetPointSize() );
    Item(1)->SetValue( (long)font.GetFamily() );
    Item(2)->SetValueFromString( font.GetFaceName(), wxPG_FULL_VALUE );
    Item(3)->SetValue( (long)font.GetStyle() );
    Item(4)->SetValue( (long)font.GetWeight() );
    Item(5)->SetValue( font.GetUnderlined() );
}

void wxFontProperty::ChildChanged( wxVariant& thisValue, int ind, wxVariant& childValue ) const
{
    wxFont& font = wxFontFromVariant(thisValue);

    if ( ind == 0 )
    {
        font.SetPointSize( wxPGVariantToInt(childValue) );
    }
    else if ( ind == 1 )
    {
        int fam = childValue.GetLong();
        if ( fam < wxDEFAULT ||
             fam > wxTELETYPE )
             fam = wxDEFAULT;
        font.SetFamily( fam );
    }
    else if ( ind == 2 )
    {
        wxString faceName;
        int faceIndex = childValue.GetLong();

        if ( faceIndex >= 0 )
            faceName = wxPGGlobalVars->m_fontFamilyChoices->GetLabel(faceIndex);

        font.SetFaceName( faceName );
    }
    else if ( ind == 3 )
    {
        int st = childValue.GetLong();
        if ( st != wxFONTSTYLE_NORMAL &&
             st != wxFONTSTYLE_SLANT &&
             st != wxFONTSTYLE_ITALIC )
             st = wxFONTWEIGHT_NORMAL;
        font.SetStyle( st );
    }
    else if ( ind == 4 )
    {
        int wt = childValue.GetLong();
        if ( wt != wxFONTWEIGHT_NORMAL &&
             wt != wxFONTWEIGHT_LIGHT &&
             wt != wxFONTWEIGHT_BOLD )
             wt = wxFONTWEIGHT_NORMAL;
        font.SetWeight( wt );
    }
    else if ( ind == 5 )
    {
        font.SetUnderlined( childValue.GetBool() );
    }
}

/*
wxSize wxFontProperty::OnMeasureImage() const
{
    return wxSize(-1,-1);
}

void wxFontProperty::OnCustomPaint(wxDC& dc,
                                        const wxRect& rect,
                                        wxPGPaintData& paintData)
{
    wxString drawFace;
    if ( paintData.m_choiceItem >= 0 )
        drawFace = wxPGGlobalVars->m_fontFamilyChoices->GetLabel(paintData.m_choiceItem);
    else
        drawFace = m_value_wxFont.GetFaceName();

    if ( drawFace.length() )
    {
        // Draw the background
        dc.SetBrush( wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)) );
        //dc.SetBrush( *wxWHITE_BRUSH );
        //dc.SetPen( *wxMEDIUM_GREY_PEN );
        dc.DrawRectangle( rect );

        wxFont oldFont = dc.GetFont();
        wxFont drawFont(oldFont.GetPointSize(),
                        wxDEFAULT,wxNORMAL,wxBOLD,false,drawFace);
        dc.SetFont(drawFont);

        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT) );
        dc.DrawText( wxT("Aa"), rect.x+2, rect.y+1 );

        dc.SetFont(oldFont);
    }
    else
    {
        // No file - just draw a white box
        dc.SetBrush ( *wxWHITE_BRUSH );
        dc.DrawRectangle ( rect );
    }
}
*/


// -----------------------------------------------------------------------
// wxSystemColourProperty
// -----------------------------------------------------------------------

// wxEnumProperty based classes cannot use wxPG_PROP_CLASS_SPECIFIC_1
#define wxPG_PROP_HIDE_CUSTOM_COLOUR        wxPG_PROP_CLASS_SPECIFIC_2

#include <wx/colordlg.h>

//#define wx_cp_es_syscolours_len 25
static const wxChar* gs_cp_es_syscolour_labels[] = {
    wxT("AppWorkspace"),
    wxT("ActiveBorder"),
    wxT("ActiveCaption"),
    wxT("ButtonFace"),
    wxT("ButtonHighlight"),
    wxT("ButtonShadow"),
    wxT("ButtonText"),
    wxT("CaptionText"),
    wxT("ControlDark"),
    wxT("ControlLight"),
    wxT("Desktop"),
    wxT("GrayText"),
    wxT("Highlight"),
    wxT("HighlightText"),
    wxT("InactiveBorder"),
    wxT("InactiveCaption"),
    wxT("InactiveCaptionText"),
    wxT("Menu"),
    wxT("Scrollbar"),
    wxT("Tooltip"),
    wxT("TooltipText"),
    wxT("Window"),
    wxT("WindowFrame"),
    wxT("WindowText"),
    wxT("Custom"),
    (const wxChar*) NULL
};

static long gs_cp_es_syscolour_values[] = {
    wxSYS_COLOUR_APPWORKSPACE,
    wxSYS_COLOUR_ACTIVEBORDER,
    wxSYS_COLOUR_ACTIVECAPTION,
    wxSYS_COLOUR_BTNFACE,
    wxSYS_COLOUR_BTNHIGHLIGHT,
    wxSYS_COLOUR_BTNSHADOW,
    wxSYS_COLOUR_BTNTEXT ,
    wxSYS_COLOUR_CAPTIONTEXT,
    wxSYS_COLOUR_3DDKSHADOW,
    wxSYS_COLOUR_3DLIGHT,
    wxSYS_COLOUR_BACKGROUND,
    wxSYS_COLOUR_GRAYTEXT,
    wxSYS_COLOUR_HIGHLIGHT,
    wxSYS_COLOUR_HIGHLIGHTTEXT,
    wxSYS_COLOUR_INACTIVEBORDER,
    wxSYS_COLOUR_INACTIVECAPTION,
    wxSYS_COLOUR_INACTIVECAPTIONTEXT,
    wxSYS_COLOUR_MENU,
    wxSYS_COLOUR_SCROLLBAR,
    wxSYS_COLOUR_INFOBK,
    wxSYS_COLOUR_INFOTEXT,
    wxSYS_COLOUR_WINDOW,
    wxSYS_COLOUR_WINDOWFRAME,
    wxSYS_COLOUR_WINDOWTEXT,
    wxPG_COLOUR_CUSTOM
};


WX_PG_IMPLEMENT_WXOBJECT_VARIANT_DATA_WITH_PROPER_EQ(wxPGVariantDataColourPropertyValue,
                                                     wxColourPropertyValue)


// Class body is in advprops.h

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxSystemColourProperty,wxEnumProperty,
                               wxColourPropertyValue,const wxColourPropertyValue&,Choice)


void wxSystemColourProperty::Init( int type, const wxColour& colour )
{
    wxColourPropertyValue cpv;

    if ( colour.Ok() )
        cpv.Init( type, colour );
    else
        cpv.Init( type, *wxWHITE );

    m_flags |= wxPG_PROP_STATIC_CHOICES; // Colour selection cannot be changed.

    m_value = wxColourPropertyValueToVariant(cpv);

    OnSetValue();
}


static wxPGChoices gs_wxSystemColourProperty_choicesCache;


wxSystemColourProperty::wxSystemColourProperty( const wxString& label, const wxString& name,
    const wxColourPropertyValue& value )
    : wxEnumProperty( label,
                      name,
                      gs_cp_es_syscolour_labels,
                      gs_cp_es_syscolour_values,
                      &gs_wxSystemColourProperty_choicesCache )
{
    if ( &value )
        Init( value.m_type, value.m_colour );
    else
        Init( wxPG_COLOUR_CUSTOM, *wxWHITE );
}


wxSystemColourProperty::wxSystemColourProperty( const wxString& label, const wxString& name,
    const wxChar** labels, const long* values, wxPGChoices* choicesCache,
    const wxColourPropertyValue& value )
    : wxEnumProperty( label, name, labels, values, choicesCache )
{
    if ( &value )
        Init( value.m_type, value.m_colour );
    else
        Init( wxPG_COLOUR_CUSTOM, *wxWHITE );
}


wxSystemColourProperty::wxSystemColourProperty( const wxString& label, const wxString& name,
    const wxChar** labels, const long* values, wxPGChoices* choicesCache,
    const wxColour& value )
    : wxEnumProperty( label, name, labels, values, choicesCache )
{
    if ( &value )
        Init( wxPG_COLOUR_CUSTOM, value );
    else
        Init( wxPG_COLOUR_CUSTOM, *wxWHITE );
}


wxSystemColourProperty::~wxSystemColourProperty() { }


wxColourPropertyValue wxSystemColourProperty::GetVal( const wxVariant* pVariant ) const
{
    if ( !pVariant )
        pVariant = &m_value;

    if ( pVariant->IsNull() )
        return wxColourPropertyValue(wxPG_COLOUR_UNSPECIFIED, wxColour());

    wxColourPropertyValue* v1 = &wxColourPropertyValueFromVariant(*pVariant);
    if ( v1 )
        return *v1;

    wxColour* pCol = wxGetVariantCast(*pVariant, wxColour);
    wxColour col;

    if ( pCol )
        col = *pCol;
    else
        col << *pVariant;

    wxColourPropertyValue v2( wxPG_COLOUR_CUSTOM, col );

    int colInd = ColToInd(col);
    if ( colInd != wxNOT_FOUND )
        v2.m_type = colInd;

    return v2;
}

wxVariant wxSystemColourProperty::DoTranslateVal( wxColourPropertyValue& v ) const
{
    return wxColourPropertyValueToVariant(v);
}

int wxSystemColourProperty::ColToInd( const wxColour& colour ) const
{
    size_t i;
    size_t i_max = m_choices.GetCount();

    if ( !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
        i_max -= 1;

    for ( i=0; i<i_max; i++ )
    {
        int ind = m_choices[i].GetValue();

        if ( colour == GetColour(ind) )
        {
            /*wxLogDebug(wxT("%s(%s): Index %i for ( getcolour(%i,%i,%i), colour(%i,%i,%i))"),
                GetClassName(),GetLabel().c_str(),
                (int)i,(int)GetColour(ind).Red(),(int)GetColour(ind).Green(),(int)GetColour(ind).Blue(),
                (int)colour.Red(),(int)colour.Green(),(int)colour.Blue());*/
            return ind;
        }
    }
    return wxNOT_FOUND;
}


static inline wxColour wxColourFromPGLong( long col )
{
    return wxColour((col&0xFF),((col>>8)&0xFF),((col>>16)&0xFF));
}


void wxSystemColourProperty::OnSetValue()
{
    // Convert from generic wxobject ptr to wxPGVariantDataColour
    if ( wxPGIsVariantType(m_value, wxobject) )
    {
        wxASSERT( m_value.IsValueKindOf(CLASSINFO(wxColour)) );
        wxColour* pCol = (wxColour*) m_value.GetWxObjectPtr();
#if wxCHECK_VERSION(2,8,0)
        m_value << *pCol;
#else
        wxPGVariantAssign(m_value, WXVARIANT(*pCol));
#endif
    }

    wxColourPropertyValue val = GetVal(&m_value);

    if ( val.m_type == wxPG_COLOUR_UNSPECIFIED )
    {
        m_value.MakeNull();
        return;
    }
    else
    {

        if ( val.m_type < wxPG_COLOUR_WEB_BASE )
            val.m_colour = GetColour( val.m_type );

        m_value = TranslateVal(val);
    }

    wxColourPropertyValue* pCpv = &wxColourPropertyValueFromVariant(m_value);
    wxColour col;
    if ( pCpv )
        col = pCpv->m_colour;
    else
        col << m_value;

    if ( !col.Ok() )
    {
        SetValueToUnspecified();
        SetIndex(wxNOT_FOUND);
        return;
    }

    int ind;

    if ( pCpv )
    {
        if ( pCpv->m_type < wxPG_COLOUR_WEB_BASE ||
             (m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
        {
            ind = GetIndexForValue(pCpv->m_type);
        }
        else
        {
            pCpv->m_type = wxPG_COLOUR_CUSTOM;
            ind = GetCustomColourIndex();
        }
    }
    else
    {
        ind = ColToInd(col);

        if ( ind == wxNOT_FOUND &&
             !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
            ind = GetCustomColourIndex();
    }

    SetIndex(ind);
}


wxColour wxSystemColourProperty::GetColour( int index ) const
{
    return wxSystemSettings::GetColour( (wxSystemColour)index );
}

wxString wxSystemColourProperty::ColourToString( const wxColour& col, int index ) const
{
    if ( index == wxNOT_FOUND )
        return wxString::Format(wxT("(%i,%i,%i)"),
                                (int)col.Red(),
                                (int)col.Green(),
                                (int)col.Blue());
    else
        return m_choices.GetLabel(index);
}

wxString wxSystemColourProperty::GetValueAsString( int argFlags ) const
{
    wxColourPropertyValue val = GetVal();

    int ind = GetIndex();

    // Always show custom colour for textctrl-editor
    if ( (val.m_type == wxPG_COLOUR_CUSTOM ||
          (ind == GetCustomColourIndex() &&
            !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR)) ||
          (argFlags & wxPG_PROPERTY_SPECIFIC)) )
    {
        return ColourToString(val.m_colour, wxNOT_FOUND);
    }

    if ( ind == -1 )
        return wxEmptyString;

    return ColourToString(val.m_colour, ind);
}


wxSize wxSystemColourProperty::OnMeasureImage( int ) const
{
    return wxPG_DEFAULT_IMAGE_SIZE;
}


int wxSystemColourProperty::GetCustomColourIndex() const
{
    return m_choices.GetCount() - 1;
}


bool wxSystemColourProperty::QueryColourFromUser( wxVariant& variant ) const
{
    wxASSERT( m_value.GetType() != wxT("string") );
    bool res = false;

    wxPropertyGrid* propgrid = GetGrid();
    wxASSERT( propgrid );

    // Must only occur when user triggers event
    if ( !(propgrid->GetInternalFlags() & wxPG_FL_IN_ONCUSTOMEDITOREVENT) )
        return res;

    wxColourPropertyValue val = GetVal();

    val.m_type = wxPG_COLOUR_CUSTOM;

    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(val.m_colour);
    int i;
    for ( i = 0; i < 16; i++)
    {
        wxColour colour(i*16, i*16, i*16);
        data.SetCustomColour(i, colour);
    }

    wxColourDialog dialog(propgrid, &data);
    if ( dialog.ShowModal() == wxID_OK )
    {
        wxColourData retData = dialog.GetColourData();
        val.m_colour = retData.GetColour();

        variant = DoTranslateVal(val);

        SetValueInEvent(variant);

        res = true;
    }

    return res;
}


bool wxSystemColourProperty::IntToValue( wxVariant& variant, int number, int WXUNUSED(argFlags) ) const
{
    int index = number;
    int type = GetValueForIndex(index);

    if ( type == wxPG_COLOUR_CUSTOM )
    {
        QueryColourFromUser(variant);
    }
    else
    {
        variant = TranslateVal( type, GetColour(type) );
    }

    return true;
}

// Need to do some extra event handling.
bool wxSystemColourProperty::OnEvent( wxPropertyGrid* propgrid,
                                      wxWindow* WXUNUSED(primary),
                                      wxEvent& event )
{
    bool askColour = false;

    if ( propgrid->IsMainButtonEvent(event) )
    {
        askColour = true;
    }
    else if ( event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED )
    {
		// Must override index detection since at this point GetIndex()
		// will return old value.
		wxPGOwnerDrawnComboBox* cb =
			(wxPGOwnerDrawnComboBox*)propgrid->GetEditorControl();

		if ( cb )
		{
			int index = cb->GetSelection();

			if ( index == GetCustomColourIndex() &&
				 !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
				askColour = true;
		}
    }

    if ( askColour && !propgrid->WasValueChangedInEvent() )
    {
        // We need to handle button click in case editor has been
        // switched to one that has wxButton as well.
        wxVariant variant;
        if ( QueryColourFromUser(variant) )
            return true;
    }
    return false;
}

/*class wxPGColourPropertyRenderer : public wxPGDefaultRenderer
{
public:
    virtual void Render( wxDC& dc, const wxRect& rect,
                         const wxPropertyGrid* propertyGrid, wxPGProperty* property,
                         int WXUNUSED(column), int item, int WXUNUSED(flags) ) const
    {
        wxASSERT( property->IsKindOf(CLASSINFO(wxSystemColourProperty)) );
        wxSystemColourProperty* prop = wxStaticCast(property, wxSystemColourProperty);

        dc.SetPen(*wxBLACK_PEN);
        if ( item >= 0 &&
             ( item < (int)(GetCustomColourIndex) || (prop->HasFlag(wxPG_PROP_HIDE_CUSTOM_COLOUR)))
           )
        {
            int colInd;
            const wxArrayInt& values = prop->GetValues();
            if ( values.GetCount() )
                colInd = values[item];
            else
                colInd = item;
            dc.SetBrush( wxColour( prop->GetColour( colInd ) ) );
        }
        else if ( !prop->IsValueUnspecified() )
            dc.SetBrush( prop->GetVal().m_colour );
        else
            dc.SetBrush( *wxWHITE );

        wxRect imageRect = propertyGrid->GetImageRect(property, item);
        wxLogDebug(wxT("%i, %i"),imageRect.x,imageRect.y);
        dc.DrawRectangle( rect.x+imageRect.x, rect.y+imageRect.y,
                          imageRect.width, imageRect.height );

        wxString text;
        if ( item == -1 )
            text = property->GetValueAsString();
        else
            text = property->GetChoiceString(item);
        DrawText( dc, rect, imageRect.width, text );
    }
protected:
};

wxPGColourPropertyRenderer g_wxPGColourPropertyRenderer;

wxPGCellRenderer* wxSystemColourProperty::GetCellRenderer( int column ) const
{
    if ( column == 1 )
        return &g_wxPGColourPropertyRenderer;
    return wxEnumProperty::GetCellRenderer(column);
}*/

void wxSystemColourProperty::OnCustomPaint( wxDC& dc, const wxRect& rect,
                                            wxPGPaintData& paintdata )
{
    wxColour col;

    if ( paintdata.m_choiceItem >= 0 &&
         paintdata.m_choiceItem < (int)m_choices.GetCount() &&
         (paintdata.m_choiceItem != GetCustomColourIndex() ||
          m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
    {
        int colInd = m_choices[paintdata.m_choiceItem].GetValue();
        col = GetColour( colInd );
    }
    else if ( !IsValueUnspecified() )
    {
        col = GetVal().m_colour;
    }

    if ( col.Ok() )
    {
        dc.SetBrush(col);
        dc.DrawRectangle(rect);
    }
}


bool wxSystemColourProperty::StringToValue( wxVariant& value, const wxString& text, int argFlags ) const
{
    //
    // Accept colour format "[Name] [(R,G,B)]"
    // Name takes precedence.
    //
    wxString colourName;
    wxString colourRGB;

    int ppos = text.Find(wxT("("));

    if ( ppos == wxNOT_FOUND )
    {
        colourName = text;
    }
    else
    {
        colourName = text.substr(0, ppos);
        colourRGB = text.substr(ppos, text.length()-ppos);
    }

    // Strip spaces from extremities
    colourName.Trim(true);
    colourName.Trim(false);
    colourRGB.Trim(true);

    // Validate colourRGB string - (1,1,1) is shortest allowed
    if ( colourRGB.length() < 7 )
        colourRGB.clear();

    if ( colourRGB.length() == 0 && m_choices.GetCount() &&
         !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) &&
         colourName == m_choices.GetLabel(GetCustomColourIndex()) )
    {
        if ( !(argFlags & wxPG_EDITABLE_VALUE ))
        {
            // This really should not occurr...
            // wxASSERT(false);
            ResetNextIndex();
            return false;
        }

        QueryColourFromUser(value);
    }
    else
    {
        wxColourPropertyValue val;

        bool done = false;

        if ( colourName.length() )
        {
            // Try predefined colour first
            bool res = wxEnumProperty::StringToValue(value, colourName, argFlags);
            if ( res && GetIndex() >= 0 )
            {
                val.m_type = GetIndex();
                if ( val.m_type >= 0 && val.m_type < m_choices.GetCount() )
                    val.m_type = m_choices[val.m_type].GetValue();

                // Get proper colour for type.
                val.m_colour = GetColour(val.m_type);

                done = true;
            }
        }
        if ( colourRGB.length() && !done )
        {
            // Then check custom colour.
            val.m_type = wxPG_COLOUR_CUSTOM;

            int r = -1, g = -1, b = -1;
            wxSscanf(colourRGB.c_str(),wxT("(%i,%i,%i)"),&r,&g,&b);

            if ( r >= 0 && r <= 255 &&
                 g >= 0 && g <= 255 &&
                 b >= 0 && b <= 255 )
            {
                val.m_colour.Set(r,g,b);

                done = true;
            }
        }

        if ( !done )
        {
            ResetNextIndex();
            return false;
        }

        value = DoTranslateVal(val);
    }

    return true;
}


bool wxSystemColourProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    if ( name == wxPG_COLOUR_ALLOW_CUSTOM )
    {
        int ival = wxPGVariantToInt(value);

        SetChoicesExclusive(); // Make sure we don't corrupt colour lists of other properties

        if ( ival && (m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
        {
            // Show custom choice
            m_choices.Insert(wxT("Custom"), GetCustomColourIndex(), wxPG_COLOUR_CUSTOM);
            m_flags &= ~(wxPG_PROP_HIDE_CUSTOM_COLOUR);
        }
        else if ( !ival && !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
        {
            // Hide custom choice
            m_choices.RemoveAt(GetCustomColourIndex());
            m_flags |= wxPG_PROP_HIDE_CUSTOM_COLOUR;
        }
        return true;
    }
    return false;
}


// -----------------------------------------------------------------------
// wxColourProperty
// -----------------------------------------------------------------------

static const wxChar* gs_cp_es_normcolour_labels[] = {
    wxT("Black"),
    wxT("Maroon"),
    wxT("Navy"),
    wxT("Purple"),
    wxT("Teal"),
    wxT("Gray"),
    wxT("Green"),
    wxT("Olive"),
    wxT("Brown"),
    wxT("Blue"),
    wxT("Fuchsia"),
    wxT("Red"),
    wxT("Orange"),
    wxT("Silver"),
    wxT("Lime"),
    wxT("Aqua"),
    wxT("Yellow"),
    wxT("White"),
    wxT("Custom"),
    (const wxChar*) NULL
};

static unsigned long gs_cp_es_normcolour_colours[] = {
    wxPG_COLOUR(0,0,0),
    wxPG_COLOUR(128,0,0),
    wxPG_COLOUR(0,0,128),
    wxPG_COLOUR(128,0,128),
    wxPG_COLOUR(0,128,128),
    wxPG_COLOUR(128,128,128),
    wxPG_COLOUR(0,128,0),
    wxPG_COLOUR(128,128,0),
    wxPG_COLOUR(166,124,81),
    wxPG_COLOUR(0,0,255),
    wxPG_COLOUR(255,0,255),
    wxPG_COLOUR(255,0,0),
    wxPG_COLOUR(247,148,28),
    wxPG_COLOUR(192,192,192),
    wxPG_COLOUR(0,255,0),
    wxPG_COLOUR(0,255,255),
    wxPG_COLOUR(255,255,0),
    wxPG_COLOUR(255,255,255),
    wxPG_COLOUR(0,0,0)
};

WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR2(wxColourProperty,
                                                     gs_cp_es_normcolour_labels,
                                                     (const long*)NULL,
                                                     gs_cp_es_normcolour_colours,
                                                     TextCtrlAndButton)

// -----------------------------------------------------------------------
// wxCursorProperty
// -----------------------------------------------------------------------

#define wxPG_CURSOR_IMAGE_WIDTH     32

#define NUM_CURSORS 28

//#define wx_cp_es_syscursors_len 28
static const wxChar* gs_cp_es_syscursors_labels[NUM_CURSORS+1] = {
    wxT("Default"),
    wxT("Arrow"),
    wxT("Right Arrow"),
    wxT("Blank"),
    wxT("Bullseye"),
    wxT("Character"),
    wxT("Cross"),
    wxT("Hand"),
    wxT("I-Beam"),
    wxT("Left Button"),
    wxT("Magnifier"),
    wxT("Middle Button"),
    wxT("No Entry"),
    wxT("Paint Brush"),
    wxT("Pencil"),
    wxT("Point Left"),
    wxT("Point Right"),
    wxT("Question Arrow"),
    wxT("Right Button"),
    wxT("Sizing NE-SW"),
    wxT("Sizing N-S"),
    wxT("Sizing NW-SE"),
    wxT("Sizing W-E"),
    wxT("Sizing"),
    wxT("Spraycan"),
    wxT("Wait"),
    wxT("Watch"),
    wxT("Wait Arrow"),
    (const wxChar*) NULL
};

static long gs_cp_es_syscursors_values[NUM_CURSORS] = {
    wxCURSOR_NONE,
    wxCURSOR_ARROW,
    wxCURSOR_RIGHT_ARROW,
    wxCURSOR_BLANK,
    wxCURSOR_BULLSEYE,
    wxCURSOR_CHAR,
    wxCURSOR_CROSS,
    wxCURSOR_HAND,
    wxCURSOR_IBEAM,
    wxCURSOR_LEFT_BUTTON,
    wxCURSOR_MAGNIFIER,
    wxCURSOR_MIDDLE_BUTTON,
    wxCURSOR_NO_ENTRY,
    wxCURSOR_PAINT_BRUSH,
    wxCURSOR_PENCIL,
    wxCURSOR_POINT_LEFT,
    wxCURSOR_POINT_RIGHT,
    wxCURSOR_QUESTION_ARROW,
    wxCURSOR_RIGHT_BUTTON,
    wxCURSOR_SIZENESW,
    wxCURSOR_SIZENS,
    wxCURSOR_SIZENWSE,
    wxCURSOR_SIZEWE,
    wxCURSOR_SIZING,
    wxCURSOR_SPRAYCAN,
    wxCURSOR_WAIT,
    wxCURSOR_WATCH,
    wxCURSOR_ARROWWAIT
};

IMPLEMENT_DYNAMIC_CLASS(wxCursorProperty, wxEnumProperty)

wxCursorProperty::wxCursorProperty( const wxString& label, const wxString& name,
    int value )
    : wxEnumProperty( label,
                      name,
                      gs_cp_es_syscursors_labels,
                      gs_cp_es_syscursors_values,
                      value )
{
    m_flags |= wxPG_PROP_STATIC_CHOICES; // Cursor selection cannot be changed.
}

wxCursorProperty::~wxCursorProperty()
{
}

wxSize wxCursorProperty::OnMeasureImage( int item ) const
{
#if wxPG_CAN_DRAW_CURSOR
    if ( item != -1 && item < NUM_CURSORS )
        return wxSize(wxPG_CURSOR_IMAGE_WIDTH,wxPG_CURSOR_IMAGE_WIDTH);
#endif
    return wxSize(0,0);
}

#if wxPG_CAN_DRAW_CURSOR

void wxCursorProperty::OnCustomPaint( wxDC& dc,
                                      const wxRect& rect,
                                      wxPGPaintData& paintdata )
{
    // Background brush
    dc.SetBrush( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

    if ( paintdata.m_choiceItem >= 0 )
    {
        dc.DrawRectangle( rect );

        if ( paintdata.m_choiceItem < NUM_CURSORS )
        {
            int cursorindex = gs_cp_es_syscursors_values[paintdata.m_choiceItem];

            {
                if ( cursorindex == wxCURSOR_NONE )
                    cursorindex = wxCURSOR_ARROW;

                wxCursor cursor( cursorindex );

            #ifdef __WXMSW__
                  ::DrawIconEx( wxPG_GetHDCOf(dc),
                              rect.x,
                              rect.y,
                              (HICON)cursor.GetHandle(),
                              0,
                              0,
                              0,
                              NULL,
            #if !defined(__WXWINCE__)
                              DI_COMPAT | DI_DEFAULTSIZE |
            #endif
                              DI_NORMAL
                            );
            #endif
            }
        }
    }
}

#else
void wxCursorProperty::OnCustomPaint( wxDC&, const wxRect&, wxPGPaintData& ) { }
/*wxPGCellRenderer* wxCursorProperty::GetCellRenderer( int column ) const
{
    return wxEnumProperty::GetCellRenderer(column);
}*/
#endif

// -----------------------------------------------------------------------
// wxImageFileProperty
// -----------------------------------------------------------------------

#if wxUSE_IMAGE

const wxString& wxPGGetDefaultImageWildcard()
{
    // Form the wildcard, if not done yet
    if ( !wxPGGlobalVars->m_pDefaultImageWildcard.length() )
    {

        wxString str;

        // TODO: This section may require locking (using global).

        wxList& handlers = wxImage::GetHandlers();

        wxList::iterator node;

        // Let's iterate over the image handler list.
        //for ( wxList::Node *node = handlers.GetFirst(); node; node = node->GetNext() )
        for ( node = handlers.begin(); node != handlers.end(); node++ )
        {
            wxImageHandler *handler = (wxImageHandler*)*node;

            wxString ext_lo = handler->GetExtension();
            wxString ext_up = ext_lo.Upper();

            str.append( ext_up );
            str.append( wxT(" files (*.") );
            str.append( ext_up );
            str.append( wxT(")|*.") );
            str.append( ext_lo );
            str.append( wxT("|") );
        }

        str.append ( wxT("All files (*.*)|*.*") );

        wxPGGlobalVars->m_pDefaultImageWildcard = str;
    }

    return wxPGGlobalVars->m_pDefaultImageWildcard;
}

WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(wxImageFileProperty,
                                       wxFileProperty,
                                       const wxString&)

wxImageFileProperty::wxImageFileProperty( const wxString& label, const wxString& name,
    const wxString& value )
    : wxFileProperty(label,name,value)
{
    SetAttribute( wxPG_FILE_WILDCARD, wxPGGetDefaultImageWildcard() );

    m_pImage = (wxImage*) NULL;
    m_pBitmap = (wxBitmap*) NULL;
}

wxImageFileProperty::~wxImageFileProperty()
{
    if ( m_pBitmap )
        delete m_pBitmap;
    if ( m_pImage )
        delete m_pImage;
}

void wxImageFileProperty::OnSetValue()
{
    wxFileProperty::OnSetValue();

    // Delete old image
    if ( m_pImage )
    {
        delete m_pImage;
        m_pImage = NULL;
    }
    if ( m_pBitmap )
    {
        delete m_pBitmap;
        m_pBitmap = NULL;
    }

    // Create the image thumbnail
    if ( m_filename.FileExists() )
    {
        m_pImage = new wxImage( m_filename.GetFullPath() );
    }
}

wxSize wxImageFileProperty::OnMeasureImage( int ) const
{
    return wxPG_DEFAULT_IMAGE_SIZE;
}

void wxImageFileProperty::OnCustomPaint( wxDC& dc,
                                         const wxRect& rect,
                                         wxPGPaintData& )
{
    if ( m_pBitmap || (m_pImage && m_pImage->Ok() ) )
    {
        // Draw the thumbnail

        // Create the bitmap here because required size is not known in OnSetValue().
        if ( !m_pBitmap )
        {
            m_pImage->Rescale( rect.width, rect.height );
            m_pBitmap = new wxBitmap( *m_pImage );
            delete m_pImage;
            m_pImage = NULL;
        }

        dc.DrawBitmap( *m_pBitmap, rect.x, rect.y, false );
    }
    else
    {
        // No file - just draw a white box
        dc.SetBrush( *wxWHITE_BRUSH );
        dc.DrawRectangle ( rect );
    }
}

#endif // wxUSE_IMAGE

// -----------------------------------------------------------------------
// wxMultiChoiceProperty
// -----------------------------------------------------------------------

#if wxUSE_CHOICEDLG

#include <wx/choicdlg.h>

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxMultiChoiceProperty,wxPGProperty,
                               wxArrayInt,const wxArrayInt&,TextCtrlAndButton)

wxMultiChoiceProperty::wxMultiChoiceProperty( const wxString& label,
                                              const wxString& name,
                                              const wxPGChoices& choices,
                                              const wxArrayString& value)
                                                : wxPGProperty(label,name)
{
    m_choices.Assign(choices);
    SetValue(value);
}

wxMultiChoiceProperty::wxMultiChoiceProperty( const wxString& label,
                                              const wxString& name,
                                              const wxArrayString& strings,
                                              const wxArrayString& value)
                                                : wxPGProperty(label,name)
{
    m_choices.Set(strings);
    SetValue(value);
}

wxMultiChoiceProperty::wxMultiChoiceProperty( const wxString& label,
                                              const wxString& name,
                                              const wxArrayString& value)
                                                : wxPGProperty(label,name)
{
    wxArrayString strings;
    m_choices.Set(strings);
    SetValue(value);
}

wxMultiChoiceProperty::~wxMultiChoiceProperty()
{
}

void wxMultiChoiceProperty::OnSetValue()
{
    GenerateValueAsString();
}

wxString wxMultiChoiceProperty::GetValueAsString( int ) const
{
    return m_display;
}

void wxMultiChoiceProperty::GenerateValueAsString()
{
    wxArrayString strings;

    if ( wxPGIsVariantType(m_value, arrstring) )
        strings = m_value.GetArrayString();

    wxString& tempStr = m_display;
    unsigned int i;
    unsigned int itemCount = strings.GetCount();

    tempStr.Empty();

    if ( itemCount )
        tempStr.append( wxT("\"") );

    for ( i = 0; i < itemCount; i++ )
    {
        tempStr.append( strings[i] );
        tempStr.append( wxT("\"") );
        if ( i < (itemCount-1) )
            tempStr.append ( wxT(" \"") );
    }
}

wxArrayInt wxMultiChoiceProperty::GetValueAsIndices() const
{
    const wxArrayInt& valueArr = wxArrayIntFromVariant(GetValue());
    unsigned int i;

    // Translate values to string indices.
    wxArrayInt selections;

    if ( !m_choices.IsOk() || !m_choices.GetCount() || !(&valueArr) )
    {
        for ( i=0; i<valueArr.GetCount(); i++ )
            selections.Add(-1);
    }
    else
    {
        for ( i=0; i<valueArr.GetCount(); i++ )
        {
            int sIndex = m_choices.Index(valueArr[i]);
            if ( sIndex >= 0 )
                selections.Add(sIndex);
        }
    }

    return selections;
}

bool wxMultiChoiceProperty::OnEvent( wxPropertyGrid* propgrid,
                                     wxWindow* WXUNUSED(primary),
                                     wxEvent& event )
{
    if ( propgrid->IsMainButtonEvent(event) )
    {
        // Update the value
        PrepareValueForDialogEditing(propgrid);

        wxArrayString labels = m_choices.GetLabels();
        unsigned int choiceCount;

        if ( m_choices.IsOk() )
            choiceCount = m_choices.GetCount();
        else
            choiceCount = 0;

        // launch editor dialog
        wxMultiChoiceDialog dlg( propgrid,
                                 _("Make a selection:"),
                                 m_label,
                                 choiceCount,
                                 choiceCount?&labels[0]:NULL,
                                 wxCHOICEDLG_STYLE );

        dlg.Move( propgrid->GetGoodEditorDialogPosition(this,dlg.GetSize()) );

        wxArrayString strings = m_value.GetArrayString();
        wxArrayString extraStrings;

        dlg.SetSelections(m_choices.GetIndicesForStrings(strings, &extraStrings));

        if ( dlg.ShowModal() == wxID_OK && choiceCount )
        {
            int userStringMode = GetAttributeAsLong(wxT("UserStringMode"), 0);

            wxArrayInt arrInt = dlg.GetSelections();

            wxVariant variant;

            // Strings that were not in list of choices
            wxArrayString value;

            // Translate string indices to strings

            unsigned int n;
            if ( userStringMode == 1 )
            {
                for (n=0;n<extraStrings.size();n++)
                    value.push_back(extraStrings[n]);
            }

            unsigned int i;
            for ( i=0; i<arrInt.GetCount(); i++ )
                value.Add(m_choices.GetLabel(arrInt.Item(i)));

            if ( userStringMode == 2 )
            {
                for (n=0;n<extraStrings.size();n++)
                    value.push_back(extraStrings[n]);
            }

            variant = WXVARIANT(value);

            SetValueInEvent(variant);

            return true;
        }
    }
    return false;
}

int wxMultiChoiceProperty::GetChoiceInfo( wxPGChoiceInfo* choiceinfo )
{
    if ( choiceinfo )
        choiceinfo->m_choices = &m_choices;
    return -1;
}

bool wxMultiChoiceProperty::StringToValue( wxVariant& variant, const wxString& text, int ) const
{
    wxArrayString arr;

    int userStringMode = GetAttributeAsLong(wxT("UserStringMode"), 0);

    WX_PG_TOKENIZER2_BEGIN(text,wxT('"'))
        if ( userStringMode > 0 || (m_choices.IsOk() && m_choices.Index( token ) != wxNOT_FOUND) )
            arr.Add(token);
    WX_PG_TOKENIZER2_END()

    wxVariant v(arr);
    variant = v;

    return true;
}

#endif // wxUSE_CHOICEDLG


// -----------------------------------------------------------------------
// wxDateProperty
// -----------------------------------------------------------------------

#if wxUSE_DATETIME


#if wxUSE_DATEPICKCTRL
    #define dtCtrl      DatePickerCtrl
#else
    #define dtCtrl      TextCtrl
#endif

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxDateProperty,
                               wxPGProperty,
                               wxDateTime,
                               const wxDateTime&,
                               dtCtrl)


wxString wxDateProperty::ms_defaultDateFormat;


wxDateProperty::wxDateProperty( const wxString& label,
                                const wxString& name,
                                const wxDateTime& value )
    : wxPGProperty(label,name)
{
    //wxPGRegisterDefaultValueType(wxDateTime)

#if wxUSE_DATEPICKCTRL
    wxPGRegisterEditorClass(DatePickerCtrl);

    m_dpStyle = wxDP_DEFAULT | wxDP_SHOWCENTURY;
#else
    m_dpStyle = 0;
#endif

    SetValue( value );
}

wxDateProperty::~wxDateProperty()
{
}

void wxDateProperty::OnSetValue()
{
    //
    // Convert invalid dates to unspecified value
    if ( m_value.GetType() == wxT("datetime") )
    {
        if ( !m_value.GetDateTime().IsValid() )
            m_value.MakeNull();
    }
}

bool wxDateProperty::StringToValue( wxVariant& variant, const wxString& text,
                                    int WXUNUSED(argFlags) ) const
{
    wxDateTime dt;

#if wxCHECK_VERSION(2,9,0)
    const char* c = dt.ParseFormat(text, wxString(wxDefaultDateTimeFormat), wxDefaultDateTime, NULL);
#else
    const wxChar* c = dt.ParseFormat(text, wxDefaultDateTimeFormat);
#endif

    if ( c )
    {
        variant = dt;
        return true;
    }

    return false;
}

wxString wxDateProperty::GetValueAsString( int argFlags ) const
{
    const wxChar* format = (const wxChar*) NULL;

    wxDateTime dateTime = m_value.GetDateTime();

    if ( !dateTime.IsValid() )
        return wxT("Invalid");

    if ( !ms_defaultDateFormat.length() )
    {
#if wxUSE_DATEPICKCTRL
        bool showCentury = m_dpStyle & wxDP_SHOWCENTURY ? true : false;
#else
        bool showCentury = true;
#endif
        ms_defaultDateFormat = DetermineDefaultDateFormat( showCentury );
    }

    if ( m_format.length() &&
         !(argFlags & wxPG_FULL_VALUE) )
            format = m_format.c_str();

    // Determine default from locale
    // NB: This is really simple stuff, but can't figure anything
    //     better without proper support in wxLocale
    if ( !format )
        format = ms_defaultDateFormat.c_str();

    return dateTime.Format(format);
}

wxString wxDateProperty::DetermineDefaultDateFormat( bool showCentury )
{
    // This code is basicly copied from datectlg.cpp's SetFormat
    //
    wxString format;

    wxDateTime dt;
    dt.ParseFormat(wxT("2003-10-13"), wxT("%Y-%m-%d"));
    wxString str(dt.Format(wxT("%x")));

    const wxChar *p = str.c_str();
    while ( *p )
    {
        int n=wxAtoi(p);
        if (n == dt.GetDay())
        {
            format.Append(wxT("%d"));
            p += 2;
        }
        else if (n == (int)dt.GetMonth()+1)
        {
            format.Append(wxT("%m"));
            p += 2;
        }
        else if (n == dt.GetYear())
        {
            format.Append(wxT("%Y"));
            p += 4;
        }
        else if (n == (dt.GetYear() % 100))
        {
            if (showCentury)
                format.Append(wxT("%Y"));
            else
                format.Append(wxT("%y"));
            p += 2;
        }
        else
            format.Append(*p++);
    }

    return format;
}

bool wxDateProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    if ( name == wxPG_DATE_FORMAT )
    {
        m_format = value.GetString();
        return true;
    }
    else if ( name == wxPG_DATE_PICKER_STYLE )
    {
        m_dpStyle = value.GetLong();
        ms_defaultDateFormat.clear();  // This may need recalculation
        return true;
    }
    return false;
}

#endif  // wxUSE_DATETIME


// -----------------------------------------------------------------------
// wxPropertyGridInterface
// -----------------------------------------------------------------------

void wxPropertyGridInterface::InitAllTypeHandlers()
{
    //wxPG_INIT_REQUIRED_TYPE(wxColour)
    //wxPG_INIT_REQUIRED_TYPE(wxFont)
    //wxPG_INIT_REQUIRED_TYPE(wxArrayInt)
    //wxPG_INIT_REQUIRED_TYPE(wxColourPropertyValue)
/*#if wxUSE_DATETIME
    wxPGRegisterDefaultValueType(wxDateTime)
#endif*/
}

// -----------------------------------------------------------------------

void wxPropertyGridInterface::RegisterAdditionalEditors()
{
#if wxUSE_SPINBTN
    wxPGRegisterEditorClass(SpinCtrl);
#endif
#if wxUSE_DATEPICKCTRL
    wxPGRegisterEditorClass(DatePickerCtrl);
#endif
}

// -----------------------------------------------------------------------

#endif // wxPG_INCLUDE_ADVPROPS
