/////////////////////////////////////////////////////////////////////////////
// Name:        props.cpp
// Purpose:     Basic Property Classes
// Author:      Jaakko Salli
// Modified by:
// Created:     May-14-2004
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
    #include "wx/combobox.h"
    #include "wx/layout.h"
    #include "wx/sizer.h"
    #include "wx/textdlg.h"
    #include "wx/filedlg.h"
    #include "wx/statusbr.h"
    #include "wx/intl.h"
#endif

#include <wx/filename.h>

#include <wx/propgrid/propgrid.h>

#include <wx/propgrid/propdev.h>


#define wxPG_CUSTOM_IMAGE_WIDTH     20 // for wxColourProperty etc.


// -----------------------------------------------------------------------
// wxStringProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxStringProperty,wxPGProperty,
                               wxString,const wxString&,TextCtrl)

wxStringProperty::wxStringProperty( const wxString& label,
                                    const wxString& name,
                                    const wxString& value )
    : wxPGProperty(label,name)
{
    SetValue(value);
}

void wxStringProperty::OnSetValue()
{
    if ( !m_value.IsNull() && m_value.GetString() == wxT("<composed>") )
        SetFlag(wxPG_PROP_COMPOSED_VALUE);

    if ( HasFlag(wxPG_PROP_COMPOSED_VALUE) )
    {
        wxString s;
        GenerateComposedValue(s, 0);
        m_value = s;
    }
}

wxStringProperty::~wxStringProperty() { }

wxString wxStringProperty::GetValueAsString( int argFlags ) const
{
    wxString s = m_value.GetString();

    if ( GetChildCount() && HasFlag(wxPG_PROP_COMPOSED_VALUE) )
    {
        // Value stored in m_value is non-editable, non-full value
        if ( (argFlags & wxPG_FULL_VALUE) || (argFlags & wxPG_EDITABLE_VALUE) )
            GenerateComposedValue(s, argFlags);

        return s;
    }

    // If string is password and value is for visual purposes,
    // then return asterisks instead the actual string.
    if ( (m_flags & wxPG_PROP_PASSWORD) && !(argFlags & (wxPG_FULL_VALUE|wxPG_EDITABLE_VALUE)) )
        return wxString(wxChar('*'), s.Length());

    return s;
}

bool wxStringProperty::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    if ( GetChildCount() && HasFlag(wxPG_PROP_COMPOSED_VALUE) )
        return wxPGProperty::StringToValue(variant, text, argFlags);

    if ( m_value.IsNull() || m_value.GetString() != text )
    {
        variant = text;
        return true;
    }

    return false;
}

bool wxStringProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    if ( name == wxPG_STRING_PASSWORD )
    {
        m_flags &= ~(wxPG_PROP_PASSWORD);
        if ( wxPGVariantToInt(value) ) m_flags |= wxPG_PROP_PASSWORD;
        RecreateEditor();
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------

#ifndef wxHAS_STRTOLL
    #if wxCHECK_VERSION(2,9,0)
        #define wxHAS_STRTOLL
    #endif
#endif

#ifndef wxINT64_MAX
    #ifdef LLONG_MAX
        #define wxINT64_MAX LLONG_MAX
        #define wxINT64_MIN LLONG_MIN
    #else
        #define wxINT64_MAX wxLL(9223372036854775807)
        #define wxINT64_MIN (wxLL(-9223372036854775807)-1)
    #endif
#endif

#ifndef wxUINT64_MAX
    #define wxUINT64_MAX wxULL(0xFFFFFFFFFFFFFFFF)
    #define wxUINT64_MIN wxULL(0)
#endif

#ifndef wxHAS_STRTOLL

#ifndef wxSET_ERRNO
    #define wxSET_ERRNO(A)  errno = A
#endif

#include <errno.h>

static wxULongLong_t wxStrtoullBase(const wxChar* nptr, wxChar** endptr, int base, wxChar* sign)
{
    wxULongLong_t sum = 0;
    wxString wxstr(nptr);
    wxString::const_iterator i = wxstr.begin();
    wxString::const_iterator end = wxstr.end();

    // Skip spaces
    while ( i != end && wxIsspace(*i) ) i++;

    // Starts with sign?
    *sign = wxT(' ');
    if ( i != end )
    {
        wxChar c = *i;
        if ( c == wxT('+') || c == wxT('-') )
        {
            *sign = c;
            i++;
        }
    }

    // Starts with 0x?
    if ( i != end && *i == wxT('0') )
    {
        i++;
        if ( i != end )
        {
            if ( *i == wxT('x') && (base == 16 || base == 0) )
            {
                base = 16;
                i++;
            }
            else
            {
                if ( endptr )
                    *endptr = (wxChar*) nptr;
                wxSET_ERRNO(EINVAL);
                return sum;
            }
        }
        else
            i--;
    }

    if ( base == 0 )
        base = 10;

    for ( ; i != end; i++ )
    {
        unsigned int n;

        wxChar c = *i;
        if ( c >= wxT('0') )
        {
            if ( c <= wxT('9') )
                n = c - wxT('0');
            else
                n = wxTolower(c) - wxT('a') + 10;
        }
        else
            break;

        if ( n >= (unsigned int)base )
            // Invalid character (for this base)
            break;

        wxULongLong_t prevsum = sum;
        sum = (sum * base) + n;

        if ( sum < prevsum )
        {
            wxSET_ERRNO(ERANGE);
            break;
        }
    }

    if ( endptr )
    {
        *endptr = (wxChar*)(nptr + (i - wxstr.begin()));
    }

    return sum;
}

wxULongLong_t wxStrtoull(const wxChar* nptr, wxChar** endptr, int base)
{
    wxChar sign;
    wxULongLong_t uval = wxStrtoullBase(nptr, endptr, base, &sign);

    if ( sign == wxT('-') )
    {
        wxSET_ERRNO(ERANGE);
        uval = 0;
    }

    return uval;
}

wxLongLong_t wxStrtoll(const wxChar* nptr, wxChar** endptr, int base)
{
    wxChar sign;
    wxULongLong_t uval = wxStrtoullBase(nptr, endptr, base, &sign);
    wxLongLong_t val = 0;

    if ( sign == wxT('-') )
    {
        if ( uval <= wxULL(wxINT64_MAX+1) )
        {
            if ( uval == wxULL(wxINT64_MAX+1))
                val = -((wxLongLong_t)wxINT64_MAX) - 1;
            else
                val = -((wxLongLong_t)uval);
        }
        else
        {
            wxSET_ERRNO(ERANGE);
        }
    }
    else if ( uval <= wxINT64_MAX )
    {
        val = uval;
    }
    else
    {
        wxSET_ERRNO(ERANGE);
    }

    return val;
}

bool wxPGStringToLongLong(const wxString s, wxLongLong_t* val, int base)
{
    wxChar* endptr;
    errno = 0;
    *val = wxStrtoll(s.c_str(), &endptr, base);
    return (errno == 0 && *endptr == wxT('\0'));
}

bool wxPGStringToULongLong(const wxString s, wxULongLong_t* val, int base)
{
    wxChar* endptr;
    errno = 0;
    *val = wxStrtoull(s.c_str(), &endptr, base);
    return (errno == 0 && *endptr == wxT('\0'));
}

#else

bool wxPGStringToLongLong(const wxString s, wxLongLong_t* val, int base)
{
    return s.ToLongLong(val, base);
}

bool wxPGStringToULongLong(const wxString s, wxULongLong_t* val, int base)
{
    return s.ToULongLong(val, base);
}

#endif // !wxHAS_STRTOLL

// -----------------------------------------------------------------------
// wxNumericPropertyValidator
// -----------------------------------------------------------------------

#if wxUSE_VALIDATORS

wxNumericPropertyValidator::
    wxNumericPropertyValidator( NumericType numericType, int base )
    : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
    wxArrayString arr;
    arr.Add(wxT("0"));
    arr.Add(wxT("1"));
    arr.Add(wxT("2"));
    arr.Add(wxT("3"));
    arr.Add(wxT("4"));
    arr.Add(wxT("5"));
    arr.Add(wxT("6"));
    arr.Add(wxT("7"));

    if ( base >= 10 )
    {
        arr.Add(wxT("8"));
        arr.Add(wxT("9"));
        if ( base >= 16 )
        {
            arr.Add(wxT("a")); arr.Add(wxT("A"));
            arr.Add(wxT("b")); arr.Add(wxT("B"));
            arr.Add(wxT("c")); arr.Add(wxT("C"));
            arr.Add(wxT("d")); arr.Add(wxT("D"));
            arr.Add(wxT("e")); arr.Add(wxT("E"));
            arr.Add(wxT("f")); arr.Add(wxT("F"));
        }
    }

    if ( numericType == Signed )
    {
        arr.Add(wxT("+"));
        arr.Add(wxT("-"));
    }
    else if ( numericType == Float )
    {
        arr.Add(wxT("+"));
        arr.Add(wxT("-"));
        arr.Add(wxT("e"));

        // Use locale-specific decimal point
        arr.Add(wxString::Format(wxT("%g"), 1.1)[1]);
    }

    SetIncludes(arr);
}

bool wxNumericPropertyValidator::Validate(wxWindow* parent)
{
    if ( !wxTextValidator::Validate(parent) )
        return false;

    wxWindow* wnd = GetWindow();
    if ( !wnd->IsKindOf(CLASSINFO(wxTextCtrl)) )
        return true;

    // Do not allow zero-length string
    wxTextCtrl* tc = static_cast<wxTextCtrl*>(wnd);
    wxString text = tc->GetValue();

    if ( !text.length() )
        return false;

    return true;
}

#endif // wxUSE_VALIDATORS

// -----------------------------------------------------------------------
// wxIntProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxIntProperty,wxPGProperty,
                               long,long,TextCtrl)

wxIntProperty::wxIntProperty( const wxString& label, const wxString& name,
    long value ) : wxPGProperty(label,name)
{
    SetValue(value);
}

wxIntProperty::wxIntProperty( const wxString& label, const wxString& name,
    const wxLongLong& value ) : wxPGProperty(label,name)
{
    SetValue(wxLongLongToVariant(value));
}

