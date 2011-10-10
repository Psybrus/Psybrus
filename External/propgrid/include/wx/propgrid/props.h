/////////////////////////////////////////////////////////////////////////////
// Name:        props.h
// Purpose:     wxPropertyGrid Property Classes
// Author:      Jaakko Salli
// Modified by:
// Created:     Mar-28-2007
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_PROPS_H_
#define _WX_PROPGRID_PROPS_H_

// -----------------------------------------------------------------------

class WXDLLIMPEXP_PG wxArrayEditorDialog;

#include "editors.h"

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/valtext.h>

// -----------------------------------------------------------------------

//
// Additional property class declaration helper macros
//

//
// Property class implementation helper macros.
//

#define WX_PG_DECLARE_BASIC_TYPE_METHODS() \
    virtual wxString GetValueAsString( int argFlags = 0 ) const; \
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;

#define WX_PG_DECLARE_CHOICE_METHODS() \
    virtual bool IntToValue( wxVariant& variant, int number, int argFlags = 0 ) const; \
    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

#define WX_PG_DECLARE_EVENT_METHODS() \
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event );

#define WX_PG_DECLARE_PARENTAL_METHODS() \
    virtual void ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const; \
    virtual void RefreshChildren();

#define WX_PG_DECLARE_CUSTOM_PAINT_METHODS() \
    virtual wxSize OnMeasureImage( int item ) const; \
    virtual void OnCustomPaint( wxDC& dc, const wxRect& rect, wxPGPaintData& paintdata );

#define WX_PG_DECLARE_ATTRIBUTE_METHODS() \
    virtual bool DoSetAttribute( const wxString& name, wxVariant& value );

#define WX_PG_DECLARE_VALIDATOR_METHODS() \
    virtual wxValidator* DoGetValidator() const;

// Adds constructor function as well.
#define WX_PG_IMPLEMENT_PROPERTY_CLASS2(NAME,CLASSNAME,UPCLASS,T,T_AS_ARG,EDITOR) \
IMPLEMENT_DYNAMIC_CLASS(NAME, UPCLASS) \
WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN(NAME,T,EDITOR)

// A regular property
#define WX_PG_IMPLEMENT_PROPERTY_CLASS(NAME,UPNAME,T,T_AS_ARG,EDITOR) \
WX_PG_IMPLEMENT_PROPERTY_CLASS2(NAME,wxPG_PROPCLASS(NAME),wxPG_PROPCLASS(UPNAME),T,T_AS_ARG,EDITOR)

#define WX_PG_DECLARE_DERIVED_PROPERTY_CLASS(CLASSNAME) \
DECLARE_DYNAMIC_CLASS(CLASSNAME)

// Derived property class is one that inherits from an existing working property
// class, but assumes same value and editor type.
#define WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(NAME,UPNAME,T_AS_ARG) \
IMPLEMENT_DYNAMIC_CLASS(NAME, UPNAME)

// -----------------------------------------------------------------------

#define wxPG_NO_ESCAPE      wxPG_PROP_NO_ESCAPE     // No escape sequences
#define wxPG_ESCAPE         0                       // Escape sequences

#define WX_PG_DECLARE_STRING_PROPERTY_WITH_DECL(NAME, DECL) \
DECL NAME : public wxLongStringProperty \
{ \
    WX_PG_DECLARE_DERIVED_PROPERTY_CLASS(NAME) \
public: \
    NAME( const wxString& name = wxPG_LABEL, const wxString& label = wxPG_LABEL, const wxString& value = wxEmptyString); \
    virtual ~NAME(); \
    virtual bool OnButtonClick( wxPropertyGrid* propgrid, wxString& value ); \
    WX_PG_DECLARE_VALIDATOR_METHODS() \
};

#define WX_PG_DECLARE_STRING_PROPERTY(NAME) \
WX_PG_DECLARE_STRING_PROPERTY_WITH_DECL(NAME, class) \

#define WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR(NAME, FLAGS) \
WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(NAME,wxLongStringProperty,const wxString&) \
NAME::NAME( const wxString& name, const wxString& label, const wxString& value ) \
  : wxLongStringProperty(name,label,value) \
{ \
    m_flags |= FLAGS; \
} \
NAME::~NAME() { }

#define WX_PG_IMPLEMENT_STRING_PROPERTY(NAME, FLAGS) \
WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR(NAME,FLAGS) \
wxValidator* wxPG_PROPCLASS(NAME)::DoGetValidator () const \
{ return (wxValidator*) NULL; }

// -----------------------------------------------------------------------

#define WX_PG_DECLARE_CUSTOM_FLAGS_PROPERTY_WITH_DECL(CLASSNAME,DECL) \
DECL CLASSNAME : public wxFlagsProperty \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS(CLASSNAME) \
public: \
    CLASSNAME( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, long value = -1 ); \
    virtual ~CLASSNAME(); \
};

#define WX_PG_DECLARE_CUSTOM_FLAGS_PROPERTY(CLASSNAME) \
WX_PG_DECLARE_CUSTOM_FLAGS_PROPERTY_WITH_DECL(CLASSNAME, class)

