/////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/pen.h
// Purpose:     wxPen class
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id: pen.h 42776 2006-10-30 22:03:53Z VZ $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PEN_H_
#define _WX_PEN_H_

#include "wx/gdiobj.h"
#include "wx/bitmap.h"
#include "wx/colour.h"

typedef WXDWORD wxMSWDash;

class WXDLLEXPORT wxPen;

// VZ: this class should be made private
class WXDLLEXPORT wxPenRefData : public wxGDIRefData
{
public:
    wxPenRefData();
    wxPenRefData(const wxPenRefData& data);
    virtual ~wxPenRefData();

    bool operator==(const wxPenRefData& data) const
    {
        // we intentionally don't compare m_hPen fields here
        return m_style == data.m_style &&
               m_width == data.m_width &&
               m_join == data.m_join &&
               m_cap == data.m_cap &&
               m_colour == data.m_colour &&
               (m_style != wxSTIPPLE || m_stipple.IsSameAs(data.m_stipple)) &&
               (m_style != wxUSER_DASH ||
                (m_nbDash == data.m_nbDash &&
                    memcmp(m_dash, data.m_dash, m_nbDash*sizeof(wxDash)) == 0));
    }

protected:
    int           m_width;
    int           m_style;
    int           m_join;
    int           m_cap;
    wxBitmap      m_stipple;
    int           m_nbDash;
    wxDash *      m_dash;
    wxColour      m_colour;
    WXHPEN        m_hPen;

private:
    friend class WXDLLEXPORT wxPen;

    // Cannot use
    //  DECLARE_NO_COPY_CLASS(wxPenRefData)
    // because copy constructor is explicitly declared above;
    // but no copy assignment operator is defined, so declare
    // it private to prevent the compiler from defining it:
    wxPenRefData& operator=(const wxPenRefData&);
};

#define M_PENDATA ((wxPenRefData *)m_refData)
#define wxPENDATA(x) ((wxPenRefData *)(x).m_refData)

// ----------------------------------------------------------------------------
// Pen
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxPen : public wxGDIObject
{
public:
    wxPen();
    wxPen(const wxColour& col, int width = 1, int style = wxSOLID);
    wxPen(const wxBitmap& stipple, int width);
    virtual ~wxPen();

    bool operator==(const wxPen& pen) const
    {
        const wxPenRefData *penData = (wxPenRefData *)pen.m_refData;

        // an invalid pen is only equal to another invalid pen
        return m_refData ? penData && *M_PENDATA == *penData : !penData;
    }

    bool operator!=(const wxPen& pen) const { return !(*this == pen); }

    virtual bool Ok() const { return IsOk(); }
    virtual bool IsOk() const { return (m_refData != NULL); }

    // Override in order to recreate the pen
    void SetColour(const wxColour& col);
    void SetColour(unsigned char r, unsigned char g, unsigned char b);

    void SetWidth(int width);
    void SetStyle(int style);
    void SetStipple(const wxBitmap& stipple);
    void SetDashes(int nb_dashes, const wxDash *dash);
    void SetJoin(int join);
    void SetCap(int cap);

    wxColour& GetColour() const { return (M_PENDATA ? M_PENDATA->m_colour : wxNullColour); };
    int GetWidth() const { return (M_PENDATA ? M_PENDATA->m_width : 0); };
    int GetStyle() const { return (M_PENDATA ? M_PENDATA->m_style : 0); };
    int GetJoin() const { return (M_PENDATA ? M_PENDATA->m_join : 0); };
    int GetCap() const { return (M_PENDATA ? M_PENDATA->m_cap : 0); };
    int GetDashes(wxDash **ptr) const
    {
        *ptr = (M_PENDATA ? (wxDash*)M_PENDATA->m_dash : (wxDash*) NULL);
        return (M_PENDATA ? M_PENDATA->m_nbDash : 0);
    }
    wxDash* GetDash() const { return (M_PENDATA ? (wxDash*)M_PENDATA->m_dash : (wxDash*)NULL); };
    inline int GetDashCount() const { return (M_PENDATA ? M_PENDATA->m_nbDash : 0); };

    inline wxBitmap *GetStipple() const { return (M_PENDATA ? (& M_PENDATA->m_stipple) : (wxBitmap*) NULL); };

    // Internal
    bool RealizeResource();
    bool FreeResource(bool force = false);
    WXHANDLE GetResourceHandle() const;
    bool IsFree() const;
    void Unshare();

private:
    DECLARE_DYNAMIC_CLASS(wxPen)
};

#endif // _WX_PEN_H_
