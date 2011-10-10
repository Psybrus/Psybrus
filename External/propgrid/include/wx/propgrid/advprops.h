/////////////////////////////////////////////////////////////////////////////
// Name:        advprops.h
// Purpose:     wxPropertyGrid Advanced Properties (font, colour, etc.)
// Author:      Jaakko Salli
// Modified by:
// Created:     Sep-25-2004
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_ADVPROPS_H_
#define _WX_PROPGRID_ADVPROPS_H_

#ifdef DOXYGEN
    #define wxUSE_IMAGE 1
    #define wxUSE_CHOICEDLG 1
    #define wxUSE_DATETIME 1
#else

    #ifndef wxPG_INCLUDE_ADVPROPS
        #error "Include propgrid.h"
    #endif

#endif

#include "props.h"


// -----------------------------------------------------------------------


//
// Additional Value Type Handlers
//
#ifndef SWIG
bool WXDLLIMPEXP_PG operator == (const wxArrayInt& array1, const wxArrayInt& array2);


//
// Additional Property Editors
//
#if wxUSE_SPINBTN
WX_PG_DECLARE_EDITOR_WITH_DECL(SpinCtrl,WXDLLIMPEXP_PG)
#endif

#if wxUSE_DATEPICKCTRL
WX_PG_DECLARE_EDITOR_WITH_DECL(DatePickerCtrl,WXDLLIMPEXP_PG)
#endif

#endif  // !SWIG

// -----------------------------------------------------------------------


// Web colour is currently unsupported
#define wxPG_COLOUR_WEB_BASE        0x10000
//#define wxPG_TO_WEB_COLOUR(A)   ((wxUint32)(A+wxPG_COLOUR_WEB_BASE))


#define wxPG_COLOUR_CUSTOM      0xFFFFFF
#define wxPG_COLOUR_UNSPECIFIED (wxPG_COLOUR_CUSTOM+1)

/** @class wxColourPropertyValue
    @ingroup classes
    @brief Because text, background and other colours tend to differ between
    platforms, wxSystemColourProperty must be able to select between system
    colour and, when necessary, to pick a custom one. wxSystemColourProperty
    value makes this possible.
*/
class WXDLLIMPEXP_PG wxColourPropertyValue : public wxObject
{
public:
    /** An integer value relating to the colour, and which exact
        meaning depends on the property with which it is used.

        For wxSystemColourProperty:

        Any of wxSYS_COLOUR_XXX, or any web-colour ( use wxPG_TO_WEB_COLOUR
        macro - (currently unsupported) ), or wxPG_COLOUR_CUSTOM.

        For custom colour properties without values array specified:

        index or wxPG_COLOUR_CUSTOM

        For custom colour properties <b>with</b> values array specified:

        m_arrValues[index] or wxPG_COLOUR_CUSTOM
    */
    wxUint32    m_type;

    /** Resulting colour. Should be correct regardless of type. */
    wxColour    m_colour;

    wxColourPropertyValue()
        : wxObject()
    {
        m_type = 0;
    }

    wxColourPropertyValue( const wxColourPropertyValue& v )
        : wxObject()
    {
        m_type = v.m_type;
        m_colour = v.m_colour;
    }

    virtual ~wxColourPropertyValue()
    {
    }

    void Init( wxUint32 type, const wxColour& colour )
    {
        m_type = type;
        m_colour = colour;
    }

    wxColourPropertyValue( const wxColour& colour )
        : wxObject()
    {
        m_type = wxPG_COLOUR_CUSTOM;
        m_colour = colour;
    }

    wxColourPropertyValue( wxUint32 type )
        : wxObject()
    {
        m_type = type;
    }

    wxColourPropertyValue( wxUint32 type, const wxColour& colour )
        : wxObject()
    {
        Init( type, colour );
    }

#ifndef SWIG

    void operator= (const wxColourPropertyValue& cpv)
    {
        Init( cpv.m_type, cpv.m_colour );
    }

private:
    DECLARE_DYNAMIC_CLASS(wxColourPropertyValue)
#endif
};


#ifndef SWIG
bool WXDLLIMPEXP_PG operator == (const wxColourPropertyValue&, const wxColourPropertyValue&);

WX_PG_DECLARE_WXOBJECT_VARIANT_DATA(wxPGVariantDataColourPropertyValue, wxColourPropertyValue, WXDLLIMPEXP_PG)
#endif

#ifndef SWIG
    #define wxPG_EMPTY_CPV          (*((wxColourPropertyValue*)NULL))
    #define wxPG_NORMAL_FONT        (*wxNORMAL_FONT)
#else
    #define wxPG_EMPTY_CPV          wxCPV_wxPG_EMPTY
    #define wxPG_NORMAL_FONT        wxFONT_wxPG_NORMAL_FONT
#endif


// -----------------------------------------------------------------------
// Declare part of custom colour property macro pairs.

#if wxUSE_IMAGE || defined(SWIG)
    #include <wx/image.h>
#endif