wxIntProperty::~wxIntProperty() { }

wxString wxIntProperty::GetValueAsString( int ) const
{
    if ( wxPGIsVariantType(m_value, long) )
        return wxString::Format(wxT("%li"),m_value.GetLong());

	wxLongLong* ll = &wxLongLongFromVariant(m_value);
	if ( ll )
		return ll->ToString();

	return wxEmptyString;
}

bool wxIntProperty::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    wxString s;
    long value32;

    if ( text.length() == 0 )
    {
        variant.MakeNull();
        return true;
    }

    // We know it is a number, but let's still check
    // the return value.
    if ( text.IsNumber() )
    {
        // Remove leading zeroes, so that the number is not interpreted as octal
        wxString::const_iterator i = text.begin();
        wxString::const_iterator iMax = text.end() - 1;  // Let's allow one, last zero though

        int firstNonZeroPos = 0;

        for ( ; i != iMax; i++ )
        {
            wxChar c = *i;
            if ( c != wxT('0') && c != wxT(' ') )
                break;
            firstNonZeroPos++;
        }

        wxString useText = text.substr(firstNonZeroPos, text.length() - firstNonZeroPos);

        bool isPrevLong = wxPGIsVariantType(variant, long);

        wxLongLong_t value64 = 0;

        if ( wxPGStringToLongLong(useText, &value64, 10) &&
             ( value64 >= INT_MAX || value64 <= INT_MIN )
           )
        {
            wxLongLong* _m_value64 = &wxLongLongFromVariant(m_value);
            if ( isPrevLong || !_m_value64 || _m_value64->GetValue() != value64 )
            {
                variant = wxLongLongToVariant(value64);
                return true;
            }
        }

        if ( useText.ToLong( &value32, 0 ) )
        {
            if ( !isPrevLong || m_value.IsNull() || m_value.GetLong() != value32 )
            {
                variant = value32;
                return true;
            }
        }
    }
    else if ( argFlags & wxPG_REPORT_ERROR )
    {
    }
    return false;
}

bool wxIntProperty::IntToValue( wxVariant& variant, int value, int WXUNUSED(argFlags) ) const
{
    if ( !wxPGIsVariantType(variant, long) || variant.GetLong() != value )
    {
        variant = (long)value;
        return true;
    }
    return false;
}

bool wxIntProperty::DoValidation( const wxPGProperty* property, wxLongLong_t& value, wxPGValidationInfo* pValidationInfo, int mode )
{
    // Check for min/max
    wxLongLong_t min = wxINT64_MIN;
    wxLongLong_t max = wxINT64_MAX;
    wxVariant variant;
    bool minOk = false;
    bool maxOk = false;

    variant = property->GetAttribute(wxPGGlobalVars->m_strMin);
    if ( !variant.IsNull() )
    {
        wxPGVariantToLongLong(variant, &min);
        minOk = true;
    }

    variant = property->GetAttribute(wxPGGlobalVars->m_strMax);
    if ( !variant.IsNull() )
    {
        wxPGVariantToLongLong(variant, &max);
        maxOk = true;
    }

    if ( minOk )
    {
        if ( value < min )
        {
            if ( mode == wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE )
            {
                wxString msg;
                if ( !maxOk )
                    msg = wxString::Format(
                                _("Value must be %lld or higher"), min);
                else
                    msg = wxString::Format(
                                _("Value must be between %lld and %lld."),
                                min, max);
                pValidationInfo->m_failureMessage = msg;
            }
            else if ( mode == wxPG_PROPERTY_VALIDATION_SATURATE )
                value = min;
            else
                value = max - (min - value);
            return false;
        }
    }

    if ( maxOk )
    {
        if ( value > max )
        {
            if ( mode == wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE )
            {
                wxString msg;
                if ( !minOk )
                    msg = wxString::Format(
                                _("Value must be %lld or lower."), max);
                else
                    msg = wxString::Format(
                                _("Value must be between %lld and %lld."),
                                min, max);
                pValidationInfo->m_failureMessage = msg;
            }
            else if ( mode == wxPG_PROPERTY_VALIDATION_SATURATE )
                value = max;
            else
                value = min + (value - max);
            return false;
        }
    }

    return true;
}

bool wxIntProperty::ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const
{
    wxLongLong_t ll;
    if ( wxPGVariantToLongLong(value, &ll) )
        return DoValidation(this, ll, &validationInfo, wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE);
    return true;
}

wxValidator* wxIntProperty::GetClassValidator()
{
#if wxUSE_VALIDATORS
    WX_PG_DOGETVALIDATOR_ENTRY()

    wxValidator* validator = new wxNumericPropertyValidator(
                                    wxNumericPropertyValidator::Signed);

    WX_PG_DOGETVALIDATOR_EXIT(validator)
#else
    return NULL;
#endif
}

wxValidator* wxIntProperty::DoGetValidator() const
{
    return GetClassValidator();
}

// -----------------------------------------------------------------------
// wxUIntProperty
// -----------------------------------------------------------------------


#define wxPG_UINT_TEMPLATE_MAX 8

static const wxChar* gs_uintTemplates32[wxPG_UINT_TEMPLATE_MAX] = {
    wxT("%x"),wxT("0x%x"),wxT("$%x"),
    wxT("%X"),wxT("0x%X"),wxT("$%X"),
    wxT("%u"),wxT("%o")
};

static const wxChar* gs_uintTemplates64[wxPG_UINT_TEMPLATE_MAX] = {
    wxT("%") wxLongLongFmtSpec wxT("x"),
    wxT("0x%") wxLongLongFmtSpec wxT("x"),
    wxT("$%") wxLongLongFmtSpec wxT("x"),
    wxT("%") wxLongLongFmtSpec wxT("X"),
    wxT("0x%") wxLongLongFmtSpec wxT("X"),
    wxT("$%") wxLongLongFmtSpec wxT("X"),
    wxT("%") wxLongLongFmtSpec wxT("u"),
    wxT("%") wxLongLongFmtSpec wxT("o")
};

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxUIntProperty,wxPGProperty,
                               long,unsigned long,TextCtrl)

void wxUIntProperty::Init()
{
    m_base = 6; // This is magic number for dec base (must be same as in setattribute)
    m_realBase = 10;
    m_prefix = wxPG_PREFIX_NONE;
}

wxUIntProperty::wxUIntProperty( const wxString& label, const wxString& name,
    unsigned long value ) : wxPGProperty(label,name)
{
    Init();
    SetValue((long)value);
}

wxUIntProperty::wxUIntProperty( const wxString& label, const wxString& name,
    const wxULongLong& value ) : wxPGProperty(label,name)
{
    Init();
    SetValue(wxULongLongToVariant(value));
}

wxUIntProperty::~wxUIntProperty() { }

wxString wxUIntProperty::GetValueAsString( int ) const
{
    size_t index = m_base + m_prefix;
    if ( index >= wxPG_UINT_TEMPLATE_MAX )
        index = wxPG_BASE_DEC;

    if ( wxPGIsVariantType(m_value, long) )
        return wxString::Format(gs_uintTemplates32[index],(unsigned long)m_value.GetLong());
    else
        return wxString::Format(gs_uintTemplates64[index],wxULongLongFromVariant(m_value).GetValue());
}

bool wxUIntProperty::StringToValue( wxVariant& variant, const wxString& text, int WXUNUSED(argFlags) ) const
{
    //long unsigned value32 = 0;
    bool isPrevLong = wxPGIsVariantType(variant, long);

    if ( text.length() == 0 )
    {
        variant.MakeNull();
        return true;
    }

    size_t start = 0;
    if ( text[0] == wxT('$') )
        start++;

    wxULongLong_t value64 = 0;
    wxString s = text.substr(start, text.length() - start);

    if ( wxPGStringToULongLong(s, &value64, (unsigned int)m_realBase) )
    {
        if ( value64 >= LONG_MAX )
        {
            wxULongLong* _m_value64 = &wxULongLongFromVariant(m_value);
            if ( isPrevLong || !_m_value64 || _m_value64->GetValue() != value64 )
            {
                variant = wxULongLongToVariant(value64);
                return true;
            }
        }
        else
        {
            unsigned long value32 = wxLongLong(value64).GetLo();
            if ( !isPrevLong || m_value.GetLong() != (long)value32 )
            {
                variant = (long)value32;
                return true;
            }
        }

    }
    return false;
}

bool wxUIntProperty::IntToValue( wxVariant& variant, int number, int WXUNUSED(argFlags) ) const
{
    if ( m_value.IsNull() || m_value != (long)number )
    {
        variant = (long)number;
        return true;
    }
    return false;
}

bool wxUIntProperty::ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const
{
    // Check for min/max
    wxULongLong_t ll;
    if ( wxPGVariantToULongLong(value, &ll) )
    {
        wxULongLong_t min = wxUINT64_MIN;
        wxULongLong_t max = wxUINT64_MAX;
        wxVariant variant;

        variant = GetAttribute(wxPGGlobalVars->m_strMin);
        if ( !variant.IsNull() )
        {
            wxPGVariantToULongLong(variant, &min);
            if ( ll < min )
            {
                validationInfo.m_failureMessage = wxString::Format(_("Value must be %llu or higher"),min);
                return false;
            }
        }
        variant = GetAttribute(wxPGGlobalVars->m_strMax);
        if ( !variant.IsNull() )
        {
            wxPGVariantToULongLong(variant, &max);
            if ( ll > max )
            {
                validationInfo.m_failureMessage = wxString::Format(_("Value must be %llu or less"),max);
                return false;
            }
        }
    }
    return true;
}

wxValidator* wxUIntProperty::DoGetValidator() const
{
#if wxUSE_VALIDATORS
    WX_PG_DOGETVALIDATOR_ENTRY()

    wxValidator* validator = new wxNumericPropertyValidator(
                                    wxNumericPropertyValidator::Unsigned,
                                    m_realBase);

    WX_PG_DOGETVALIDATOR_EXIT(validator)
#else
    return NULL;
#endif
}