// This will create interface for wxFlagsProperty derived class
// named CLASSNAME.
#define WX_PG_IMPLEMENT_CUSTOM_FLAGS_PROPERTY(CLASSNAME,LABELS,VALUES,DEFVAL) \
WX_PG_IMPLEMENT_PROPERTY_CLASS(CLASSNAME,wxFlagsProperty,long_##CLASSNAME,long,TextCtrl) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, long value ) \
    : wxFlagsProperty(label,name,LABELS,VALUES,value!=-1?value:DEFVAL) \
{ \
    m_flags |= wxPG_PROP_STATIC_CHOICES; \
} \
CLASSNAME::~CLASSNAME() { }


// -----------------------------------------------------------------------

#define WX_PG_DECLARE_CUSTOM_ENUM_PROPERTY_WITH_DECL(CLASSNAME, DECL) \
class CLASSNAME : public wxEnumProperty \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS(CLASSNAME) \
public: \
    CLASSNAME( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, int value = -1 ); \
    virtual ~CLASSNAME(); \
};

#define WX_PG_DECLARE_CUSTOM_ENUM_PROPERTY(CLASSNAME) \
WX_PG_DECLARE_CUSTOM_ENUM_PROPERTY_WITH_DECL(CLASSNAME, class)

#define WX_PG_IMPLEMENT_CUSTOM_ENUM_PROPERTY(CLASSNAME,LABELS,VALUES,DEFVAL) \
WX_PG_IMPLEMENT_PROPERTY_CLASS(CLASSNAME,wxEnumProperty,long_##CLASSNAME,int,Choice) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, int value ) \
    : wxEnumProperty(label,name,LABELS,VALUES,value!=-1?value:DEFVAL) \
{ \
    m_flags |= wxPG_PROP_STATIC_CHOICES; \
} \
CLASSNAME::~CLASSNAME() { }


// -----------------------------------------------------------------------

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_WITH_DECL(CLASSNAME, DECL) \
DECL CLASSNAME : public wxPG_PROPCLASS(wxSystemColourProperty) \
{ \
    DECLARE_DYNAMIC_CLASS(CLASSNAME) \
public: \
    CLASSNAME( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, \
        const wxColourPropertyValue& value = wxColourPropertyValue() ); \
    virtual ~CLASSNAME(); \
    virtual wxColour GetColour( int index ) const; \
};

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY(CLASSNAME) \
WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_WITH_DECL(CLASSNAME, class)

#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY(CLASSNAME,LABELS,VALUES,COLOURS) \
static wxPGChoices gs_##CLASSNAME##_choicesCache; \
WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(CLASSNAME,wxSystemColourProperty,const wxColourPropertyValue&) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, \
    const wxColourPropertyValue& value ) \
    : wxPG_PROPCLASS(wxSystemColourProperty)(label,name,LABELS,VALUES,&gs_##CLASSNAME##_choicesCache,value ) \
{ \
    m_flags |= wxPG_PROP_TRANSLATE_CUSTOM; \
} \
CLASSNAME::~CLASSNAME () { } \
wxColour CLASSNAME::GetColour ( int index ) const \
{ \
    return COLOURS[m_choices.GetValue(index)]; \
}

// -----------------------------------------------------------------------

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR_WITH_DECL(CLASSNAME, DECL) \
DECL CLASSNAME : public wxPG_PROPCLASS(wxSystemColourProperty) \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS(CLASSNAME) \
public: \
    CLASSNAME( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, \
        const wxColour& value = wxColour() ); \
    virtual ~CLASSNAME(); \
    virtual wxString GetValueAsString( int argFlags ) const; \
    virtual wxColour GetColour( int index ) const; \
    virtual wxVariant DoTranslateVal( wxColourPropertyValue& v ) const; \
    void Init( wxColour colour ); \
};

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(CLASSNAME) \
WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR_WITH_DECL(CLASSNAME, class)

