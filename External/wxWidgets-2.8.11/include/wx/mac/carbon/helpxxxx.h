/////////////////////////////////////////////////////////////////////////////
// Name:        helpxxxx.h
// Purpose:     Help system: native implementation for your system. Replace
//              XXXX with suitable name.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: helpxxxx.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPXXXX_H_
#define _WX_HELPXXXX_H_

#include "wx/wx.h"

#include "wx/helpbase.h"

class WXDLLEXPORT wxXXXXHelpController: public wxHelpControllerBase
{
  DECLARE_CLASS(wxXXXXHelpController)

 public:
  wxXXXXHelpController();
  virtual ~wxXXXXHelpController();

  // Must call this to set the filename and server name
  virtual bool Initialize(const wxString& file);

  // If file is "", reloads file given  in Initialize
  virtual bool LoadFile(const wxString& file = "");
  virtual bool DisplayContents();
  virtual bool DisplaySection(int sectionNo);
  virtual bool DisplayBlock(long blockNo);
  virtual bool KeywordSearch(const wxString& k,
                             wxHelpSearchMode mode = wxHELP_SEARCH_ALL);

  virtual bool Quit();
  virtual void OnQuit();

  inline wxString GetHelpFile() const { return m_helpFile; }

protected:
  wxString m_helpFile;
};

#endif
    // _WX_HELPXXXX_H_