bool wxUIntProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    if ( name == wxPG_UINT_BASE )
    {
        int val = value.GetLong();

        m_realBase = (wxByte) val;
        if ( m_realBase > 16 )
            m_realBase = 16;

        //
        // Translate logical base to a template array index
        m_base = 7; // oct
        if ( val == wxPG_BASE_HEX )
            m_base = 3;
        else if ( val == wxPG_BASE_DEC )
            m_base = 6;
        else if ( val == wxPG_BASE_HEXL )
            m_base = 0;
        return true;
    }
    else if ( name == wxPG_UINT_PREFIX )
    {
        m_prefix = (wxByte) value.GetLong();
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// wxFloatProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxFloatProperty,wxPGProperty,
                               double,double,TextCtrl)

wxFloatProperty::wxFloatProperty( const wxString& label,
                                            const wxString& name,
                                            double value )
    : wxPGProperty(label,name)
{
    m_precision = -1;
    SetValue(value);
}

wxFloatProperty::~wxFloatProperty() { }

// This helper method provides standard way for floating point-using
// properties to convert values to string.
void wxPropertyGrid::DoubleToString(wxString& target,
                                    double value,
                                    int precision,
                                    bool removeZeroes,
                                    wxString* precTemplate)
{
    if ( precision >= 0 )
    {
        wxString text1;
        if (!precTemplate)
            precTemplate = &text1;

        if ( !precTemplate->length() )
        {
            *precTemplate = wxT("%.");
            *precTemplate << wxString::Format( wxT("%i"), precision );
            *precTemplate << wxT('f');
        }

        target.Printf( precTemplate->c_str(), value );
    }
    else
    {
        target.Printf( wxT("%f"), value );
    }

    if ( removeZeroes && precision != 0 && target.length() )
    {
        // Remove excess zeroes (do not remove this code just yet,
        // since sprintf can't do the same consistently across platforms).
        wxString::const_iterator i = target.end() - 1;
        size_t new_len = target.length() - 1;

        for ( ; i != target.begin(); i-- )
        {
            if ( wxPGGetIterChar(target, i) != wxT('0') )
                break;
            new_len--;
        }

        wxChar cur_char = wxPGGetIterChar(target, i);
        if ( cur_char != wxT('.') && cur_char != wxT(',') )
            new_len++;

        if ( new_len != target.length() )
            target.resize(new_len);
    }
}

wxString wxFloatProperty::GetValueAsString( int argFlags ) const
{
    wxString text;
    if ( !m_value.IsNull() )
    {
        wxPropertyGrid::DoubleToString(text,
                                       m_value,
                                       m_precision,
                                       !(argFlags & wxPG_FULL_VALUE),
                                       (wxString*) NULL);
    }
    return text;
}

bool wxFloatProperty::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    wxString s;
    double value;

    if ( text.length() == 0 )
    {
        variant.MakeNull();
        return true;
    }

    bool res = text.ToDouble(&value);
    if ( res )
    {
        if ( m_value.IsNull() || m_value != value )
        {
            variant = value;
            return true;
        }
    }
    else if ( argFlags & wxPG_REPORT_ERROR )
    {
    }
    return false;
}

bool wxFloatProperty::DoValidation( const wxPGProperty* property, double& value, wxPGValidationInfo* pValidationInfo, int mode )
{
    // Check for min/max
    double min = (double)wxINT64_MIN;
    double max = (double)wxINT64_MAX;
    wxVariant variant;
    bool minOk = false;
    bool maxOk = false;

    variant = property->GetAttribute(wxPGGlobalVars->m_strMin);
    if ( !variant.IsNull() )
    {
        wxPGVariantToDouble(variant, &min);
        minOk = true;
    }

    variant = property->GetAttribute(wxPGGlobalVars->m_strMax);
    if ( !variant.IsNull() )
    {
        wxPGVariantToDouble(variant, &max);
        maxOk = true;
    }

    if ( minOk )
    {
        if ( value < min )
        {
            if ( mode == wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE )
                pValidationInfo->m_failureMessage = wxString::Format(_("Value must be %f or higher"),min);
            else if ( mode == wxPG_PROPERTY_VALIDATION_SATURATE )
                value = min;
            else
                value = max - (min - value);
            return false;
        }
    }

    if ( maxOk )
    {
        wxPGVariantToDouble(variant, &max);
        if ( value > max )
        {
            if ( mode == wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE )
                pValidationInfo->m_failureMessage = wxString::Format(_("Value must be %f or less"),max);
            else if ( mode == wxPG_PROPERTY_VALIDATION_SATURATE )
                value = max;
            else
                value = min + (value - max);
            return false;
        }
    }
    return true;
}

bool wxFloatProperty::ValidateValue( wxVariant& value, wxPGValidationInfo& validationInfo ) const
{
    double fpv;
    if ( wxPGVariantToDouble(value, &fpv) )
        return DoValidation(this, fpv, &validationInfo, wxPG_PROPERTY_VALIDATION_ERROR_MESSAGE);
    return true;
}

bool wxFloatProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    if ( name == wxPG_FLOAT_PRECISION )
    {
        m_precision = value.GetLong();
        return true;
    }
    return false;
}

wxValidator*
wxFloatProperty::GetClassValidator()
{
#if wxUSE_VALIDATORS
    WX_PG_DOGETVALIDATOR_ENTRY()

    wxValidator* validator = new wxNumericPropertyValidator(
                                    wxNumericPropertyValidator::Float);

    WX_PG_DOGETVALIDATOR_EXIT(validator)
#else
    return NULL;
#endif
}

wxValidator* wxFloatProperty::DoGetValidator() const
{
    return GetClassValidator();
}

// -----------------------------------------------------------------------
// wxBoolProperty
// -----------------------------------------------------------------------

// We cannot use standard WX_PG_IMPLEMENT_PROPERTY_CLASS macro, since
// there is a custom GetEditorClass.

IMPLEMENT_DYNAMIC_CLASS(wxBoolProperty, wxPGProperty)

const wxPGEditor* wxBoolProperty::DoGetEditorClass() const
{
    // Select correct editor control.
#if wxPG_INCLUDE_CHECKBOX
    if ( !(m_flags & wxPG_PROP_USE_CHECKBOX) )
        return wxPG_EDITOR(Choice);
    return wxPG_EDITOR(CheckBox);
#else
    return wxPG_EDITOR(Choice);
#endif
}

wxBoolProperty::wxBoolProperty( const wxString& label, const wxString& name, bool value ) :
    wxPGProperty(label,name)
{
    SetValue(wxPGVariant_Bool(value));

    m_flags |= wxPG_PROP_USE_DCC;
}

wxBoolProperty::~wxBoolProperty() { }

wxString wxBoolProperty::GetValueAsString( int argFlags ) const
{
    bool value = m_value.GetBool();

    // As a fragment of composite string value,
    // make it a little more readable.
    if ( argFlags & wxPG_COMPOSITE_FRAGMENT )
    {
        if ( value )
        {
            return m_label;
        }
        else
        {
            if ( argFlags & wxPG_UNEDITABLE_COMPOSITE_FRAGMENT )
                return wxEmptyString;

            const wxChar* notFmt;
            if ( wxPGGlobalVars->m_autoGetTranslation )
                notFmt = _("Not %s");
            else
                notFmt = wxT("Not %s");

            return wxString::Format(notFmt,m_label.c_str());
        }
    }

    if ( !(argFlags & wxPG_FULL_VALUE) )
    {
        return wxPGGlobalVars->m_boolChoices[value?1:0].GetText();
    }

    wxString text;

    if (value) text = wxT("true");
    else text = wxT("false");

    return text;
}

int wxBoolProperty::GetChoiceInfo( wxPGChoiceInfo* choiceinfo )
{
    if ( choiceinfo )
        choiceinfo->m_choices = &wxPGGlobalVars->m_boolChoices;

    if ( IsValueUnspecified() )
        return -1;

    return m_value.GetBool()?1:0;
}

bool wxBoolProperty::StringToValue( wxVariant& variant, const wxString& text, int WXUNUSED(argFlags) ) const
{
    int value = 0;
    if ( text.CmpNoCase(wxPGGlobalVars->m_boolChoices[1].GetText()) == 0 ||
         text.CmpNoCase(wxT("true")) == 0 ||
         text.CmpNoCase(m_label) == 0 )
        value = 1;

    if ( text.length() == 0 )
    {
        variant.MakeNull();
        return true;
    }

    if ( m_value.IsNull() || (m_value.GetBool() && !value) || (!m_value.GetBool() && value) )
    {
        variant = wxPGVariant_Bool(value);
        return true;
    }
    return false;
}

bool wxBoolProperty::IntToValue( wxVariant& variant, int value, int ) const
{
    bool boolValue = value ? true : false;

    if ( m_value.IsNull() || boolValue != m_value.GetBool() )
    {
        variant = wxPGVariant_Bool(boolValue);
        return true;
    }
    return false;
}

bool wxBoolProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
#if wxPG_INCLUDE_CHECKBOX
    if ( name == wxPG_BOOL_USE_CHECKBOX )
    {
        int ival = wxPGVariantToInt(value);
        if ( ival )
            m_flags |= wxPG_PROP_USE_CHECKBOX;
        else
            m_flags &= ~(wxPG_PROP_USE_CHECKBOX);
        return true;
    }