#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR2(CLASSNAME,LABELS,VALUES,COLOURS,EDITOR) \
static wxPGChoices gs_##CLASSNAME##_choicesCache; \
WX_PG_IMPLEMENT_PROPERTY_CLASS(CLASSNAME,wxSystemColourProperty,wxColour,const wxColour&,EDITOR) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, const wxColour& value ) \
    : wxPG_PROPCLASS(wxSystemColourProperty)(label,name,LABELS,VALUES,&gs_##CLASSNAME##_choicesCache,value ) \
{ \
    if ( &value ) \
        Init( value ); \
    else \
        Init( *wxWHITE ); \
    m_flags |= wxPG_PROP_TRANSLATE_CUSTOM; \
} \
CLASSNAME::~CLASSNAME() { } \
void CLASSNAME::Init( wxColour colour ) \
{ \
    if ( !colour.Ok() ) \
        colour = *wxWHITE; \
    wxVariant variant; \
    variant << colour; \
    m_value = variant; \
    int ind = ColToInd(colour); \
    if ( ind < 0 ) \
        ind = m_choices.GetCount() - 1; \
    SetIndex( ind ); \
} \
wxString CLASSNAME::GetValueAsString( int argFlags ) const \
{ \
    const wxPGEditor* editor = GetEditorClass(); \
    if ( editor != wxPG_EDITOR(Choice) && \
         editor != wxPG_EDITOR(ChoiceAndButton) && \
         editor != wxPG_EDITOR(ComboBox) ) \
        argFlags |= wxPG_PROPERTY_SPECIFIC; \
    return wxSystemColourProperty::GetValueAsString(argFlags); \
} \
wxColour CLASSNAME::GetColour( int index ) const \
{ \
    return COLOURS[m_choices.GetValue(index)]; \
} \
wxVariant CLASSNAME::DoTranslateVal( wxColourPropertyValue& v ) const \
{ \
    wxVariant variant; \
    variant << v.m_colour; \
    return variant; \
}


#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(CLASSNAME,LABELS,VALUES,COLOURS) \
WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR2(CLASSNAME,LABELS,VALUES,COLOURS,Choice)

// -----------------------------------------------------------------------

//
// These macros helps creating DoGetValidator
#define WX_PG_DOGETVALIDATOR_ENTRY() \
    static wxValidator* s_ptr = (wxValidator*) NULL; \
    if ( s_ptr ) return s_ptr;

// Common function exit
#define WX_PG_DOGETVALIDATOR_EXIT(VALIDATOR) \
    s_ptr = VALIDATOR; \
    wxPGGlobalVars->m_arrValidators.Add( (void*) VALIDATOR ); \
    return VALIDATOR;

// -----------------------------------------------------------------------

#if wxUSE_VALIDATORS

/**
    A more comprehensive numeric validator class.
*/
class WXDLLIMPEXP_PG wxNumericPropertyValidator : public wxTextValidator
{
public:
    enum NumericType
    {
        Signed = 0,
        Unsigned,
        Float
    };

    wxNumericPropertyValidator( NumericType numericType, int base = 10 );
    virtual ~wxNumericPropertyValidator() { }
    virtual bool Validate(wxWindow* parent);
};

#endif // wxUSE_VALIDATORS

// -----------------------------------------------------------------------

#ifndef SWIG

/** @class wxPGInDialogValidator
    @ingroup classes
    @brief Creates and manages a temporary wxTextCtrl for validation purposes.
    Uses wxPropertyGrid's current editor, if available.
*/
class WXDLLIMPEXP_PG wxPGInDialogValidator
{
public:
    wxPGInDialogValidator()
    {
        m_textCtrl = NULL;
    }

    ~wxPGInDialogValidator()
    {
        if ( m_textCtrl )
            m_textCtrl->Destroy();
    }

    bool DoValidate( wxPropertyGrid* propGrid, wxValidator* validator, const wxString& value );
    
private:
    wxTextCtrl*         m_textCtrl;
};

#endif  // SWIG


// -----------------------------------------------------------------------
// Property classes
// -----------------------------------------------------------------------

#define wxPG_PROP_PASSWORD  wxPG_PROP_CLASS_SPECIFIC_2

/** @class wxStringProperty
	@ingroup classes
    @brief Basic property with string value.

    <b>Supported special attributes:</b>
    - "Password": set to 1 inorder to enable wxTE_PASSWORD on the editor.

    @remarks
    - If value "<composed>" is set, then actual value is formed (or composed) from
      values of child properties.
*/
class WXDLLIMPEXP_PG wxStringProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxStringProperty)
public:
    wxStringProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                      const wxString& value = wxEmptyString );
    virtual ~wxStringProperty();

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()

    /** This is updated so "<composed>" special value can be handled.
    */
    virtual void OnSetValue();

protected:
};

// -----------------------------------------------------------------------

#ifndef SWIG
/** Constants used with DoValidation() methods.
*/
enum
{
    /** Instead of modifying the value, show an error message.
    */
    wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE      = 0,

    /** Modify value, but stick with the limitations.
    */
    wxPG_PROPERTY_VALIDATION_SATURATE           = 1,

    /** Modify value, wrap around on overflow.
    */
    wxPG_PROPERTY_VALIDATION_WRAP               = 2
};
#endif

// -----------------------------------------------------------------------

/** @class wxIntProperty
	@ingroup classes
    @brief Basic property with integer value. Seamlessly supports 64-bit integer (wxLongLong) on overflow.

    <b>Example how to use seamless 64-bit integer support</b>

      Getting value:

      @code
          wxLongLong_t value = pg->GetPropertyValueAsLongLong();
      @endcode

         or

      @code
          wxLongLong_t value;
          wxVariant variant = property->GetValue();
          if ( variant.GetType() == wxT("wxLongLong") )
              value = wxLongLongFromVariant(variant);
          else
              value = variant.GetLong();
      @endcode

      Setting value:

       @code
          pg->SetPropertyValue(longLongVal);
      @endcode

         or

      @code
          property->SetValue(WXVARIANT(longLongVal));
      @endcode


    <b>Supported special attributes:</b>
    - "Min", "Max": Specify acceptable value range.
*/
class WXDLLIMPEXP_PG wxIntProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxIntProperty)
public:
    wxIntProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                   long value = 0 );
    virtual ~wxIntProperty();

    wxIntProperty( const wxString& label, const wxString& name, const wxLongLong& value );
    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    virtual bool ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const;
    virtual bool IntToValue( wxVariant& variant, int number, int argFlags = 0 ) const;
    static wxValidator* GetClassValidator();
    virtual wxValidator* DoGetValidator() const;

    /** Validation helper.
    */
    static bool DoValidation( const wxPGProperty* property, wxLongLong_t& value, wxPGValidationInfo* pValidationInfo, int mode = wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE );

