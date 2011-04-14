/////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/print.h
// Purpose:     wxPrinter, wxPrintPreview classes
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: print.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRINT_H_
#define _WX_PRINT_H_

#include "wx/prntbase.h"

/*
* Represents the printer: manages printing a wxPrintout object
*/

class WXDLLEXPORT wxPrinter: public wxPrinterBase
{
    DECLARE_DYNAMIC_CLASS(wxPrinter)

public:
    wxPrinter(wxPrintData *data = NULL);
    virtual ~wxPrinter();

    virtual bool Print(wxWindow *parent, wxPrintout *printout, bool prompt = true);
    virtual bool PrintDialog(wxWindow *parent);
    virtual bool Setup(wxWindow *parent);
};

/*
* wxPrintPreview
* Programmer creates an object of this class to preview a wxPrintout.
*/

class WXDLLEXPORT wxPrintPreview: public wxPrintPreviewBase
{
    DECLARE_CLASS(wxPrintPreview)

public:
    wxPrintPreview(wxPrintout *printout, wxPrintout *printoutForPrinting = NULL, wxPrintData *data = NULL);
    virtual ~wxPrintPreview();

    virtual bool Print(bool interactive);
    virtual void DetermineScaling();
};

#endif
// _WX_PRINT_H_