// -----------------------------------------------------------------------

/** @class wxFontProperty
    @ingroup classes
    @brief Property representing wxFont.
*/
class WXDLLIMPEXP_PG wxFontProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxFontProperty)
public:

    wxFontProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, const wxFont& value = wxFont() );
    virtual ~wxFontProperty();
    virtual void OnSetValue();
    virtual wxString GetValueAsString( int argFlags = 0 ) const;

    WX_PG_DECLARE_EVENT_METHODS()
    WX_PG_DECLARE_PARENTAL_METHODS()
    //WX_PG_DECLARE_CUSTOM_PAINT_METHODS()

protected:
};

// -----------------------------------------------------------------------


/** If set, then match from list is searched for a custom colour. */
#define wxPG_PROP_TRANSLATE_CUSTOM      wxPG_PROP_CLASS_SPECIFIC_1


/** @class wxSystemColourProperty
    @ingroup classes
    @brief Has dropdown list of wxWidgets system colours. Value used is
    of wxColourPropertyValue type.
*/
class WXDLLIMPEXP_PG wxSystemColourProperty : public wxEnumProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxSystemColourProperty)
public:

    wxSystemColourProperty( const wxString& label = wxPG_LABEL,
                            const wxString& name = wxPG_LABEL,
                            const wxColourPropertyValue& value = wxColourPropertyValue() );
    ~wxSystemColourProperty();

    virtual void OnSetValue();
    virtual bool IntToValue( wxVariant& variant, int number, int argFlags = 0 ) const;

    /** Override in derived class to customize how colours are printed as strings.
    */
    virtual wxString ColourToString( const wxColour& col, int index ) const;

    /** Returns index of entry that triggers colour picker dialog
        (default is last).
    */
    virtual int GetCustomColourIndex() const;

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_EVENT_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()

    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()
    //virtual wxSize GetImageSize( int item ) const;
    //virtual wxPGCellRenderer* GetCellRenderer( int column ) const;

    // Helper function to show the colour dialog
    bool QueryColourFromUser( wxVariant& variant ) const;

    /** Default is to use wxSystemSettings::GetColour(index). Override to use
        custom colour tables etc.
    */
    virtual wxColour GetColour( int index ) const;

    wxColourPropertyValue GetVal( const wxVariant* pVariant = NULL ) const;

protected:

    // Special constructors to be used by derived classes.
    wxSystemColourProperty( const wxString& label, const wxString& name,
        const wxChar** labels, const long* values, wxPGChoices* choicesCache,
        const wxColourPropertyValue& value );
    wxSystemColourProperty( const wxString& label, const wxString& name,
        const wxChar** labels, const long* values, wxPGChoices* choicesCache,
        const wxColour& value );

    void Init( int type, const wxColour& colour );

    // Utility functions for internal use
    virtual wxVariant DoTranslateVal( wxColourPropertyValue& v ) const;
    wxVariant TranslateVal( wxColourPropertyValue& v ) const
    {
        return DoTranslateVal( v );
    }
    wxVariant TranslateVal( int type, const wxColour& colour ) const
    {
        wxColourPropertyValue v(type, colour);
        return DoTranslateVal( v );
    }

    // Translates colour to a int value, return wxNOT_FOUND if no match.
    int ColToInd( const wxColour& colour ) const;
};

// -----------------------------------------------------------------------

WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR_WITH_DECL(wxColourProperty, class WXDLLIMPEXP_PG)

// -----------------------------------------------------------------------

/** @class wxCursorProperty
    @ingroup classes
    @brief Property representing wxCursor.
*/
class WXDLLIMPEXP_PG wxCursorProperty : public wxEnumProperty
{
    DECLARE_DYNAMIC_CLASS(wxCursorProperty)

    wxCursorProperty( const wxString& label= wxPG_LABEL,
                      const wxString& name= wxPG_LABEL,
                      int value = 0 );
    virtual ~wxCursorProperty();

    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()
    //virtual wxSize GetImageSize( int item ) const;
    //virtual wxPGCellRenderer* GetCellRenderer( int column ) const;
};

// -----------------------------------------------------------------------

#if wxUSE_IMAGE

WXDLLIMPEXP_PG const wxString& wxPGGetDefaultImageWildcard();

/** @class wxImageFileProperty
    @ingroup classes
    @brief Property representing image file(name).
*/
class WXDLLIMPEXP_PG wxImageFileProperty : public wxFileProperty
{
    DECLARE_DYNAMIC_CLASS(wxImageFileProperty)
public:

    wxImageFileProperty( const wxString& label= wxPG_LABEL,
                         const wxString& name = wxPG_LABEL,
                         const wxString& value = wxEmptyString);
    virtual ~wxImageFileProperty();

    virtual void OnSetValue();

    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()

protected:
    wxBitmap*   m_pBitmap; // final thumbnail area
    wxImage*    m_pImage; // intermediate thumbnail area
};

#endif

#if wxUSE_CHOICEDLG || defined(SWIG)