protected:
};

// -----------------------------------------------------------------------

/** @class wxUIntProperty
	@ingroup classes
    @brief Basic property with unsigned integer value. Seamlessly supports 64-bit integer (wxULongLong) on overflow.

    <b>Supported special attributes:</b>
    - "Min", "Max": Specify acceptable value range.
    - "Base": Define base. Valid constants are wxPG_BASE_OCT, wxPG_BASE_DEC, wxPG_BASE_HEX and wxPG_BASE_HEXL
    (lowercase characters). Arbitrary bases are <b>not</b> supported.
    - "Prefix": Possible values are wxPG_PREFIX_NONE, wxPG_PREFIX_0x, and wxPG_PREFIX_DOLLAR_SIGN.
    Only wxPG_PREFIX_NONE works with Decimal and Octal numbers.

    @remarks
    - For example how to use seamless 64-bit integer support, see wxIntProperty documentation (just
      use wxULongLong instead of wxLongLong).
*/
class WXDLLIMPEXP_PG wxUIntProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxUIntProperty)
public:
    wxUIntProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                   unsigned long value = 0 );
    virtual ~wxUIntProperty();
    wxUIntProperty( const wxString& label, const wxString& name, const wxULongLong& value );
    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()
    virtual bool ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const;
    virtual bool IntToValue( wxVariant& variant, int number, int argFlags = 0 ) const;
    virtual wxValidator* DoGetValidator() const;
protected:
    wxByte      m_base;
    wxByte      m_realBase; // translated to 8,16,etc.
    wxByte      m_prefix;
private:
    void Init();
};

// -----------------------------------------------------------------------

/** @class wxFloatProperty
	@ingroup classes
    @brief Basic property with double-precision floating point value.

    <b>Supported special attributes:</b>
    - "Precision": Sets the (max) precision used when floating point value is rendered as text.
    The default -1 means infinite precision.
*/
class WXDLLIMPEXP_PG wxFloatProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxFloatProperty)
public:
    wxFloatProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                     double value = 0.0 );
    virtual ~wxFloatProperty();

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()
    virtual bool ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const;

    /** Validation helper.
    */
    static bool DoValidation( const wxPGProperty* property, double& value, wxPGValidationInfo* pValidationInfo, int mode = wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE );

    static wxValidator* GetClassValidator();

protected:
    int m_precision;
    virtual wxValidator* DoGetValidator () const;
};

// -----------------------------------------------------------------------

/** @class wxBoolProperty
	@ingroup classes
    @brief Basic property with boolean value.

    <b>Supported special attributes:</b>
    - "UseCheckbox": Set to 1 to use check box editor instead of combo box.
    - "UseDClickCycling": Set to 1 to cycle combo box instead showing the list.
*/
class WXDLLIMPEXP_PG wxBoolProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxBoolProperty)
public:
    wxBoolProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                    bool value = false );
    virtual ~wxBoolProperty();

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_CHOICE_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()
};

// -----------------------------------------------------------------------

/** @class wxBaseEnumProperty
    @ingroup classes
    @brief
    Derive dynamic custom properties with choices from this class.

    @remarks
    - Updating private index is important. You can do this either by calling SetIndex()
      in IntToValue, and then letting wxBaseEnumProperty::OnSetValue be called (by
      not implementing it, or by calling super class function in it) -OR-
      you can just call SetIndex in OnSetValue.
*/
class WXDLLIMPEXP_PG wxBaseEnumProperty : public wxPGProperty
{
public:
    wxBaseEnumProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL );

    virtual void OnSetValue();
    virtual wxString GetValueAsString( int argFlags ) const;
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;
    virtual bool ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const;

    // If wxPG_FULL_VALUE is not set in flags, then the value is interpreted
    // as index to choices list. Otherwise, it is actual value.
    virtual bool IntToValue( wxVariant& variant, int number, int argFlags = 0 ) const;

    //
    // Additional virtuals

    // This must be overridden to have non-index based value
    virtual int GetIndexForValue( int value ) const;

    // This returns string and value for index
    // Returns NULL if beyond last item
    // pvalue is never NULL - always set it.
    virtual const wxString* GetEntry( size_t index, int* pvalue ) const = 0;

    int GetValueForIndex( size_t index ) const
    {
        int v;
        GetEntry( index, &v );
        return v;
    }

    virtual void OnValidationFailure( wxVariant& pendingValue );

protected:

    int GetIndex() const;
    void SetIndex( int index );

    bool ValueFromString_( wxVariant& value, const wxString& text, int argFlags ) const;
    bool ValueFromInt_( wxVariant& value, int intVal, int argFlags ) const;

    static void ResetNextIndex() { ms_nextIndex = -2; }

