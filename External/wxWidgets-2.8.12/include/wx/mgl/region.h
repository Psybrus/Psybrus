/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mgl/region.h
// Purpose:     wxRegion class
// Author:      Vaclav Slavik
// Created:     2001/03/12
// RCS-ID:      $Id: region.h 43286 2006-11-10 19:58:43Z ABX $
// Copyright:   (c) 2001-2002 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MGL_REGION_H_
#define _WX_MGL_REGION_H_

#include "wx/list.h"

class MGLRegion;

class WXDLLEXPORT wxRegion : public wxRegionBase
{
public:
    wxRegion();
    wxRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h);
    wxRegion(const wxPoint& topLeft, const wxPoint& bottomRight);
    wxRegion(const wxRect& rect);
    wxRegion(const MGLRegion& region);
    wxRegion(size_t n, const wxPoint *points, int fillStyle = wxODDEVEN_RULE );
    wxRegion(const wxBitmap& bmp)
    {
        Union(bmp);
    }
    wxRegion(const wxBitmap& bmp,
             const wxColour& transColour, int tolerance = 0)
    {
        Union(bmp, transColour, tolerance);
    }

    virtual ~wxRegion();

    // wxRegionBase methods
    virtual void Clear();
    virtual bool IsEmpty() const;

    // implementation from now on:
    const MGLRegion& GetMGLRegion() const;

protected:
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    // wxRegionBase pure virtuals
    virtual bool DoIsEqual(const wxRegion& region) const;
    virtual bool DoGetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const;
    virtual wxRegionContain DoContainsPoint(wxCoord x, wxCoord y) const;
    virtual wxRegionContain DoContainsRect(const wxRect& rect) const;

    virtual bool DoOffset(wxCoord x, wxCoord y);
    virtual bool DoUnionWithRect(const wxRect& rect);
    virtual bool DoUnionWithRegion(const wxRegion& region);
    virtual bool DoIntersect(const wxRegion& region);
    virtual bool DoSubtract(const wxRegion& region);
    virtual bool DoXor(const wxRegion& region);

private:
    DECLARE_DYNAMIC_CLASS(wxRegion);
    friend class WXDLLEXPORT wxRegionIterator;
};


WX_DECLARE_EXPORTED_LIST(wxRect, wxRegionRectList);

class WXDLLEXPORT wxRegionIterator : public wxObject
{
public:
    wxRegionIterator(void);
    wxRegionIterator(const wxRegion& region);
    virtual ~wxRegionIterator(void);

    void Reset(void) { m_currentNode = NULL; }
    void Reset(const wxRegion& region);

    operator bool (void) const { return (m_currentNode != NULL); }

    bool HaveRects(void) const { return (m_currentNode != NULL); }

    void operator ++ (void);
    void operator ++ (int);

    wxCoord GetX(void) const;
    wxCoord GetY(void) const;
    wxCoord GetW(void) const;
    wxCoord GetWidth(void) const { return GetW(); }
    wxCoord GetH(void) const;
    wxCoord GetHeight(void) const { return GetH(); }
    wxRect GetRect() const { return wxRect(GetX(), GetY(), GetWidth(), GetHeight()); }

private:
    wxRegionRectList         m_rects;
    wxRegionRectList::Node  *m_currentNode;

    DECLARE_DYNAMIC_CLASS(wxRegionIterator);
};

#endif // _WX_MGL_REGION_H_
