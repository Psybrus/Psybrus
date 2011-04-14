/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mac/carbon/private/print.h
// Purpose:     private implementation for printing on MacOS
// Author:      Stefan Csomor
// Modified by:
// Created:     03/02/99
// RCS-ID:      $Id: print.h 58157 2009-01-16 19:21:13Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MAC_PRIVATE_PRINT_H_
#define _WX_MAC_PRIVATE_PRINT_H_

#include "wx/cmndata.h"
#include "wx/print.h"
#include "wx/mac/private.h"

#if TARGET_CARBON && !defined(__DARWIN__)
#  include <PMApplication.h>
#endif

#ifndef __DARWIN__
#  include "Printing.h"
#endif

#if !PM_USE_SESSION_APIS
#error "only Carbon Printing Session API is supported"
#endif

class WXDLLEXPORT wxMacCarbonPrintData : public wxPrintNativeDataBase
{
public :
                            wxMacCarbonPrintData() ;
    virtual                 ~wxMacCarbonPrintData() ;

    virtual bool            TransferTo( wxPrintData &data );
    virtual bool            TransferFrom( const wxPrintData &data );

    virtual bool            IsOk() const ;

    virtual void            TransferFrom( wxPageSetupDialogData * )  ;
    virtual void            TransferTo( wxPageSetupDialogData * ) ;

    virtual void            TransferFrom( wxPrintDialogData * )  ;
    virtual void            TransferTo( wxPrintDialogData * ) ;
public :
    PMPrintSession          m_macPrintSession ;
    PMPaper                 m_macPaper ;
    PMPageFormat            m_macPageFormat ;
    PMPrintSettings         m_macPrintSettings ;
private:
    DECLARE_DYNAMIC_CLASS(wxMacCarbonPrintData)
} ;

#endif // _WX_MAC_PRIVATE_PRINT_H_