private:
    // This is private so that classes are guaranteed to use GetIndex
    // for up-to-date index value.
    int                     m_index;

    // Relies on ValidateValue being called always before OnSetValue
    static int              ms_nextIndex;
};

// -----------------------------------------------------------------------

// If set, then selection of choices is static and should not be
// changed (i.e. returns NULL in GetPropertyChoices).
#define wxPG_PROP_STATIC_CHOICES    wxPG_PROP_CLASS_SPECIFIC_1

/** @class wxEnumProperty
    @ingroup classes
    @brief
    You can derive custom properties with choices from this class. See wxBaseEnumProperty
    for remarks.
*/
class WXDLLIMPEXP_PG wxEnumProperty : public wxBaseEnumProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxEnumProperty)
public:

#ifndef SWIG
    wxEnumProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
        const wxChar** labels = NULL, const long* values = NULL, int value = 0 );
    wxEnumProperty( const wxString& label, const wxString& name, 
        wxPGChoices& choices, int value = 0 );

    // Special constructor for caching choices (used by derived class)
    wxEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values, wxPGChoices* choicesCache, int value = 0 );

    wxEnumProperty( const wxString& label, const wxString& name,
        const wxArrayString& labels, const wxArrayInt& values = wxArrayInt(),
        int value = 0 );
#else
    wxEnumProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
        const wxArrayString& labels = wxArrayString(), const wxArrayInt& values = wxArrayInt(),
        int value = 0 );
#endif

    virtual ~wxEnumProperty();

    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );
    virtual int GetIndexForValue( int value ) const;
    virtual const wxString* GetEntry( size_t index, int* pvalue ) const;

    size_t GetItemCount() const { return m_choices.GetCount(); }
    const wxPGChoices& GetChoices() const { return m_choices; }

protected:
    wxPGChoices             m_choices;
};

// -----------------------------------------------------------------------

/** @class wxEditEnumProperty
    @ingroup classes
    @brief
    wxEnumProperty with wxString value and writable combo box editor.

    @remarks
    Uses int value, similar to wxEnumProperty, unless text entered by user is
    is not in choices (in which case string value is used).
*/
class WXDLLIMPEXP_PG wxEditEnumProperty : public wxEnumProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxEditEnumProperty)
public:

    wxEditEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values, const wxString& value );
    wxEditEnumProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
        const wxArrayString& labels = wxArrayString(), const wxArrayInt& values = wxArrayInt(),
        const wxString& value = wxEmptyString );
    wxEditEnumProperty( const wxString& label, const wxString& name,
        wxPGChoices& choices, const wxString& value = wxEmptyString );

    // Special constructor for caching choices (used by derived class)
    wxEditEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values, wxPGChoices* choicesCache, const wxString& value );

    virtual ~wxEditEnumProperty();

protected:
};

// -----------------------------------------------------------------------

/** @class wxFlagsProperty
    @ingroup classes
    @brief
    Represents a bit set that fits in a long integer. wxBoolProperty sub-properties
    are created for editing individual bits. Textctrl is created to manually edit
    the flags as a text; a continous sequence of spaces, commas and semicolons
    is considered as a flag id separator.
    <b>Note:</b> When changing "choices" (ie. flag labels) of wxFlagsProperty, you
    will need to use SetPropertyChoices - otherwise they will not get updated properly.
*/
class WXDLLIMPEXP_PG wxFlagsProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxFlagsProperty)
public:

#ifndef SWIG
    wxFlagsProperty( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values = NULL, long value = 0 );
    wxFlagsProperty( const wxString& label, const wxString& name, 
        wxPGChoices& choices, long value = 0 );
#endif
    wxFlagsProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
        const wxArrayString& labels = wxArrayString(),
        const wxArrayInt& values = wxArrayInt(), int value = 0 );
    virtual ~wxFlagsProperty ();

    virtual void OnSetValue();
    virtual wxString GetValueAsString( int argFlags ) const;
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int flags ) const;
    virtual void ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const;
    virtual void RefreshChildren();

    // this is necessary for conveying m_choices
    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

    // helpers
    size_t GetItemCount() const { return m_choices.GetCount(); }
    const wxString& GetLabel( size_t ind ) const { return m_choices.GetLabel(ind); }

protected:
    wxPGChoices             m_choices;

    // Used to detect if choices have been changed
    wxPGChoicesData*        m_oldChoicesData;

    // Needed to properly mark changed sub-properties
    long                    m_oldValue;

    // Converts string id to a relevant bit.
    long IdToBit( const wxString& id ) const;

    // Creates children and sets value.
    void Init();
};

// -----------------------------------------------------------------------

/** @class wxPGFileDialogAdapter
    @ingroup classes
*/
class WXDLLIMPEXP_PG wxPGFileDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    virtual bool DoShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property );
};

// -----------------------------------------------------------------------

#include <wx/filename.h>

// Indicates first bit useable by derived properties.
#define wxPG_PROP_SHOW_FULL_FILENAME  wxPG_PROP_CLASS_SPECIFIC_1