#endif
    if ( name == wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING )
    {
        int ival = wxPGVariantToInt(value);
        if ( ival )
            m_flags |= wxPG_PROP_USE_DCC;
        else
            m_flags &= ~(wxPG_PROP_USE_DCC);
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// wxBaseEnumProperty
// -----------------------------------------------------------------------

int wxBaseEnumProperty::ms_nextIndex = -2;

wxBaseEnumProperty::wxBaseEnumProperty( const wxString& label, const wxString& name )
    : wxPGProperty(label,name)
{
    m_value = wxPGVariant_Zero;
}

/** If has values array, then returns number at index with value -
    otherwise just returns the value.
*/
int wxBaseEnumProperty::GetIndexForValue( int value ) const
{
    return value;
}

void wxBaseEnumProperty::OnSetValue()
{
    if ( wxPGIsVariantType(m_value, long) )
        ValueFromInt_( m_value, m_value.GetLong(), wxPG_FULL_VALUE );
    else if ( wxPGIsVariantType(m_value, string) )
        ValueFromString_( m_value, m_value.GetString(), 0 );
    else
        wxASSERT( false );

    if ( ms_nextIndex != -2 )
    {
        m_index = ms_nextIndex;
        ms_nextIndex = -2;
    }
}

bool wxBaseEnumProperty::ValidateValue( wxVariant& value, wxPGValidationInfo& WXUNUSED(validationInfo) ) const
{
    // Make sure string value is in the list,
    // unless property has string as preferred value type
    // To reduce code size, use conversion here as well
    if ( wxPGIsVariantType(value, string) &&
         !this->IsKindOf(CLASSINFO(wxEditEnumProperty)) )
        return ValueFromString_( value, value.GetString(), wxPG_PROPERTY_SPECIFIC );

    return true;
}

wxString wxBaseEnumProperty::GetValueAsString( int ) const
{
    if ( wxPGIsVariantType(m_value, string) )
        return m_value.GetString();

    if ( m_index >= 0 )
    {
        int unusedVal;
        const wxString* pstr = GetEntry( m_index, &unusedVal );

        if ( pstr )
            return *pstr;
    }
    return wxEmptyString;
}

bool wxBaseEnumProperty::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    return ValueFromString_( variant, text, argFlags );
}

bool wxBaseEnumProperty::IntToValue( wxVariant& variant, int intVal, int argFlags ) const
{
    return ValueFromInt_( variant, intVal, argFlags );
}

bool wxBaseEnumProperty::ValueFromString_( wxVariant& value, const wxString& text, int argFlags ) const
{
    size_t i = 0;
    const wxString* entryLabel;
    int entryValue;
    int useIndex = -1;
    long useValue = 0;

    entryLabel = GetEntry(i, &entryValue);
    while ( entryLabel )
    {
        if ( text.CmpNoCase(*entryLabel) == 0 )
        {
            useIndex = (int)i;
            useValue = (long)entryValue;
            break;
        }

        i++;
        entryLabel = GetEntry(i, &entryValue);
    }

    bool asText = false;

    bool isEdit = this->IsKindOf(CLASSINFO(wxEditEnumProperty));

    // If text not any of the choices, store as text instead
    // (but only if we are wxEditEnumProperty)
    if ( useIndex == -1 &&
         isEdit )
    {
        asText = true;
    }

    int setAsNextIndex = -2;
    int curIndex = GetIndex();

    if ( asText )
    {
        setAsNextIndex = -1;
        value = text;
    }
    else if ( curIndex != useIndex )
    {
        if ( useIndex != -1 )
        {
            setAsNextIndex = useIndex;
            value = (long)useValue;
        }
        else
        {
            setAsNextIndex = -1;
            value = wxPGVariant_MinusOne;
        }
    }

    if ( setAsNextIndex != -2 )
    {
        // If wxPG_PROPERTY_SPECIFIC is set, then this is done for
        // validation purposes only, and index must not be changed
        if ( !(argFlags & wxPG_PROPERTY_SPECIFIC) )
            ms_nextIndex = setAsNextIndex;

        if ( isEdit || setAsNextIndex != -1 )
            return true;
        else
            return false;
    }
    return false;
}

bool wxBaseEnumProperty::ValueFromInt_( wxVariant& variant, int intVal, int argFlags ) const
{
    // If wxPG_FULL_VALUE is *not* in argFlags, then intVal is index from combo box.
    //
    int curIndex = GetIndex();
    ms_nextIndex = -2;

    if ( argFlags & wxPG_FULL_VALUE )
    {
        ms_nextIndex = GetIndexForValue( intVal );
    }
    else
    {
        if ( curIndex != intVal )
        {
            ms_nextIndex = intVal;
        }
    }

    if ( ms_nextIndex != -2 )
    {
        if ( !(argFlags & wxPG_FULL_VALUE) )
            GetEntry(intVal, &intVal);

        variant = (long)intVal;

        return true;
    }

    return false;
}

void
wxBaseEnumProperty::OnValidationFailure( wxVariant& WXUNUSED(pendingValue) )
{
    // Revert index
    ResetNextIndex();
}

void wxBaseEnumProperty::SetIndex( int index )
{
    ms_nextIndex = -2;
    m_index = index;
}

int wxBaseEnumProperty::GetIndex() const
{
    if ( m_value.IsNull() )
        return -1;

    if ( ms_nextIndex != -2 )
        return ms_nextIndex;

    return m_index;
}

// -----------------------------------------------------------------------
// wxEnumProperty
// -----------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxEnumProperty, wxPGProperty)

WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN(wxEnumProperty,long,Choice)

wxEnumProperty::wxEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
    const long* values, int value ) : wxBaseEnumProperty(label,name)
{
    SetIndex(0);

    if ( labels )
    {
        m_choices.Add(labels,values);

        if ( GetItemCount() )
            SetValue( (long)value );
    }
}

wxEnumProperty::wxEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
    const long* values, wxPGChoices* choicesCache, int value )
    : wxBaseEnumProperty(label,name)
{
    SetIndex(0);

    wxASSERT( choicesCache );

    if ( choicesCache->IsOk() )
    {
        m_choices.Assign( *choicesCache );
        m_value = wxPGVariant_Zero;
    }
    else if ( labels )
    {
        m_choices.Add(labels,values);

        if ( GetItemCount() )
            SetValue( (long)value );
    }
}

wxEnumProperty::wxEnumProperty( const wxString& label, const wxString& name,
    const wxArrayString& labels, const wxArrayInt& values, int value ) : wxBaseEnumProperty(label,name)
{
    SetIndex(0);

    if ( &labels && labels.size() )
    {
        m_choices.Set(labels, values);

        if ( GetItemCount() )
            SetValue( (long)value );
    }
}

wxEnumProperty::wxEnumProperty( const wxString& label, const wxString& name,
    wxPGChoices& choices, int value )
    : wxBaseEnumProperty(label,name)
{
    m_choices.Assign( choices );

    if ( GetItemCount() )
        SetValue( (long)value );
}

int wxEnumProperty::GetIndexForValue( int value ) const
{
    if ( !m_choices.IsOk() )
        return -1;

    int intVal = m_choices.Index(value);
    if ( intVal >= 0 )
        return intVal;

    return value;
}

wxEnumProperty::~wxEnumProperty ()
{
}

const wxString* wxEnumProperty::GetEntry( size_t index, int* pvalue ) const
{
    if ( m_choices.IsOk() && index < m_choices.GetCount() )
    {
        int value = m_choices.GetValue(index);

        if ( pvalue )
            *pvalue = value;

        return &m_choices.GetLabel(index);
    }
    return (const wxString*) NULL;
}

int wxEnumProperty::GetChoiceInfo( wxPGChoiceInfo* choiceinfo )
{
    if ( choiceinfo )
        choiceinfo->m_choices = &m_choices;

    if ( !m_choices.IsOk() )
        return -1;

    return GetIndex();
}

// -----------------------------------------------------------------------
// wxEditEnumProperty
// -----------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxEditEnumProperty, wxPGProperty)

WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN(wxEditEnumProperty,wxString,ComboBox)

wxEditEnumProperty::wxEditEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
    const long* values, const wxString& value )
    : wxEnumProperty(label,name,labels,values,0)
{
    SetValue( value );
}

wxEditEnumProperty::wxEditEnumProperty( const wxString& label, const wxString& name, const wxChar** labels,
    const long* values, wxPGChoices* choicesCache, const wxString& value )
    : wxEnumProperty(label,name,labels,values,choicesCache,0)
{
    SetValue( value );
}

wxEditEnumProperty::wxEditEnumProperty( const wxString& label, const wxString& name,
    const wxArrayString& labels, const wxArrayInt& values, const wxString& value )
    : wxEnumProperty(label,name,labels,values,0)
{
    SetValue( value );
}

wxEditEnumProperty::wxEditEnumProperty( const wxString& label, const wxString& name,
    wxPGChoices& choices, const wxString& value )
    : wxEnumProperty(label,name,choices,0)
{
    SetValue( value );
}

wxEditEnumProperty::~wxEditEnumProperty()
{
}

// -----------------------------------------------------------------------
// wxFlagsProperty
// -----------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxFlagsProperty,wxPGProperty)

WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN(wxFlagsProperty,long,TextCtrl)

void wxFlagsProperty::Init()
{
    SetFlag(wxPG_PROP_AGGREGATE);  // This is must be done here to support flag props
                                   // with inital zero children.

    long value = m_value;

    //
    // Generate children
    //
    unsigned int i;

    unsigned int prevChildCount = m_children.GetCount();

    int oldSel = -1;
    if ( prevChildCount )
    {
        wxPropertyGridState* state = GetParentState();

        // State safety check (it may be NULL in immediate parent)
        wxASSERT( state );

        if ( state )
        {
            wxPGProperty* selected = state->GetSelection();
            if ( selected )
            {
                if ( selected->GetParent() == this )
                    oldSel = selected->GetArrIndex();
                else if ( selected == this )
                    oldSel = -2;
            }
        }
        state->DoClearSelection();
    }

    // Delete old children
    for ( i=0; i<prevChildCount; i++ )
        delete ( (wxPGProperty*) m_children[i] );

    m_children.Empty();

    if ( m_choices.IsOk() )
    {
        const wxPGChoices& choices = m_choices;

        for ( i=0; i<GetItemCount(); i++ )
        {
            bool child_val = ( value & choices.GetValue(i) )?true:false;

            wxPGProperty* boolProp;

        #if wxUSE_INTL
            if ( wxPGGlobalVars->m_autoGetTranslation )
            {
                boolProp = new wxBoolProperty( ::wxGetTranslation( GetLabel(i) ), wxPG_LABEL, child_val );
            }
            else
        #endif
            {
                boolProp = new wxBoolProperty( GetLabel(i), wxPG_LABEL, child_val );
            }
            AddPrivateChild(boolProp);
        }

        m_oldChoicesData = m_choices.GetDataPtr();
    }

    m_oldValue = m_value;

    if ( prevChildCount )
        SubPropsChanged(oldSel);
}