/** @class wxMultiChoiceProperty
    @ingroup classes
    @brief
    Property that manages a value resulting from wxMultiChoiceDialog. Value is
    array of strings. You can get value as array of choice values/indices by
    calling wxMultiChoiceProperty::GetValueAsArrayInt().

    <b>Supported special attributes:</b>
    - "UserStringMode": If > 0, allow user to manually enter strings that are not
      in the list of choices. If this value is 1, user strings
      are preferably placed in front of valid choices. If value is 2, then
      those strings will placed behind valid choices.
*/
class WXDLLIMPEXP_PG wxMultiChoiceProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxMultiChoiceProperty)
public:

    wxMultiChoiceProperty( const wxString& label,
                           const wxString& name,
                           const wxArrayString& strings,
                           const wxArrayString& value );
#ifndef SWIG
    wxMultiChoiceProperty( const wxString& label,
                           const wxString& name,
                           const wxPGChoices& choices,
                           const wxArrayString& value = wxArrayString() );

    wxMultiChoiceProperty( const wxString& label = wxPG_LABEL,
                           const wxString& name = wxPG_LABEL,
                           const wxArrayString& value = wxArrayString() );
#endif
    virtual ~wxMultiChoiceProperty();

    virtual void OnSetValue();
    virtual wxString GetValueAsString( int flags = 0 ) const;
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;
    WX_PG_DECLARE_EVENT_METHODS()

    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

    wxArrayInt GetValueAsArrayInt() const
    {
        return m_choices.GetValuesForStrings(m_value.GetArrayString());
    }

protected:

    void GenerateValueAsString();

    // Returns translation of values into string indices.
    wxArrayInt GetValueAsIndices() const;

    wxArrayString       m_valueAsStrings;  // Value as array of strings

    wxPGChoices         m_choices;

    wxString            m_display; // Cache displayed text since generating it is relatively complicated.
};

#endif // wxUSE_CHOICEDLG

// -----------------------------------------------------------------------

#if wxUSE_DATETIME || defined(SWIG)

/** @class wxDateProperty
    @ingroup classes
    @brief Property representing wxDateTime.

    <b>Supported special attributes:</b>
    - "DateFormat": Determines displayed date format.
    - "PickerStyle": Determines window style used with wxDatePickerCtrl.
       Default is wxDP_DEFAULT | wxDP_SHOWCENTURY. Using wxDP_ALLOWNONE 
       enables additional support for unspecified property value.
*/
class WXDLLIMPEXP_PG wxDateProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxDateProperty)
public:

    wxDateProperty( const wxString& label = wxPG_LABEL,
                    const wxString& name = wxPG_LABEL,
                    const wxDateTime& value = wxDateTime() );
    virtual ~wxDateProperty();

    virtual void OnSetValue();
    virtual wxString GetValueAsString( int flags = 0 ) const;
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;

    WX_PG_DECLARE_ATTRIBUTE_METHODS()

    void SetFormat( const wxString& format )
    {
        m_format = format;
    }

    const wxString& GetFormat() const
    {
        return m_format;
    }
    
    void SetDateValue( const wxDateTime& dt )
    {
        //m_valueDateTime = dt;
        m_value = dt;
    }

    wxDateTime GetDateValue() const
    {
        //return m_valueDateTime;
        return m_value;
    }

    long GetDatePickerStyle() const
    {
        return m_dpStyle;
    }

protected:
    wxString        m_format;
    long            m_dpStyle;  // DatePicker style

    static wxString ms_defaultDateFormat;
    static wxString DetermineDefaultDateFormat( bool showCentury );
};

#endif

// -----------------------------------------------------------------------

#if wxUSE_SPINBTN

//
// Implement an editor control that allows using wxSpinCtrl (actually,
// a combination of wxTextCtrl and wxSpinButton) to edit value of
// wxIntProperty and wxFloatProperty (and similar).
//
// Note that new editor classes needs to be registered before use.
// This can be accomplished using wxPGRegisterEditorClass macro, which
// is used for SpinCtrl in wxPropertyGridInterface::RegisterAdditionalEditors
// (see below). Registeration can also be performed in a constructor of a
// property that is likely to require the editor in question.
//


#include <wx/spinbutt.h>
#include "editors.h"


// NOTE: Regardless that this class inherits from a working editor, it has
//   all necessary methods to work independently. wxTextCtrl stuff is only
//   used for event handling here.
class WXDLLIMPEXP_PG wxPGSpinCtrlEditor : public wxPGTextCtrlEditor
{
    WX_PG_DECLARE_EDITOR_CLASS(wxPGSpinCtrlEditor)
public:
    virtual ~wxPGSpinCtrlEditor();

    // See below for short explanations of what these are suppposed to do.
    wxPG_DECLARE_CREATECONTROLS

    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd, wxEvent& event ) const;

private:
    mutable wxString m_tempString;
};

#endif

// -----------------------------------------------------------------------

#endif // _WX_PROPGRID_ADVPROPS_H_
