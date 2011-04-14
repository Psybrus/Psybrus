///////////////////////////////////////////////////////////////////////////////
// Name:        statusbr.h
// Purpose:     native implementation of wxStatusBar. Optional; can use generic
//              version instead.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: statusbr.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_STATBAR_H_
#define   _WX_STATBAR_H_

class WXDLLEXPORT wxStatusBarMac : public wxStatusBarGeneric
{
  DECLARE_DYNAMIC_CLASS(wxStatusBarMac);

  wxStatusBarMac();
   wxStatusBarMac(wxWindow *parent, wxWindowID id,
           long style = 0,
           const wxString& name = wxPanelNameStr)
  {
      Create(parent, id, style, name);
  }

  virtual ~wxStatusBarMac();

  bool Create(wxWindow *parent, wxWindowID id,
              long style ,
              const wxString& name = wxPanelNameStr) ;

  virtual void DrawFieldText(wxDC& dc, int i);
  virtual void DrawField(wxDC& dc, int i);
  virtual void SetStatusText(const wxString& text, int number = 0) ;
  ////////////////////////////////////////////////////////////////////////
  // Implementation

  virtual void MacSuperEnabled( bool enable ) ;

  void OnPaint(wxPaintEvent& event);
protected:

  DECLARE_EVENT_TABLE()
};

#endif
    // _WX_STATBAR_H_