wxFlagsProperty::wxFlagsProperty( const wxString& label, const wxString& name,
    const wxChar** labels, const long* values, long value ) : wxPGProperty(label,name)
{
    m_oldChoicesData = (wxPGChoicesData*) NULL;

    if ( labels )
    {
        m_choices.Set(labels,values);

        wxASSERT( GetItemCount() );

        SetValue( value );
    }
    else
    {
        m_value = wxPGVariant_Zero;
    }
}

wxFlagsProperty::wxFlagsProperty( const wxString& label, const wxString& name,
        const wxArrayString& labels, const wxArrayInt& values, int value )
    : wxPGProperty(label,name)
{
    m_oldChoicesData = (wxPGChoicesData*) NULL;

    if ( &labels && labels.size() )
    {
        m_choices.Set(labels,values);

        wxASSERT( GetItemCount() );

        SetValue( (long)value );
    }
    else
    {
        m_value = wxPGVariant_Zero;
    }
}

wxFlagsProperty::wxFlagsProperty( const wxString& label, const wxString& name,
    wxPGChoices& choices, long value )
    : wxPGProperty(label,name)
{
    m_oldChoicesData = (wxPGChoicesData*) NULL;

    if ( choices.IsOk() )
    {
        m_choices.Assign(choices);

        wxASSERT( GetItemCount() );

        SetValue( value );
    }
    else
    {
        m_value = wxPGVariant_Zero;
    }
}

wxFlagsProperty::~wxFlagsProperty()
{
}

void wxFlagsProperty::OnSetValue()
{
    if ( !m_choices.IsOk() || !GetItemCount() )
    {
        m_value = wxPGVariant_Zero;
    }
    else
    {
        long val = m_value.GetLong();

        long fullFlags = 0;

        // normalize the value (i.e. remove extra flags)
        unsigned int i;
        const wxPGChoices& choices = m_choices;
        for ( i = 0; i < GetItemCount(); i++ )
        {
            fullFlags |= choices.GetValue(i);
        }

        val &= fullFlags;

        m_value = val;

        // Need to (re)init now?
        if ( GetCount() != GetItemCount() ||
             m_choices.GetDataPtr() != m_oldChoicesData )
        {
            Init();
        }
    }

    long newFlags = m_value;

    if ( newFlags != m_oldValue )
    {
        // Set child modified states
        unsigned int i;
        const wxPGChoices& choices = m_choices;
        for ( i = 0; i<GetItemCount(); i++ )
        {
            int flag;

            flag = choices.GetValue(i);

            if ( (newFlags & flag) != (m_oldValue & flag) )
                Item(i)->SetFlag( wxPG_PROP_MODIFIED );
        }

        m_oldValue = newFlags;
    }
}

wxString wxFlagsProperty::GetValueAsString( int ) const
{
    wxString text;

    if ( !m_choices.IsOk() )
        return text;

    long flags = m_value;
    unsigned int i;
    const wxPGChoices& choices = m_choices;

    for ( i = 0; i < GetItemCount(); i++ )
    {
        int doAdd;
        doAdd = ( flags & choices.GetValue(i) );

        if ( doAdd )
        {
            text += choices.GetLabel(i);
            text += wxT(", ");
        }
    }

    // remove last comma
    if ( text.Len() > 1 )
        text.Truncate ( text.Len() - 2 );

    return text;
}

// Translate string into flag tokens
bool wxFlagsProperty::StringToValue( wxVariant& variant, const wxString& text, int ) const
{
    if ( !m_choices.IsOk() )
        return false;

    long newFlags = 0;

    // semicolons are no longer valid delimeters
    WX_PG_TOKENIZER1_BEGIN(text,wxT(','))

        if ( token.length() )
        {
            // Determine which one it is
            long bit = IdToBit( token );

            if ( bit != -1 )
            {
                // Changed?
                newFlags |= bit;
            }
            else
            {
                break;
            }
        }

    WX_PG_TOKENIZER1_END()

    variant = newFlags;

    if ( m_value.IsNull() || newFlags != m_value.GetLong() )
        return true;

    return false;
}

// Converts string id to a relevant bit.
long wxFlagsProperty::IdToBit( const wxString& id ) const
{
    unsigned int i;
    for ( i = 0; i < GetItemCount(); i++ )
    {
        if ( id == GetLabel(i) )
        {
            return m_choices.GetValue(i);
        }
    }
    return -1;
}

void wxFlagsProperty::RefreshChildren()
{
    if ( !m_choices.IsOk() || !GetCount() ) return;

    int flags = m_value.GetLong();

    const wxPGChoices& choices = m_choices;
    unsigned int i;
    for ( i = 0; i < GetItemCount(); i++ )
    {
        long flag;

        flag = choices.GetValue(i);

        long subVal = flags & flag;
        wxPGProperty* p = Item(i);

        if ( subVal != (m_oldValue & flag) )
            p->SetFlag( wxPG_PROP_MODIFIED );

        p->SetValue( subVal?true:false );
    }

    m_oldValue = flags;
}

void wxFlagsProperty::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
{
    long oldValue = thisValue.GetLong();
    long val = childValue.GetLong();
    unsigned long vi = m_choices.GetValue(childIndex);
    if ( val )
        thisValue = (long)(oldValue | vi);
    else
        thisValue = (long)(oldValue & ~(vi));
}

int wxFlagsProperty::GetChoiceInfo( wxPGChoiceInfo* choiceinfo )
{
    if ( choiceinfo )
        choiceinfo->m_choices = &m_choices;
    return -1;
}

// -----------------------------------------------------------------------
// wxDirProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(wxDirProperty,wxLongStringProperty,const wxString&)

wxDirProperty::wxDirProperty( const wxString& name, const wxString& label, const wxString& value )
  : wxLongStringProperty(name,label,value)
{
    m_flags |= wxPG_NO_ESCAPE;
}
wxDirProperty::~wxDirProperty() { }

wxValidator* wxDirProperty::DoGetValidator() const
{
    return wxFileProperty::GetClassValidator();
}

bool wxDirProperty::OnButtonClick( wxPropertyGrid* propGrid, wxString& value )
{
    wxSize dlg_sz(300,400);

    wxDirDialog dlg( propGrid,
                     m_dlgMessage.length() ? m_dlgMessage : wxString(_("Choose a directory:")),
                     value,
                     0,
#if !wxPG_SMALL_SCREEN
                     propGrid->GetGoodEditorDialogPosition(this,dlg_sz),
                     dlg_sz );
#else
                     wxDefaultPosition,
                     wxDefaultSize );
#endif

    if ( dlg.ShowModal() == wxID_OK )
    {
        value = dlg.GetPath();
        return true;
    }
    return false;
}

bool wxDirProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    if ( name == wxPG_DIR_DIALOG_MESSAGE )
    {
        m_dlgMessage = value.GetString();
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// wxPGFileDialogAdapter
// -----------------------------------------------------------------------

bool wxPGFileDialogAdapter::DoShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property )
{
    wxFileProperty* fileProp = NULL;
    wxString path;
    int indFilter = -1;

    if ( property->IsKindOf(CLASSINFO(wxFileProperty)) )
    {
        fileProp = ((wxFileProperty*)property);
        path = fileProp->m_filename.GetPath();
        indFilter = fileProp->m_indFilter;

        if ( !path.length() && fileProp->m_basePath.length() )
            path = fileProp->m_basePath;
    }
    else
    {
        wxFileName fn(property->GetValue().GetString());
        path = fn.GetPath();
    }

    wxFileDialog dlg( propGrid->GetPanel(),
                      property->GetAttribute(wxT("DialogTitle"), _("Choose a file")),
                      property->GetAttribute(wxT("InitialPath"), path),
                      wxEmptyString,
                      property->GetAttribute(wxPG_FILE_WILDCARD, _("All files (*.*)|*.*")),
                      0,
                      wxDefaultPosition );

    if ( indFilter >= 0 )
        dlg.SetFilterIndex( indFilter );

    if ( dlg.ShowModal() == wxID_OK )
    {
        if ( fileProp )
            fileProp->m_indFilter = dlg.GetFilterIndex();
        SetValue( dlg.GetPath() );
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// wxFileProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxFileProperty,wxPGProperty,
                               wxString,const wxString&,TextCtrlAndButton)

wxFileProperty::wxFileProperty( const wxString& label, const wxString& name,
    const wxString& value ) : wxPGProperty(label,name)
{
    m_flags |= wxPG_PROP_SHOW_FULL_FILENAME;
    m_indFilter = -1;
    SetAttribute( wxPG_FILE_WILDCARD, _("All files (*.*)|*.*") );

    SetValue(value);
}

wxFileProperty::~wxFileProperty() {}

#if wxUSE_VALIDATORS

wxValidator* wxFileProperty::GetClassValidator()
{
    WX_PG_DOGETVALIDATOR_ENTRY()

    // Atleast wxPython 2.6.2.1 required that the string argument is given
    static wxString v;
    wxTextValidator* validator = new wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST,&v);

    wxArrayString exChars;
    exChars.Add(wxT("?"));
    exChars.Add(wxT("*"));
    exChars.Add(wxT("|"));
    exChars.Add(wxT("<"));
    exChars.Add(wxT(">"));
    exChars.Add(wxT("\""));

    validator->SetExcludes(exChars);

    WX_PG_DOGETVALIDATOR_EXIT(validator)
}

wxValidator* wxFileProperty::DoGetValidator() const
{
    return GetClassValidator();
}

#endif