/** @class wxFileProperty
    @ingroup classes
    @brief
    Like wxLongStringProperty, but the button triggers file selector instead.

    <b>Supported special attributes:</b>
    - "Wildcard": Sets wildcard (see wxFileDialog for format details), "All files..."
      is default.
    - "ShowFullPath": Default 1. When 0, only the file name is shown (i.e. drive
      and directory are hidden).
    - "ShowRelativePath": If set, then the filename is shown relative to the given
      path string.
    - "InitialPath": Sets the initial path of where to look for files.
    - "DialogTitle": Sets a specific title for the dir dialog.
*/
class WXDLLIMPEXP_PG wxFileProperty : public wxPGProperty
{
    friend class wxPGFileDialogAdapter;
    WX_PG_DECLARE_PROPERTY_CLASS(wxFileProperty)
public:

    wxFileProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
        const wxString& value = wxEmptyString );
    virtual ~wxFileProperty ();

    virtual void OnSetValue();
    virtual wxString GetValueAsString( int argFlags ) const;
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;
    //virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* wnd_primary, wxEvent& event );
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const;

    WX_PG_DECLARE_ATTRIBUTE_METHODS()

    static wxValidator* GetClassValidator();
    virtual wxValidator* DoGetValidator() const;

protected:
    wxString    m_wildcard;
    wxString    m_basePath; // If set, then show path relative to it
    wxString    m_initialPath; // If set, start the file dialog here
    wxString    m_dlgTitle; // If set, used as title for file dialog
    wxFileName  m_filename; // used as primary storage
    int         m_indFilter; // index to the selected filter
};

// -----------------------------------------------------------------------

#define wxPG_PROP_NO_ESCAPE     wxPG_PROP_CLASS_SPECIFIC_1


/** @class wxPGLongStringDialogAdapter
    @ingroup classes
*/
class WXDLLIMPEXP_PG wxPGLongStringDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    virtual bool DoShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property );
};


/** @class wxLongStringProperty
    @ingroup classes
    @brief
    Like wxStringProperty, but has a button that triggers a small text
    editor dialog.
*/
class WXDLLIMPEXP_PG wxLongStringProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxLongStringProperty)
public:

    wxLongStringProperty( const wxString& label = wxPG_LABEL,
                          const wxString& name = wxPG_LABEL,
                          const wxString& value = wxEmptyString );
    virtual ~wxLongStringProperty();

    virtual wxString GetValueAsString( int argFlags = 0 ) const;
    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;

    WX_PG_DECLARE_EVENT_METHODS()

    //  Shows string editor dialog. Value to be edited should be read from value, and
    //  if dialog is not cancelled, it should be stored back and true should be returned
    //  if that was the case.
    virtual bool OnButtonClick( wxPropertyGrid* propgrid, wxString& value );

    static bool DisplayEditorDialog( wxPGProperty* prop, wxPropertyGrid* propGrid, wxString& value );

protected:
};

// -----------------------------------------------------------------------

/** @class wxDirProperty
    @ingroup classes
    @brief
    Like wxLongStringProperty, but the button triggers dir selector instead.

    <b>Supported special attributes:</b>
    - "DialogMessage": Sets specific message in the dir selector.
*/
class WXDLLIMPEXP_PG wxDirProperty : public wxLongStringProperty
{
#ifndef SWIG
    DECLARE_DYNAMIC_CLASS(wxDirProperty)
#endif
public:
    wxDirProperty( const wxString& name = wxPG_LABEL, const wxString& label = wxPG_LABEL,
                   const wxString& value = wxEmptyString );
    virtual ~wxDirProperty();

    WX_PG_DECLARE_ATTRIBUTE_METHODS()
    WX_PG_DECLARE_VALIDATOR_METHODS()

    virtual bool OnButtonClick ( wxPropertyGrid* propGrid, wxString& value );

protected:
    wxString    m_dlgMessage;
};


// -----------------------------------------------------------------------

// wxBoolProperty specific flags
#define wxPG_PROP_USE_CHECKBOX      wxPG_PROP_CLASS_SPECIFIC_1
// DCC = Double Click Cycles
#define wxPG_PROP_USE_DCC           wxPG_PROP_CLASS_SPECIFIC_2


// -----------------------------------------------------------------------

/** @class wxArrayStringProperty
    @ingroup classes
    @brief
    Property that manages a list of strings.
*/
class WXDLLIMPEXP_PG wxArrayStringProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxArrayStringProperty)
public:

    wxArrayStringProperty( const wxString& label = wxPG_LABEL,
                           const wxString& name = wxPG_LABEL,
                           const wxArrayString& value = wxArrayString() );
    virtual ~wxArrayStringProperty();

    virtual void OnSetValue();
    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_EVENT_METHODS()

    virtual void GenerateValueAsString();

    //  Shows string editor dialog. Value to be edited should be read from value, and
    //  if dialog is not cancelled, it should be stored back and true should be returned
    //  if that was the case.
    virtual bool OnCustomStringEdit( wxWindow* parent, wxString& value );

    // Helper.
    virtual bool OnButtonClick( wxPropertyGrid* propgrid,
                                wxWindow* primary,
                                const wxChar* cbt );

#ifndef SWIG
    // Creates wxArrayEditorDialog for string editing. Called in OnButtonClick.
    virtual wxArrayEditorDialog* CreateEditorDialog();
