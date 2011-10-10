/////////////////////////////////////////////////////////////////////////////
// Name:        drawnp.h
// Purpose:     Private header for wxDrawnShape
// Author:      Julian Smart
// Modified by:
// Created:     12/07/98
// RCS-ID:      $Id: drawnp.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _OGL_DRAWNP_H_
#define _OGL_DRAWNP_H_


/*
 * Drawing operations
 *
 */

#define  DRAWOP_SET_PEN             1
#define  DRAWOP_SET_BRUSH           2
#define  DRAWOP_SET_FONT            3
#define  DRAWOP_SET_TEXT_COLOUR     4
#define  DRAWOP_SET_BK_COLOUR       5
#define  DRAWOP_SET_BK_MODE         6
#define  DRAWOP_SET_CLIPPING_RECT   7
#define  DRAWOP_DESTROY_CLIPPING_RECT 8

/*
#define  DRAWOP_CREATE_PEN          10
#define  DRAWOP_CREATE_BRUSH        11
#define  DRAWOP_CREATE_FONT         12
*/

#define  DRAWOP_DRAW_LINE           20
#define  DRAWOP_DRAW_POLYLINE       21
#define  DRAWOP_DRAW_POLYGON        22
#define  DRAWOP_DRAW_RECT           23
#define  DRAWOP_DRAW_ROUNDED_RECT   24
#define  DRAWOP_DRAW_ELLIPSE        25
#define  DRAWOP_DRAW_POINT          26
#define  DRAWOP_DRAW_ARC            27
#define  DRAWOP_DRAW_TEXT           28
#define  DRAWOP_DRAW_SPLINE         29
#define  DRAWOP_DRAW_ELLIPTIC_ARC   30

/*
 * Base, virtual class
 *
 */

class WXDLLIMPEXP_OGL wxDrawOp: public wxObject
{
public:
  inline wxDrawOp(int theOp) { m_op = theOp; }
  inline ~wxDrawOp() {}
  inline virtual void Scale(double WXUNUSED(xScale), double WXUNUSED(yScale)) {};
  inline virtual void Translate(double WXUNUSED(x), double WXUNUSED(y)) {};
  inline virtual void Rotate(double WXUNUSED(x), double WXUNUSED(y), double WXUNUSED(theta), double WXUNUSED(sinTheta), double WXUNUSED(cosTheta)) {};
  virtual void Do(wxDC& dc, double xoffset, double yoffset) = 0;
  virtual wxDrawOp *Copy(wxPseudoMetaFile *newImage) = 0;
#if wxUSE_PROLOGIO
  virtual wxExpr *WriteExpr(wxPseudoMetaFile *image) = 0;
  virtual void ReadExpr(wxPseudoMetaFile *image, wxExpr *expr) = 0;
#endif
  inline int GetOp() const { return m_op; }

  // Draw an outline using the current operation. By default, return false (not drawn)
  virtual bool OnDrawOutline(wxDC& WXUNUSED(dc), double WXUNUSED(x), double WXUNUSED(y), double WXUNUSED(w), double WXUNUSED(h),
    double WXUNUSED(oldW), double WXUNUSED(oldH)) { return false; }

  // Get the perimeter point using this data
  virtual bool GetPerimeterPoint(double WXUNUSED(x1), double WXUNUSED(y1),
                                     double WXUNUSED(x2), double WXUNUSED(y2),
                                     double *WXUNUSED(x3), double *WXUNUSED(y3),
                                     double WXUNUSED(xOffset), double WXUNUSED(yOffset),
                                     int WXUNUSED(attachmentMode))
  { return false; }

protected:
  int           m_op;

};

/*
 * Set font, brush, text colour
 *
 */

class WXDLLIMPEXP_OGL wxOpSetGDI: public wxDrawOp
{
 public:
  wxOpSetGDI(int theOp, wxPseudoMetaFile *theImage, int theGdiIndex, int theMode = 0);
  void Do(wxDC& dc, double xoffset, double yoffset);
  wxDrawOp *Copy(wxPseudoMetaFile *newImage);
#if wxUSE_PROLOGIO
  wxExpr *WriteExpr(wxPseudoMetaFile *image);
  void ReadExpr(wxPseudoMetaFile *image, wxExpr *expr);
#endif

public:
  int               m_mode;
  int               m_gdiIndex;
  wxPseudoMetaFile* m_image;
  unsigned char     m_r;
  unsigned char     m_g;
  unsigned char     m_b;
};

/*
 * Set/destroy clipping
 *
 */

class WXDLLIMPEXP_OGL wxOpSetClipping: public wxDrawOp
{
public:
  wxOpSetClipping(int theOp, double theX1, double theY1, double theX2, double theY2);
  void Do(wxDC& dc, double xoffset, double yoffset);
  void Scale(double xScale, double yScale);
  void Translate(double x, double y);
  wxDrawOp *Copy(wxPseudoMetaFile *newImage);
#if wxUSE_PROLOGIO
  wxExpr *WriteExpr(wxPseudoMetaFile *image);
  void ReadExpr(wxPseudoMetaFile *image, wxExpr *expr);
#endif

public:
  double     m_x1;
  double     m_y1;
  double     m_x2;
  double     m_y2;
};

/*
 * Draw line, rectangle, rounded rectangle, ellipse, point, arc, text
 *
 */

class WXDLLIMPEXP_OGL wxOpDraw: public wxDrawOp
{
 public:
  wxOpDraw(int theOp, double theX1, double theY1, double theX2, double theY2,
         double radius = 0.0, const wxString& s = wxEmptyString);
  ~wxOpDraw();
  void Do(wxDC& dc, double xoffset, double yoffset);
  void Scale(double scaleX, double scaleY);
  void Translate(double x, double y);
  void Rotate(double x, double y, double theta, double sinTheta, double cosTheta);
  wxDrawOp *Copy(wxPseudoMetaFile *newImage);
#if wxUSE_PROLOGIO
  wxExpr *WriteExpr(wxPseudoMetaFile *image);
  void ReadExpr(wxPseudoMetaFile *image, wxExpr *expr);
#endif

public:
  double     m_x1;
  double     m_y1;
  double     m_x2;
  double     m_y2;
  double     m_x3;
  double     m_y3;
  double     m_radius;
  wxString   m_textString;

};

/*
 * Draw polyline, spline, polygon
 *
 */

class WXDLLIMPEXP_OGL wxOpPolyDraw: public wxDrawOp
{
public:
  wxOpPolyDraw(int theOp, int n, wxRealPoint *thePoints);
  ~wxOpPolyDraw();
  void Do(wxDC& dc, double xoffset, double yoffset);
  void Scale(double scaleX, double scaleY);
  void Translate(double x, double y);
  void Rotate(double x, double y, double theta, double sinTheta, double cosTheta);
  wxDrawOp *Copy(wxPseudoMetaFile *newImage);
#if wxUSE_PROLOGIO
  wxExpr *WriteExpr(wxPseudoMetaFile *image);
  void ReadExpr(wxPseudoMetaFile *image, wxExpr *expr);
#endif

  // Draw an outline using the current operation.
  virtual bool OnDrawOutline(wxDC& dc, double x, double y, double w, double h,
    double oldW, double oldH);

  // Get the perimeter point using this data
  bool GetPerimeterPoint(double x1, double y1,
                                     double x2, double y2,
                                     double *x3, double *y3,
                                     double xOffset, double yOffset,
                                     int attachmentMode);

public:
  wxRealPoint*  m_points;
  int           m_noPoints;

};

#endif
 // _OGL_DRAWNP_H_