void wxFileProperty::OnSetValue()
{
    const wxString& fnstr = m_value.GetString();

    m_filename = fnstr;

    if ( !m_filename.HasName() )
    {
        m_value = wxPGVariant_EmptyString;
        m_filename.Clear();
    }

    // Find index for extension.
    if ( m_indFilter < 0 && fnstr.length() )
    {
        wxString ext = m_filename.GetExt();
        int curind = 0;
        size_t pos = 0;
        size_t len = m_wildcard.length();

        pos = m_wildcard.find(wxT("|"), pos);
        while ( pos != wxString::npos && pos < (len-3) )
        {
            size_t ext_begin = pos + 3;

            pos = m_wildcard.find(wxT("|"), ext_begin);
            if ( pos == wxString::npos )
                pos = len;
            wxString found_ext = m_wildcard.substr(ext_begin, pos-ext_begin);

            if ( found_ext.length() > 0 )
            {
                if ( found_ext[0] == wxT('*') )
                {
                    m_indFilter = curind;
                    break;
                }
                if ( ext.CmpNoCase(found_ext) == 0 )
                {
                    m_indFilter = curind;
                    break;
                }
            }

            if ( pos != len )
                pos = m_wildcard.find(wxT("|"), pos+1);

            curind++;
        }
    }
}

wxString wxFileProperty::GetValueAsString( int argFlags ) const
{
    // Always return empty string when name component is empty
    wxString fullName = m_filename.GetFullName();
    if ( !fullName.length() )
        return fullName;

    if ( argFlags & wxPG_FULL_VALUE )
    {
        return m_filename.GetFullPath();
    }
    else if ( m_flags & wxPG_PROP_SHOW_FULL_FILENAME )
    {
        if ( m_basePath.Length() )
        {
            wxFileName fn2(m_filename);
            fn2.MakeRelativeTo(m_basePath);
            return fn2.GetFullPath();
        }
        return m_filename.GetFullPath();
    }

    return m_filename.GetFullName();
}

wxPGEditorDialogAdapter* wxFileProperty::GetEditorDialog() const
{
    return new wxPGFileDialogAdapter();
}

bool wxFileProperty::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    if ( (m_flags & wxPG_PROP_SHOW_FULL_FILENAME) || (argFlags & wxPG_FULL_VALUE) )
    {
        if ( m_filename != text )
        {
            variant = text;
            return true;
        }
    }
    else
    {
        if ( m_filename.GetFullName() != text )
        {
            wxFileName fn = m_filename;
            fn.SetFullName(text);
            variant = fn.GetFullPath();
            return true;
        }
    }

    return false;
}