#endif

protected:
    wxString        m_display; // Cache for displayed text.
};

// -----------------------------------------------------------------------

#define WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR_WITH_DECL(PROPNAME, DECL) \
DECL wxPG_PROPCLASS(PROPNAME) : public wxPG_PROPCLASS(wxArrayStringProperty) \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS(wxPG_PROPCLASS(PROPNAME)) \
public: \
    wxPG_PROPCLASS(PROPNAME)( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, \
                              const wxArrayString& value = wxArrayString() ); \
    ~wxPG_PROPCLASS(PROPNAME)(); \
    virtual void GenerateValueAsString(); \
    virtual bool StringToValue( wxVariant& value, const wxString& text, int = 0 ) const; \
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event ); \
    virtual bool OnCustomStringEdit( wxWindow* parent, wxString& value ); \
    WX_PG_DECLARE_VALIDATOR_METHODS() \
};

#define WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAM) \
WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAM, class)

#define WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAME,DELIMCHAR,CUSTBUTTXT) \
WX_PG_IMPLEMENT_PROPERTY_CLASS(PROPNAME,wxArrayStringProperty,wxArrayString,const wxArrayString&,TextCtrlAndButton) \
wxPG_PROPCLASS(PROPNAME)::wxPG_PROPCLASS(PROPNAME)( const wxString& label, const wxString& name, const wxArrayString& value ) \
    : wxPG_PROPCLASS(wxArrayStringProperty)(label,name,value) \
{ \
    wxPG_PROPCLASS(PROPNAME)::GenerateValueAsString(); \
} \
wxPG_PROPCLASS(PROPNAME)::~wxPG_PROPCLASS(PROPNAME)() { } \
void wxPG_PROPCLASS(PROPNAME)::GenerateValueAsString() \
{ \
    wxChar delimChar = DELIMCHAR; \
    if ( delimChar == wxT('"') ) \
        wxPG_PROPCLASS(wxArrayStringProperty)::GenerateValueAsString(); \
    else \
        wxPropertyGrid::ArrayStringToString(m_display,m_value.GetArrayString(),0,DELIMCHAR,0); \
} \
bool wxPG_PROPCLASS(PROPNAME)::StringToValue( wxVariant& variant, const wxString& text, int ) const \
{ \
    wxChar delimChar = DELIMCHAR; \
    if ( delimChar == wxT('"') ) \
        return wxPG_PROPCLASS(wxArrayStringProperty)::StringToValue(variant, text, 0); \
    \
    wxArrayString arr; \
    WX_PG_TOKENIZER1_BEGIN(text,DELIMCHAR) \
        arr.Add( token ); \
    WX_PG_TOKENIZER1_END() \
    variant = arr; \
    return true; \
} \
bool wxPG_PROPCLASS(PROPNAME)::OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event ) \
{ \
    if ( event.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED ) \
        return OnButtonClick(propgrid,primary,(const wxChar*) CUSTBUTTXT); \
    return false; \
}

#define WX_PG_DECLARE_ARRAYSTRING_PROPERTY(PROPNAME) \
WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAME)

#define WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_DECL(PROPNAME, DECL) \
WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR_WITH_DECL(PROPNAME, DECL)

#define WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY(PROPNAME,DELIMCHAR,CUSTBUTTXT) \
WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAME,DELIMCHAR,CUSTBUTTXT) \
wxValidator* wxPG_PROPCLASS(PROPNAME)::DoGetValidator () const \
{ return (wxValidator*) NULL; }


// -----------------------------------------------------------------------
// wxArrayEditorDialog
// -----------------------------------------------------------------------

#define wxAEDIALOG_STYLE \
    (wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxOK | wxCANCEL | wxCENTRE)

class WXDLLIMPEXP_PG wxArrayEditorDialog : public wxDialog
{
public:
    wxArrayEditorDialog();
    virtual ~wxArrayEditorDialog() { }

    void Init();

    wxArrayEditorDialog( wxWindow *parent,
                         const wxString& message,
                         const wxString& caption,
                         long style = wxAEDIALOG_STYLE,
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& sz = wxDefaultSize );

    bool Create( wxWindow *parent,
                 const wxString& message,
                 const wxString& caption,
                 long style = wxAEDIALOG_STYLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& sz = wxDefaultSize );

    /** Set value modified by dialog.
    */
    virtual void SetDialogValue( const wxVariant& WXUNUSED(value) )
    {
        wxFAIL_MSG(wxT("re-implement this member function in derived class"));
    }

    /** Return value modified by dialog.
    */
    virtual wxVariant GetDialogValue() const
    {
        wxFAIL_MSG(wxT("re-implement this member function in derived class"));
        return wxVariant();
    }

    /** Override to return wxValidator to be used with the wxTextCtrl
        in dialog. Note that the validator is used in the standard 
        wx way, ie. it immediately prevents user from entering invalid
        input.

        @remarks
        Dialog frees the validator.
    */
    virtual wxValidator* GetTextCtrlValidator() const
    {
        return (wxValidator*) NULL;
    }

    // Returns true if array was actually modified
    bool IsModified() const { return m_modified; }

    //const wxArrayString& GetStrings() const { return m_array; }

    // implementation from now on
    void OnUpdateClick(wxCommandEvent& event);
    void OnAddClick(wxCommandEvent& event);
    void OnDeleteClick(wxCommandEvent& event);
    void OnListBoxClick(wxCommandEvent& event);
    void OnUpClick(wxCommandEvent& event);
    void OnDownClick(wxCommandEvent& event);
    //void OnCustomEditClick(wxCommandEvent& event);
    void OnIdle(wxIdleEvent& event);

#if defined(__WXPYTHON__) && !defined(SWIG)
    // This is the python object that contains and owns the C++ representation.
    PyObject*                   m_scriptObject;
#endif
protected:
    wxTextCtrl*     m_edValue;
    wxListBox*      m_lbStrings;

    wxButton*       m_butAdd;       // Button pointers
    wxButton*       m_butCustom;    // required for disabling/enabling changing.
    wxButton*       m_butUpdate;
    wxButton*       m_butRemove;
    wxButton*       m_butUp;
    wxButton*       m_butDown;

    //wxArrayString   m_array;

    const wxChar*   m_custBtText;
    //wxArrayStringPropertyClass*     m_pCallingClass;

    bool            m_modified;

    unsigned char   m_curFocus;

    // These must be overridden - must return true on success.
    virtual wxString ArrayGet( size_t index ) = 0;
    virtual size_t ArrayGetCount() = 0;
    virtual bool ArrayInsert( const wxString& str, int index ) = 0;
    virtual bool ArraySet( size_t index, const wxString& str ) = 0;
    virtual void ArrayRemoveAt( int index ) = 0;
    virtual void ArraySwap( size_t first, size_t second ) = 0;

private:
#ifndef SWIG
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxArrayEditorDialog)
    DECLARE_EVENT_TABLE()
#endif
};

// -----------------------------------------------------------------------
// wxPGArrayStringEditorDialog
// -----------------------------------------------------------------------

class WXDLLIMPEXP_PG wxPGArrayStringEditorDialog : public wxArrayEditorDialog
{
public:
    wxPGArrayStringEditorDialog();
    virtual ~wxPGArrayStringEditorDialog() { }

    void Init();

    virtual void SetDialogValue( const wxVariant& value )
    {
        m_array = value.GetArrayString();
    }

    virtual wxVariant GetDialogValue() const
    {
        return m_array;
    }

    void SetCustomButton( const wxChar* custBtText, wxArrayStringProperty* pcc )
    {
        m_custBtText = custBtText;
        m_pCallingClass = pcc;
    }

    void OnCustomEditClick(wxCommandEvent& event);

protected:
    wxArrayString   m_array;

    wxArrayStringProperty*     m_pCallingClass;

    virtual wxString ArrayGet( size_t index );
    virtual size_t ArrayGetCount();
    virtual bool ArrayInsert( const wxString& str, int index );
    virtual bool ArraySet( size_t index, const wxString& str );
    virtual void ArrayRemoveAt( int index );
    virtual void ArraySwap( size_t first, size_t second );

private:
#ifndef SWIG
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxPGArrayStringEditorDialog)
    DECLARE_EVENT_TABLE()
#endif
};

// -----------------------------------------------------------------------

// Since 1.3.1, separate wxParentProperty is no longer needed
#if wxPG_COMPATIBILITY_1_2_0
    typedef wxStringProperty wxParentProperty;
#endif

// -----------------------------------------------------------------------

/** @class wxCustomProperty
    @ingroup classes
    @brief This is a somewhat inefficient but versatile property class.

   Base class offers the following:
     - By default has string value type.
     - Has capacity to have choices.
     - Can have custom-paint bitmap.
*/
class WXDLLIMPEXP_PG wxCustomProperty : public wxPGProperty
{
#ifndef SWIG
    WX_PG_DECLARE_PROPERTY_CLASS(wxCustomProperty)
#endif
public:

    wxCustomProperty( const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL );
    virtual ~wxCustomProperty();

    virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;
    virtual wxString GetValueAsString( int argFlags ) const;

#ifdef wxPG_COMPATIBILITY_1_0_0
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event );
#endif
    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()

    virtual bool IntToValue( wxVariant& variant, int number, int argFlags ) const;
    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

    virtual void ChildChanged( wxVariant& a1, int a2, wxVariant& a3 ) const
    {
        wxUnusedVar(a1);
        wxUnusedVar(a2);
        wxUnusedVar(a3);
    }

    WX_PG_DECLARE_ATTRIBUTE_METHODS()

protected:
    wxPGChoices             m_choices;
#ifdef wxPG_COMPATIBILITY_1_0_0
    wxPropertyGridCallback  m_callback;
#endif
    wxPGPaintCallback       m_paintCallback;
};

// -----------------------------------------------------------------------

// Helper functions

WXDLLIMPEXP_PG bool wxPGStringToLongLong(const wxString s, wxLongLong_t* val, int base);
WXDLLIMPEXP_PG bool wxPGStringToULongLong(const wxString s, wxULongLong_t* val, int base);

// -----------------------------------------------------------------------

#endif // _WX_PROPGRID_PROPS_H_