bool wxFileProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
    // Return false on some occasions to make sure those attribs will get
    // stored in m_attributes.
    if ( name == wxPG_FILE_SHOW_FULL_PATH )
    {
        if ( wxPGVariantToInt(value) )
            m_flags |= wxPG_PROP_SHOW_FULL_FILENAME;
        else
            m_flags &= ~(wxPG_PROP_SHOW_FULL_FILENAME);
        return true;
    }
    else if ( name == wxPG_FILE_WILDCARD )
    {
        m_wildcard = value.GetString();
    }
    else if ( name == wxPG_FILE_SHOW_RELATIVE_PATH )
    {
        m_basePath = value.GetString();

        // Make sure wxPG_FILE_SHOW_FULL_PATH is also set
        m_flags |= wxPG_PROP_SHOW_FULL_FILENAME;
    }
    else if ( name == wxPG_FILE_INITIAL_PATH )
    {
        m_initialPath = value.GetString();
        return true;
    }
    else if ( name == wxPG_FILE_DIALOG_TITLE )
    {
        m_dlgTitle = value.GetString();
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// wxPGLongStringDialogAdapter
// -----------------------------------------------------------------------

bool wxPGLongStringDialogAdapter::DoShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property )
{
    wxString val1 = property->GetValueAsString(0);
    wxString val_orig = val1;

    wxString value;
    if ( !property->HasFlag(wxPG_PROP_NO_ESCAPE) )
        wxPropertyGrid::ExpandEscapeSequences(value, val1);
    else
        value = wxString(val1);

    // Run editor dialog.
    if ( wxLongStringProperty::DisplayEditorDialog(property, propGrid, value) )
    {
        if ( !property->HasFlag(wxPG_PROP_NO_ESCAPE) )
            wxPropertyGrid::CreateEscapeSequences(val1,value);
        else
            val1 = value;

        if ( val1 != val_orig )
        {
            SetValue( val1 );
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------
// wxLongStringProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxLongStringProperty,wxPGProperty,
                               wxString,const wxString&,TextCtrlAndButton)

wxLongStringProperty::wxLongStringProperty( const wxString& label, const wxString& name,
    const wxString& value ) : wxPGProperty(label,name)
{
    SetValue(value);
}

wxLongStringProperty::~wxLongStringProperty() {}

wxString wxLongStringProperty::GetValueAsString( int ) const
{
    return m_value;
}

bool wxLongStringProperty::OnEvent( wxPropertyGrid* propGrid, wxWindow* WXUNUSED(primary),
                                    wxEvent& event )
{
    if ( propGrid->IsMainButtonEvent(event) )
    {
        // Update the value
        PrepareValueForDialogEditing(propGrid);

        wxString val1 = GetValueAsString(0);
        wxString val_orig = val1;

        wxString value;
        if ( !(m_flags & wxPG_PROP_NO_ESCAPE) )
            wxPropertyGrid::ExpandEscapeSequences(value,val1);
        else
            value = wxString(val1);

        // Run editor dialog.
        if ( OnButtonClick(propGrid,value) )
        {
            if ( !(m_flags & wxPG_PROP_NO_ESCAPE) )
                wxPropertyGrid::CreateEscapeSequences(val1,value);
            else
                val1 = value;

            if ( val1 != val_orig )
            {
                SetValueInEvent( val1 );
                return true;
            }
        }
    }
    return false;
}

bool wxLongStringProperty::OnButtonClick( wxPropertyGrid* propGrid, wxString& value )
{
    return DisplayEditorDialog(this, propGrid, value);
}

bool wxLongStringProperty::DisplayEditorDialog( wxPGProperty* prop, wxPropertyGrid* propGrid, wxString& value )

{
    // launch editor dialog
    wxDialog* dlg = new wxDialog(propGrid,-1,prop->GetLabel(),wxDefaultPosition,wxDefaultSize,
                                 wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLIP_CHILDREN);

    dlg->SetFont(propGrid->GetFont()); // To allow entering chars of the same set as the propGrid

    // Multi-line text editor dialog.
#if !wxPG_SMALL_SCREEN
    const int spacing = 8;
#else
    const int spacing = 4;
#endif
    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* rowsizer = new wxBoxSizer( wxHORIZONTAL );
    wxTextCtrl* ed = new wxTextCtrl(dlg,11,value,
        wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);

    rowsizer->Add( ed, 1, wxEXPAND|wxALL, spacing );
    topsizer->Add( rowsizer, 1, wxEXPAND, 0 );

    wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
    buttonSizer->AddButton(new wxButton(dlg, wxID_OK));
    buttonSizer->AddButton(new wxButton(dlg, wxID_CANCEL));
    buttonSizer->Realize();
    topsizer->Add( buttonSizer, 0,
                   wxALIGN_RIGHT|wxALIGN_CENTRE_VERTICAL|wxBOTTOM|wxRIGHT,
                   spacing );

    dlg->SetSizer( topsizer );
    topsizer->SetSizeHints( dlg );

#if !wxPG_SMALL_SCREEN
    dlg->SetSize(400,300);

    dlg->Move( propGrid->GetGoodEditorDialogPosition(prop,dlg->GetSize()) );
#endif

    int res = dlg->ShowModal();

    if ( res == wxID_OK )
    {
        value = ed->GetValue();
        dlg->Destroy();
        return true;
    }
    dlg->Destroy();
    return false;
}

bool wxLongStringProperty::StringToValue( wxVariant& variant, const wxString& text, int ) const
{
    if ( m_value.IsNull() || m_value != text )
    {
        variant = text;
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// wxArrayEditorDialog
// -----------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxArrayEditorDialog, wxDialog)
    EVT_IDLE(wxArrayEditorDialog::OnIdle)
    EVT_LISTBOX(24, wxArrayEditorDialog::OnListBoxClick)
    EVT_TEXT_ENTER(21, wxArrayEditorDialog::OnAddClick)
    EVT_BUTTON(22, wxArrayEditorDialog::OnAddClick)
    EVT_BUTTON(23, wxArrayEditorDialog::OnDeleteClick)
    EVT_BUTTON(25, wxArrayEditorDialog::OnUpClick)
    EVT_BUTTON(26, wxArrayEditorDialog::OnDownClick)
    EVT_BUTTON(27, wxArrayEditorDialog::OnUpdateClick)
    //EVT_BUTTON(28, wxArrayEditorDialog::OnCustomEditClick)
END_EVENT_TABLE()

IMPLEMENT_ABSTRACT_CLASS(wxArrayEditorDialog, wxDialog)

#include <wx/statline.h>

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnIdle(wxIdleEvent& event)
{
    //
    // Do control focus detection here.
    //

    wxWindow* focused = FindFocus();

    // This strange focus thing is a workaround for wxGTK wxListBox focus
    // reporting bug.
    if ( m_curFocus == 0 && focused != m_edValue &&
         focused != m_butAdd && focused != m_butUpdate &&
         m_lbStrings->GetSelection() >= 0 )
    {
        // ListBox was just focused.
        m_butAdd->Enable(false);
        m_butUpdate->Enable(false);
        m_butRemove->Enable(true);
        m_butUp->Enable(true);
        m_butDown->Enable(true);
        m_curFocus = 1;
    }
    else if ( (m_curFocus == 1 && focused == m_edValue) /*|| m_curFocus == 2*/ )
    {
        // TextCtrl was just focused.
        m_butAdd->Enable(true);
        bool upd_enable = false;
        if ( m_lbStrings->GetCount() && m_lbStrings->GetSelection() >= 0 )
            upd_enable = true;
        m_butUpdate->Enable(upd_enable);
        m_butRemove->Enable(false);
        m_butUp->Enable(false);
        m_butDown->Enable(false);
        m_curFocus = 0;
    }

    event.Skip();
}

// -----------------------------------------------------------------------

wxArrayEditorDialog::wxArrayEditorDialog()
    : wxDialog()
{
    Init();
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::Init()
{
    m_custBtText = (const wxChar*) NULL;
#if defined(__WXPYTHON__)
    m_scriptObject = NULL;
#endif
}

// -----------------------------------------------------------------------

wxArrayEditorDialog::wxArrayEditorDialog( wxWindow *parent,
                                          const wxString& message,
                                          const wxString& caption,
                                          long style,
                                          const wxPoint& pos,
                                          const wxSize& sz )
    : wxDialog()
{
    Init();
    Create(parent,message,caption,style,pos,sz);
}

// -----------------------------------------------------------------------

bool wxArrayEditorDialog::Create( wxWindow *parent,
                                  const wxString& message,
                                  const wxString& caption,
                                  long style,
                                  const wxPoint& pos,
                                  const wxSize& sz )
{
    // On wxMAC the dialog shows incorrectly if style is not exactly wxCAPTION
    // FIXME: This should be only a temporary fix.
#ifdef __WXMAC__
    int useStyle = wxCAPTION;
#else
    int useStyle = style;
#endif

    bool res = wxDialog::Create(parent, wxID_ANY, caption, pos, sz, useStyle);

    SetFont(parent->GetFont()); // To allow entering chars of the same set as the propGrid

#if !wxPG_SMALL_SCREEN
    const int spacing = 4;
#else
    const int spacing = 3;
#endif

    m_modified = false;

    m_curFocus = 1;

    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );

    // Message
    if ( message.length() )
        topsizer->Add( new wxStaticText(this,-1,message),
            0, wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL|wxALL, spacing );

    // String editor
    wxBoxSizer* rowsizer = new wxBoxSizer( wxHORIZONTAL );
    m_edValue = new wxTextCtrl(this,21,wxEmptyString,
        wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    wxValidator* validator = GetTextCtrlValidator();
    if ( validator )
    {
        m_edValue->SetValidator( *validator );
        delete validator;
    }
    rowsizer->Add( m_edValue,
        1, wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL|wxALL, spacing );

    // Add button
    m_butAdd = new wxButton(this,22,_("Add"));
    rowsizer->Add( m_butAdd,
        0, wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, spacing );
    topsizer->Add( rowsizer, 0, wxEXPAND, spacing );

    // Separator line
    topsizer->Add( new wxStaticLine(this,-1),
        0, wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT, spacing );

    rowsizer = new wxBoxSizer( wxHORIZONTAL );

    // list box
    m_lbStrings = new wxListBox(this, 24, wxDefaultPosition, wxDefaultSize);
    unsigned int i;
    for ( i=0; i<ArrayGetCount(); i++ )
        m_lbStrings->Append( ArrayGet(i) );
    rowsizer->Add( m_lbStrings, 1, wxEXPAND|wxRIGHT, spacing );

    // Manipulator buttons
    wxBoxSizer* colsizer = new wxBoxSizer( wxVERTICAL );
    m_butCustom = (wxButton*) NULL;
    if ( m_custBtText )
    {
        m_butCustom = new wxButton(this,28,::wxGetTranslation(m_custBtText));
        colsizer->Add( m_butCustom,
            0, wxALIGN_CENTER|wxTOP/*wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT*/,
            spacing );
    }
    m_butUpdate = new wxButton(this,27,_("Update"));
    colsizer->Add( m_butUpdate,
        0, wxALIGN_CENTER|wxTOP, spacing );
    m_butRemove = new wxButton(this,23,_("Remove"));
    colsizer->Add( m_butRemove,
        0, wxALIGN_CENTER|wxTOP, spacing );
    m_butUp = new wxButton(this,25,_("Up"));
    colsizer->Add( m_butUp,
        0, wxALIGN_CENTER|wxTOP, spacing );
    m_butDown = new wxButton(this,26,_("Down"));
    colsizer->Add( m_butDown,
        0, wxALIGN_CENTER|wxTOP, spacing );
    rowsizer->Add( colsizer, 0, 0, spacing );

    topsizer->Add( rowsizer, 1, wxLEFT|wxRIGHT|wxEXPAND, spacing );

    // Separator line
    topsizer->Add( new wxStaticLine(this,-1),
        0, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, spacing );

    // Standard dialog buttons
    wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
    buttonSizer->AddButton(new wxButton(this, wxID_OK));
    buttonSizer->AddButton(new wxButton(this, wxID_CANCEL));
    buttonSizer->Realize();
    topsizer->Add( buttonSizer, 0,
                   wxALIGN_RIGHT|wxALIGN_CENTRE_VERTICAL|wxALL,
                   spacing );

    m_edValue->SetFocus();

    SetSizer( topsizer );
    topsizer->SetSizeHints( this );

#if !wxPG_SMALL_SCREEN
    if ( sz.x == wxDefaultSize.x &&
         sz.y == wxDefaultSize.y )
        SetSize( wxSize(275,360) );
    else
        SetSize(sz);
#endif

    return res;
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnAddClick(wxCommandEvent& )
{
    wxString text = m_edValue->GetValue();
    if ( text.length() )
    {
        if ( ArrayInsert( text, -1 ) )
        {
            m_lbStrings->Append( text );
            m_modified = true;
            m_edValue->Clear();
        }
    }
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnDeleteClick(wxCommandEvent& )
{
    int index = m_lbStrings->GetSelection();
    if ( index >= 0 )
    {
        ArrayRemoveAt( index );
        m_lbStrings->Delete ( index );
        m_modified = true;
    }
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnUpClick(wxCommandEvent& )
{
    int index = m_lbStrings->GetSelection();
    if ( index > 0 )
    {
        ArraySwap(index-1,index);
        /*wxString old_str = m_array[index-1];
        wxString new_str = m_array[index];
        m_array[index-1] = new_str;
        m_array[index] = old_str;*/
        m_lbStrings->SetString ( index-1, ArrayGet(index-1) );
        m_lbStrings->SetString ( index, ArrayGet(index) );
        m_lbStrings->SetSelection ( index-1 );
        m_modified = true;
    }
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnDownClick(wxCommandEvent& )
{
    int index = m_lbStrings->GetSelection();
    int lastStringIndex = ((int) m_lbStrings->GetCount()) - 1;
    if ( index >= 0 && index < lastStringIndex )
    {
        ArraySwap(index,index+1);
        /*wxString old_str = m_array[index+1];
        wxString new_str = m_array[index];
        m_array[index+1] = new_str;
        m_array[index] = old_str;*/
        m_lbStrings->SetString ( index+1, ArrayGet(index+1) );
        m_lbStrings->SetString ( index, ArrayGet(index) );
        m_lbStrings->SetSelection ( index+1 );
        m_modified = true;
    }
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnUpdateClick(wxCommandEvent& )
{
    int index = m_lbStrings->GetSelection();
    if ( index >= 0 )
    {
        wxString str = m_edValue->GetValue();
        if ( ArraySet(index,str) )
        {
            m_lbStrings->SetString ( index, str );
            //m_array[index] = str;
            m_modified = true;
        }
    }
}

// -----------------------------------------------------------------------

void wxArrayEditorDialog::OnListBoxClick(wxCommandEvent& )
{
    int index = m_lbStrings->GetSelection();
    if ( index >= 0 )
    {
        m_edValue->SetValue( m_lbStrings->GetString(index) );
    }
}

// -----------------------------------------------------------------------
// wxPGArrayStringEditorDialog
// -----------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPGArrayStringEditorDialog, wxArrayEditorDialog)

BEGIN_EVENT_TABLE(wxPGArrayStringEditorDialog, wxArrayEditorDialog)
    EVT_BUTTON(28, wxPGArrayStringEditorDialog::OnCustomEditClick)
END_EVENT_TABLE()

// -----------------------------------------------------------------------

wxString wxPGArrayStringEditorDialog::ArrayGet( size_t index )
{
    return m_array[index];
}

size_t wxPGArrayStringEditorDialog::ArrayGetCount()
{
    return m_array.GetCount();
}

bool wxPGArrayStringEditorDialog::ArrayInsert( const wxString& str, int index )
{
    if (index<0)
        m_array.Add(str);
    else
        m_array.Insert(str,index);
    return true;
}

bool wxPGArrayStringEditorDialog::ArraySet( size_t index, const wxString& str )
{
    m_array[index] = str;
    return true;
}

void wxPGArrayStringEditorDialog::ArrayRemoveAt( int index )
{
    m_array.RemoveAt(index);
}

void wxPGArrayStringEditorDialog::ArraySwap( size_t first, size_t second )
{
    wxString old_str = m_array[first];
    wxString new_str = m_array[second];
    m_array[first] = new_str;
    m_array[second] = old_str;
}

wxPGArrayStringEditorDialog::wxPGArrayStringEditorDialog()
    : wxArrayEditorDialog()
{
    Init();
}

void wxPGArrayStringEditorDialog::Init()
{
    m_pCallingClass = (wxArrayStringProperty*) NULL;
}

void wxPGArrayStringEditorDialog::OnCustomEditClick(wxCommandEvent& )
{
    wxASSERT( m_pCallingClass );
    wxString str = m_edValue->GetValue();
    if ( m_pCallingClass->OnCustomStringEdit(m_parent,str) )
    {
        //m_edValue->SetValue ( str );
        m_lbStrings->Append ( str );
        m_array.Add ( str );
        m_modified = true;
    }
}

// -----------------------------------------------------------------------
// wxArrayStringProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxArrayStringProperty,  // Property name
                               wxPGProperty,  // Property we inherit from
                               wxArrayString,  // Value type name
                               const wxArrayString&,  // Value type, as given in constructor
                               TextCtrlAndButton)  // Initial editor

wxArrayStringProperty::wxArrayStringProperty( const wxString& label,
                                                        const wxString& name,
                                                        const wxArrayString& array )
    : wxPGProperty(label,name)
{
    SetValue( array );
}

wxArrayStringProperty::~wxArrayStringProperty() { }

void wxArrayStringProperty::OnSetValue()
{
    GenerateValueAsString();
}

wxString wxArrayStringProperty::GetValueAsString( int WXUNUSED(argFlags) ) const
{
    return m_display;
}

// Converts wxArrayString to a string separated by delimeters and spaces.
// preDelim is useful for "str1" "str2" style. Set flags to 1 to do slash
// conversion.
void wxPropertyGrid::ArrayStringToString( wxString& dst, const wxArrayString& src,
                                          wxChar preDelim, wxChar postDelim,
                                          int flags )
{
    wxString pdr;

    unsigned int i;
    unsigned int itemCount = src.GetCount();

    wxChar preas[2];

    dst.Empty();

    if ( !preDelim )
        preas[0] = 0;
    else if ( (flags & 1) )
    {
        preas[0] = preDelim;
        preas[1] = 0;
        pdr = wxT("\\");
        pdr += preDelim;
    }

    if ( itemCount )
        dst.append( preas );

    wxASSERT( postDelim );
    wxString postDelimStr(postDelim);
    //wxString preDelimStr(preDelim);

    for ( i = 0; i < itemCount; i++ )
    {
        wxString str( src.Item(i) );

        // Do some character conversion.
        // Convertes \ to \\ and <preDelim> to \<preDelim>
        // Useful when preDelim and postDelim are "\"".
        if ( flags & 1 )
        {
            str.Replace( wxT("\\"), wxT("\\\\"), true );
            if ( pdr.length() )
                str.Replace( preas, pdr, true );
        }

        dst.append( str );

        if ( i < (itemCount-1) )
        {
            dst.append( postDelimStr );
            dst.append( wxT(" ") );
            dst.append( preas );
        }
        else if ( preDelim )
            dst.append( postDelimStr );
    }
}

#define ARRSTRPROP_ARRAY_TO_STRING(STRING,ARRAY) \
    wxPropertyGrid::ArrayStringToString(STRING,ARRAY,wxT('"'),wxT('"'),1);

void wxArrayStringProperty::GenerateValueAsString()
{
    wxArrayString arr = m_value.GetArrayString();
    ARRSTRPROP_ARRAY_TO_STRING(m_display, arr)
}

// Default implementation doesn't do anything.
bool wxArrayStringProperty::OnCustomStringEdit( wxWindow*, wxString& )
{
    return false;
}

wxArrayEditorDialog* wxArrayStringProperty::CreateEditorDialog()
{
    return new wxPGArrayStringEditorDialog();
}

bool wxArrayStringProperty::OnButtonClick( wxPropertyGrid* propGrid,
                                           wxWindow* WXUNUSED(primaryCtrl),
                                           const wxChar* cbt )
{
    // Update the value
    PrepareValueForDialogEditing(propGrid);

    if ( !propGrid->EditorValidate() )
        return false;

    // Create editor dialog.
    wxArrayEditorDialog* dlg = CreateEditorDialog();
#if wxUSE_VALIDATORS
    wxValidator* validator = GetValidator();
    wxPGInDialogValidator dialogValidator;
#endif

    wxPGArrayStringEditorDialog* strEdDlg = wxDynamicCast(dlg, wxPGArrayStringEditorDialog);

    if ( strEdDlg )
        strEdDlg->SetCustomButton(cbt, this);

    dlg->SetDialogValue( wxVariant(m_value) );
    dlg->Create(propGrid, wxEmptyString, m_label);

#if !wxPG_SMALL_SCREEN
    dlg->Move( propGrid->GetGoodEditorDialogPosition(this,dlg->GetSize()) );
#endif

    bool retVal;

    for (;;)
    {
        retVal = false;

        int res = dlg->ShowModal();

        if ( res == wxID_OK && dlg->IsModified() )
        {
            wxVariant value = dlg->GetDialogValue();
            if ( !value.IsNull() )
            {
                wxArrayString actualValue = value.GetArrayString();
                wxString tempStr;
                ARRSTRPROP_ARRAY_TO_STRING(tempStr, actualValue)
            #if wxUSE_VALIDATORS
                if ( dialogValidator.DoValidate( propGrid, validator, tempStr ) )
            #endif
                {
                    SetValueInEvent( actualValue );
                    retVal = true;
                    break;
                }
            }
            else
                break;
        }
        else
            break;
    }

    delete dlg;

    return retVal;
}

bool wxArrayStringProperty::OnEvent( wxPropertyGrid* propGrid,
                                     wxWindow* primary,
                                     wxEvent& event )
{
    if ( propGrid->IsMainButtonEvent(event) )
        return OnButtonClick(propGrid,primary,(const wxChar*) NULL);
    return false;
}

bool wxArrayStringProperty::StringToValue( wxVariant& variant, const wxString& text, int ) const
{
    wxArrayString arr;

    WX_PG_TOKENIZER2_BEGIN(text,wxT('"'))

        // Need to replace backslashes with empty characters
        // (opposite what is done in GenerateValueString).
        token.Replace ( wxT("\\\\"), wxT("\\"), true );

        arr.Add( token );

    WX_PG_TOKENIZER2_END()

    variant = arr;

    return true;
}

// -----------------------------------------------------------------------
// wxCustomProperty
// -----------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxCustomProperty, wxPGProperty)

const wxPGEditor* wxCustomProperty::DoGetEditorClass() const
{
    return wxPG_EDITOR(TextCtrl);
}

wxCustomProperty::wxCustomProperty( const wxString& label,
                                    const wxString& name )
    : wxPGProperty(label,name)
{
#ifdef wxPG_COMPATIBILITY_1_0_0
    m_callback = (wxPropertyGridCallback) NULL;
#endif
    m_paintCallback = (wxPGPaintCallback) NULL;
    m_value = wxPGVariant_EmptyString;  // Do this to avoid having 'unspecified' value
}

wxCustomProperty::~wxCustomProperty()
{
}

bool wxCustomProperty::StringToValue( wxVariant& variant, const wxString& text, int WXUNUSED(argFlags) ) const
{
    if ( text != m_value.GetString() )
    {
        variant = text;
        return true;
    }
    return false;
}

wxString wxCustomProperty::GetValueAsString( int /*argFlags*/ ) const
{
    return m_value;
}

// Need to do some extra event handling.
#ifdef wxPG_COMPATIBILITY_1_0_0
bool wxCustomProperty::OnEvent( wxPropertyGrid* propGrid, wxWindow* primary, wxEvent& event )
{
    if ( propGrid->IsMainButtonEvent(event) )
    {
        if ( m_callback )
            return m_callback(propGrid,this,primary,0);
    }
    return false;
}

#endif

wxSize wxCustomProperty::OnMeasureImage( int item ) const
{
    if ( m_paintCallback )
        return wxSize(-wxPG_CUSTOM_IMAGE_WIDTH,-wxPG_CUSTOM_IMAGE_WIDTH);

    return wxPGProperty::OnMeasureImage(item);
}

void wxCustomProperty::OnCustomPaint( wxDC& dc,
                                           const wxRect& rect,
                                           wxPGPaintData& paintData )
{
    if ( m_paintCallback )
        m_paintCallback(this,dc,rect,paintData);
    else
        wxPGProperty::OnCustomPaint(dc,rect,paintData);
}

bool wxCustomProperty::IntToValue( wxVariant& variant, int number, int ) const
{
    int index = m_choices.Index(number);
    if ( index == -1 )
        index = number;

    const wxString& sAtIndex = m_choices.GetLabel(index);
    if ( sAtIndex != m_value.GetString() )
    {
        variant = sAtIndex;
        return true;
    }

    return false;
}

int wxCustomProperty::GetChoiceInfo( wxPGChoiceInfo* choiceinfo )
{
    if ( choiceinfo )
        choiceinfo->m_choices = &m_choices;

    if ( !m_choices.IsOk() )
        return -1;

    return m_choices.Index(m_value.GetString());
}

bool wxCustomProperty::DoSetAttribute( const wxString& name, wxVariant& value )
{
#ifdef wxPG_COMPATIBILITY_1_0_0
    wxPropertyGrid* grid = GetGrid();
    if ( name == wxPG_CUSTOM_EDITOR )
    {
        if ( grid )
            grid->SetPropertyEditor( this, (wxPGEditor*) value.GetVoidPtr() );
        else
            SetEditor( (wxPGEditor*) value.GetVoidPtr() );
        return true;
    }
    else if ( name == wxPG_CUSTOM_IMAGE )
    {
        wxBitmap* bmp = (wxBitmap*) value.GetWxObjectPtr();
        if ( grid )
            grid->SetPropertyImage(this,*bmp);
        else
            SetValueImage(*bmp);
        return true;
    }
    else if ( name == wxPG_CUSTOM_CALLBACK )
    {
        m_callback = (wxPropertyGridCallback) value.GetVoidPtr();
        return true;
    }
    else
#endif
    if ( name == wxPG_CUSTOM_PAINT_CALLBACK )
    {
        void* voidValue = value.GetVoidPtr();
        m_paintCallback = (wxPGPaintCallback) voidValue;
        if ( voidValue )
            m_flags |= wxPG_PROP_CUSTOMIMAGE;
        else if ( !GetValueImage() )
            m_flags &= ~(wxPG_PROP_CUSTOMIMAGE);
        return true;
    }
    else
    if ( name == wxPG_CUSTOM_PRIVATE_CHILDREN )
    {
        if ( wxPGVariantToInt(value) )
        {
            SetFlag( wxPG_PROP_AGGREGATE );
        }
        else
        {
            ClearFlag( wxPG_PROP_AGGREGATE );
        }
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------

#if wxUSE_VALIDATORS
bool wxPGInDialogValidator::DoValidate( wxPropertyGrid* propGrid,
                                        wxValidator* validator,
                                        const wxString& value )
{
    if ( !validator )
        return true;

    wxTextCtrl* tc = m_textCtrl;

    if ( !tc )
    {
        {
            tc = new wxTextCtrl( propGrid, wxPG_SUBID_TEMP1, wxEmptyString,
                                 wxPoint(30000,30000));
            tc->Hide();
        }

        m_textCtrl = tc;
    }

    tc->SetValue(value);

    validator->SetWindow(tc);
    bool res = validator->Validate(propGrid);

    return res;
}
#else
bool wxPGInDialogValidator::DoValidate( wxPropertyGrid* WXUNUSED(propGrid),
                                        wxValidator* WXUNUSED(validator),
                                        const wxString& WXUNUSED(value) )
{
    return true;
}
#endif

// -----------------------------------------------------------------------

