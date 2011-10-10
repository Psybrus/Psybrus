/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/printwin.cpp
// Purpose:     wxMacPrinter framework
// Author:      Julian Smart
// Modified by: Stefan Csomor
// Created:     04/01/98
// RCS-ID:      $Id: printmac.cpp 58882 2009-02-13 16:05:12Z SC $
// Copyright:   (c) Julian Smart, Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_PRINTING_ARCHITECTURE

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/dc.h"
    #include "wx/app.h"
    #include "wx/msgdlg.h"
    #include "wx/dcprint.h"
    #include "wx/math.h"
#endif

#include "wx/mac/uma.h"

#include "wx/mac/printmac.h"
#include "wx/mac/private/print.h"

#include "wx/printdlg.h"
#include "wx/paper.h"
#include "wx/mac/printdlg.h"

#include <stdlib.h>

IMPLEMENT_DYNAMIC_CLASS(wxMacCarbonPrintData, wxPrintNativeDataBase)
IMPLEMENT_DYNAMIC_CLASS(wxMacPrinter, wxPrinterBase)
IMPLEMENT_CLASS(wxMacPrintPreview, wxPrintPreviewBase)

bool wxMacCarbonPrintData::IsOk() const
{
    return (m_macPageFormat != kPMNoPageFormat) && (m_macPrintSettings != kPMNoPrintSettings) && (m_macPrintSession != kPMNoReference);
}
wxMacCarbonPrintData::wxMacCarbonPrintData()
{
    m_macPageFormat = kPMNoPageFormat;
    m_macPrintSettings = kPMNoPrintSettings;
    m_macPrintSession = kPMNoReference ;
    m_macPaper = kPMNoData;
    
    if ( PMCreateSession( &m_macPrintSession ) == noErr )
    {
        if ( PMCreatePageFormat(&m_macPageFormat) == noErr )
        {
            PMSessionDefaultPageFormat(m_macPrintSession,
                    m_macPageFormat);
            PMGetPageFormatPaper(m_macPageFormat, &m_macPaper);
        }
        
        if ( PMCreatePrintSettings(&m_macPrintSettings) == noErr )
        {
            PMSessionDefaultPrintSettings(m_macPrintSession,
                m_macPrintSettings);
        }
    }
}

wxMacCarbonPrintData::~wxMacCarbonPrintData()
{
    (void)PMRelease(m_macPageFormat);
    (void)PMRelease(m_macPrintSettings);
    (void)PMRelease(m_macPrintSession);
}

bool wxMacCarbonPrintData::TransferFrom( const wxPrintData &data )
{
    PMPrinter printer;
    PMSessionGetCurrentPrinter(m_macPrintSession, &printer);

    wxSize papersize = wxDefaultSize;
    const wxPaperSize paperId = data.GetPaperId();
    if ( paperId != wxPAPER_NONE && wxThePrintPaperDatabase )
    {
        papersize = wxThePrintPaperDatabase->GetSize(paperId);
        if ( papersize != wxDefaultSize )
        {
            papersize.x /= 10;
            papersize.y /= 10;
        }
    }
    else
    {
        papersize = data.GetPaperSize();
    }
    
    if ( papersize != wxDefaultSize )
    {
        papersize.x *= mm2pt;
        papersize.y *= mm2pt;
        
        double height, width;
        PMPaperGetHeight(m_macPaper, &height);
        PMPaperGetWidth(m_macPaper, &width);
        
        if ( fabs( width - papersize.x ) >= 5 || 
            fabs( height - papersize.y ) >= 5 )
        {
            // we have to change the current paper
            CFArrayRef paperlist = 0 ;
            if ( PMPrinterGetPaperList( printer, &paperlist ) == noErr )
            {
                PMPaper bestPaper = kPMNoData ;
                CFIndex top = CFArrayGetCount(paperlist);
                for ( CFIndex i = 0 ; i < top ; ++ i )
                {
                    PMPaper paper = (PMPaper) CFArrayGetValueAtIndex( paperlist, i );
                    PMPaperGetHeight(paper, &height);
                    PMPaperGetWidth(paper, &width);
                    if ( fabs( width - papersize.x ) < 5 && 
                        fabs( height - papersize.y ) < 5 )
                    {
                        // TODO test for duplicate hits and use additional
                        // criteria for best match
                        bestPaper = paper;
                    }
                }
                if ( bestPaper != kPMNoData )
                {
                    PMPageFormat pageFormat;
                    PMCreatePageFormatWithPMPaper(&pageFormat, bestPaper);
                    PMCopyPageFormat( pageFormat, m_macPageFormat );
                    PMRelease(pageFormat);
                    PMGetPageFormatPaper(m_macPageFormat, &m_macPaper);
                }
            }
        }
    }

    PMSetCopies( m_macPrintSettings , data.GetNoCopies() , false ) ;
    PMSetCollate(m_macPrintSettings, data.GetCollate());
    if ( data.IsOrientationReversed() )
        PMSetOrientation( m_macPageFormat , ( data.GetOrientation() == wxLANDSCAPE ) ?
            kPMReverseLandscape : kPMReversePortrait , false ) ;
    else
        PMSetOrientation( m_macPageFormat , ( data.GetOrientation() == wxLANDSCAPE ) ?
            kPMLandscape : kPMPortrait , false ) ;
    // collate cannot be set
#if 0 // not yet tested
    if ( !m_printerName.empty() )
        PMSessionSetCurrentPrinter( m_macPrintSession , wxMacCFStringHolder( m_printerName , wxFont::GetDefaultEncoding() ) ) ;
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if ( &PMSetDuplex!=NULL )
    {
        PMDuplexMode mode = 0 ;
        switch( data.GetDuplex() )
        {
            case wxDUPLEX_HORIZONTAL :
                mode = kPMDuplexNoTumble ;
                break ;
            case wxDUPLEX_VERTICAL :
                mode = kPMDuplexTumble ;
                break ;
            case wxDUPLEX_SIMPLEX :
            default :
                mode = kPMDuplexNone ;
                break ;
        }
        PMSetDuplex( m_macPrintSettings, mode ) ;
    }
#endif    
    // PMQualityMode not accessible via API
    // TODO: use our quality property to determine optimal resolution
    PMResolution res;
    PMTag tag = kPMMaxSquareResolution;
    PMPrinterGetPrinterResolution(printer, tag, &res);
    PMSetResolution(m_macPageFormat, &res);

    // after setting the new resolution the format has to be updated, otherwise the page rect remains 
    // at the 'old' scaling
    PMSessionValidatePageFormat(m_macPrintSession,
        m_macPageFormat, kPMDontWantBoolean);
    PMSessionValidatePrintSettings(m_macPrintSession,
        m_macPrintSettings, kPMDontWantBoolean);

    return true ;
}

bool wxMacCarbonPrintData::TransferTo( wxPrintData &data )
{
    OSStatus err = noErr ;

    UInt32 copies ;
    err = PMGetCopies( m_macPrintSettings , &copies ) ;
    if ( err == noErr )
        data.SetNoCopies( copies ) ;

    PMOrientation orientation ;
    err = PMGetOrientation( m_macPageFormat , &orientation ) ;
    if ( err == noErr )
    {
        if ( orientation == kPMPortrait || orientation == kPMReversePortrait )
        {
            data.SetOrientation( wxPORTRAIT  );
            data.SetOrientationReversed( orientation == kPMReversePortrait );
        }
        else
        {
            data.SetOrientation( wxLANDSCAPE );
            data.SetOrientationReversed( orientation == kPMReverseLandscape );
        }
    }

    Boolean collate;
    if (PMGetCollate(m_macPrintSettings, &collate) == noErr)
        data.SetCollate(collate);
#if 0
    {
        wxMacCFStringHolder name ;
        PMPrinter printer ;
        PMSessionGetCurrentPrinter( m_macPrintSession ,
            &printer ) ;
        m_printerName = name.AsString() ;
    }
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if ( &PMGetDuplex!=NULL )
    {
        PMDuplexMode mode = 0 ;
        PMGetDuplex( m_macPrintSettings, &mode ) ;
        switch( mode )
        {
            case kPMDuplexNoTumble :
                data.SetDuplex(wxDUPLEX_HORIZONTAL);
                break ;
            case kPMDuplexTumble :
                data.SetDuplex(wxDUPLEX_VERTICAL);
                break ;
            case kPMDuplexNone :
            default :
                data.SetDuplex(wxDUPLEX_SIMPLEX);
                break ;
        }
    }
#endif
    // PMQualityMode not yet accessible via API
    
    double height, width;
    PMPaperGetHeight(m_macPaper, &height);
    PMPaperGetWidth(m_macPaper, &width);

    wxSize sz((int)(width * pt2mm + 0.5 ) ,
         (int)(height * pt2mm + 0.5 ));
    data.SetPaperSize(sz);
    wxPaperSize id = wxThePrintPaperDatabase->GetSize(wxSize(sz.x* 10, sz.y * 10));
    if (id != wxPAPER_NONE)
    {
        data.SetPaperId(id);
    }
    return true ;
}

void wxMacCarbonPrintData::TransferFrom( wxPageSetupData *data )
{
    // should we setup the page rect here ?
    // since MacOS sometimes has two same paper rects with different
    // page rects we could make it roundtrip safe perhaps
}

void wxMacCarbonPrintData::TransferTo( wxPageSetupData* data )
{
    PMRect rPaper;
    OSStatus err = PMGetUnadjustedPaperRect(m_macPageFormat, &rPaper);
    if ( err == noErr )
    {
        wxSize sz((int)(( rPaper.right - rPaper.left ) * pt2mm + 0.5 ) ,
             (int)(( rPaper.bottom - rPaper.top ) * pt2mm + 0.5 ));
        data->SetPaperSize(sz);

        PMRect rPage ;
        err = PMGetUnadjustedPageRect(m_macPageFormat , &rPage ) ;
        if ( err == noErr )
        {
            data->SetMinMarginTopLeft( wxPoint (
                (int)(((double) rPage.left - rPaper.left ) * pt2mm) ,
                (int)(((double) rPage.top - rPaper.top ) * pt2mm) ) ) ;

            data->SetMinMarginBottomRight( wxPoint (
                (wxCoord)(((double) rPaper.right - rPage.right ) * pt2mm),
                (wxCoord)(((double) rPaper.bottom - rPage.bottom ) * pt2mm)) ) ;

            if ( data->GetMarginTopLeft().x < data->GetMinMarginTopLeft().x )
                data->SetMarginTopLeft( wxPoint( data->GetMinMarginTopLeft().x ,
                    data->GetMarginTopLeft().y ) ) ;

            if ( data->GetMarginBottomRight().x < data->GetMinMarginBottomRight().x )
                data->SetMarginBottomRight( wxPoint( data->GetMinMarginBottomRight().x ,
                    data->GetMarginBottomRight().y ) );

            if ( data->GetMarginTopLeft().y < data->GetMinMarginTopLeft().y )
                data->SetMarginTopLeft( wxPoint( data->GetMarginTopLeft().x , data->GetMinMarginTopLeft().y ) );

            if ( data->GetMarginBottomRight().y < data->GetMinMarginBottomRight().y )
                data->SetMarginBottomRight( wxPoint( data->GetMarginBottomRight().x ,
                    data->GetMinMarginBottomRight().y) );
        }
    }
}

void wxMacCarbonPrintData::TransferTo( wxPrintDialogData* data )
{
    UInt32 minPage , maxPage ;
    PMGetPageRange( m_macPrintSettings , &minPage , &maxPage ) ;
    data->SetMinPage( minPage ) ;
    data->SetMaxPage( maxPage ) ;
    UInt32 copies ;
    PMGetCopies( m_macPrintSettings , &copies ) ;
    data->SetNoCopies( copies ) ;
    UInt32 from , to ;
    PMGetFirstPage( m_macPrintSettings , &from ) ;
    PMGetLastPage( m_macPrintSettings , &to ) ;
    if ( to >= 0x7FFFFFFF ) //  due to an OS Bug we don't get back kPMPrintAllPages
    {
        data->SetAllPages( true ) ;
        // This means all pages, more or less
        data->SetFromPage(1);
        data->SetToPage(9999);
    }
    else
    {
        data->SetFromPage( from ) ;
        data->SetToPage( to ) ;
        data->SetAllPages( false );
    }
}

void wxMacCarbonPrintData::TransferFrom( wxPrintDialogData* data )
{
    PMSetPageRange( m_macPrintSettings , data->GetMinPage() , data->GetMaxPage() ) ;
    PMSetCopies( m_macPrintSettings , data->GetNoCopies() , false ) ;
    PMSetFirstPage( m_macPrintSettings , data->GetFromPage() , false ) ;

    if (data->GetAllPages() || data->GetFromPage() == 0)
        PMSetLastPage( m_macPrintSettings , (UInt32) kPMPrintAllPages, true ) ;
    else
        PMSetLastPage( m_macPrintSettings , (UInt32) data->GetToPage() , false ) ;
}

/*
* Printer
*/

wxMacPrinter::wxMacPrinter(wxPrintDialogData *data):
wxPrinterBase(data)
{
}

wxMacPrinter::~wxMacPrinter(void)
{
}

bool wxMacPrinter::Print(wxWindow *parent, wxPrintout *printout, bool prompt)
{
    sm_abortIt = false;
    sm_abortWindow = NULL;

    if (!printout)
        return false;

    printout->SetIsPreview(false);
    if (m_printDialogData.GetMinPage() < 1)
        m_printDialogData.SetMinPage(1);
    if (m_printDialogData.GetMaxPage() < 1)
        m_printDialogData.SetMaxPage(9999);

    // Create a suitable device context
    wxPrinterDC *dc = NULL;
    if (prompt)
    {
        wxMacPrintDialog dialog(parent, & m_printDialogData);
        if (dialog.ShowModal() == wxID_OK)
        {
            dc = wxDynamicCast(dialog.GetPrintDC(), wxPrinterDC);
            wxASSERT(dc);
            m_printDialogData = dialog.GetPrintDialogData();
        }
    }
    else
    {
        dc = new wxPrinterDC( m_printDialogData.GetPrintData() ) ;
    }

    // May have pressed cancel.
    if (!dc || !dc->Ok())
    {
        if (dc)
            delete dc;
        return false;
    }

    // on the mac we have always pixels as addressing mode with 72 dpi
    printout->SetPPIScreen(72, 72);
    PMResolution res;
    wxMacCarbonPrintData* nativeData = (wxMacCarbonPrintData*)
          (m_printDialogData.GetPrintData().GetNativeData());
    PMGetResolution((nativeData->m_macPageFormat), &res);
    printout->SetPPIPrinter(int(res.hRes), int(res.vRes));

    // Set printout parameters
    printout->SetDC(dc);

    int w, h;
    dc->GetSize(&w, &h);
    printout->SetPageSizePixels((int)w, (int)h);
    printout->SetPaperRectPixels(dc->GetPaperRect());
    wxCoord mw, mh;
    dc->GetSizeMM(&mw, &mh);
    printout->SetPageSizeMM((int)mw, (int)mh);

    // Create an abort window
    wxBeginBusyCursor();

    printout->OnPreparePrinting();

    // Get some parameters from the printout, if defined
    int fromPage, toPage;
    int minPage, maxPage;
    printout->GetPageInfo(&minPage, &maxPage, &fromPage, &toPage);

    if (maxPage == 0)
    {
        wxEndBusyCursor();
        return false;
    }

    // Only set min and max, because from and to have been
    // set by the user
    m_printDialogData.SetMinPage(minPage);
    m_printDialogData.SetMaxPage(maxPage);

    wxWindow *win = CreateAbortWindow(parent, printout);
    wxSafeYield(win,true);

    if (!win)
    {
        wxEndBusyCursor();
        wxMessageBox(wxT("Sorry, could not create an abort dialog."), wxT("Print Error"), wxOK, parent);
        delete dc;

        return false;
    }

    sm_abortWindow = win;
    sm_abortWindow->Show(true);
    wxSafeYield(win,true);

    printout->OnBeginPrinting();

    bool keepGoing = true;

    int copyCount;
    for (copyCount = 1; copyCount <= m_printDialogData.GetNoCopies(); copyCount ++)
    {
        if (!printout->OnBeginDocument(m_printDialogData.GetFromPage(), m_printDialogData.GetToPage()))
        {
            wxEndBusyCursor();
            wxMessageBox(wxT("Could not start printing."), wxT("Print Error"), wxOK, parent);
            break;
        }
        if (sm_abortIt)
            break;

        int pn;
        for (pn = m_printDialogData.GetFromPage();
        keepGoing && (pn <= m_printDialogData.GetToPage()) && printout->HasPage(pn);
        pn++)
        {
            if (sm_abortIt)
            {
                keepGoing = false;
                break;
            }
            else
            {
#if TARGET_CARBON
                if ( UMAGetSystemVersion() >= 0x1000 )
#endif
                {
#if !wxMAC_USE_CORE_GRAPHICS
                    GrafPtr thePort ;
                    GetPort( &thePort ) ;
#endif
                    wxSafeYield(win,true);
#if !wxMAC_USE_CORE_GRAPHICS
                    SetPort( thePort ) ;
#endif
                }
                dc->StartPage();
                keepGoing = printout->OnPrintPage(pn);
                dc->EndPage();
            }
        }
        printout->OnEndDocument();
    }

    printout->OnEndPrinting();

    if (sm_abortWindow)
    {
        sm_abortWindow->Show(false);
        delete sm_abortWindow;
        sm_abortWindow = NULL;
    }

    wxEndBusyCursor();

    delete dc;

    return true;
}

wxDC* wxMacPrinter::PrintDialog(wxWindow *parent)
{
    wxDC* dc = (wxDC*) NULL;

    wxPrintDialog dialog(parent, & m_printDialogData);
    int ret = dialog.ShowModal();

    if (ret == wxID_OK)
    {
        dc = dialog.GetPrintDC();
        m_printDialogData = dialog.GetPrintDialogData();
    }

    return dc;
}

bool wxMacPrinter::Setup(wxWindow *parent)
{
#if 0
    wxPrintDialog dialog(parent, & m_printDialogData);
    dialog.GetPrintDialogData().SetSetupDialog(true);

    int ret = dialog.ShowModal();

    if (ret == wxID_OK)
        m_printDialogData = dialog.GetPrintDialogData();

    return (ret == wxID_OK);
#endif

    return wxID_CANCEL;
}

/*
* Print preview
*/

wxMacPrintPreview::wxMacPrintPreview(wxPrintout *printout,
                                     wxPrintout *printoutForPrinting,
                                     wxPrintDialogData *data)
                                     : wxPrintPreviewBase(printout, printoutForPrinting, data)
{
    DetermineScaling();
}

wxMacPrintPreview::wxMacPrintPreview(wxPrintout *printout, wxPrintout *printoutForPrinting, wxPrintData *data):
wxPrintPreviewBase(printout, printoutForPrinting, data)
{
    DetermineScaling();
}

wxMacPrintPreview::~wxMacPrintPreview(void)
{
}

bool wxMacPrintPreview::Print(bool interactive)
{
    if (!m_printPrintout)
        return false;

    wxMacPrinter printer(&m_printDialogData);
    return printer.Print(m_previewFrame, m_printPrintout, interactive);
}

void wxMacPrintPreview::DetermineScaling(void)
{
    int screenWidth , screenHeight ;
    wxDisplaySize( &screenWidth , &screenHeight ) ;

    wxSize ppiScreen( 72 , 72 ) ;
    wxSize ppiPrinter( 72 , 72 ) ;
    
    // Note that with Leopard, screen dpi=72 is no longer a given
    m_previewPrintout->SetPPIScreen( ppiScreen.x , ppiScreen.y ) ;
    
    wxCoord w , h ;
    wxCoord ww, hh;
    wxRect paperRect;

    // Get a device context for the currently selected printer
    wxPrinterDC printerDC(m_printDialogData.GetPrintData());
    if (printerDC.Ok())
    {
        printerDC.GetSizeMM(&ww, &hh);
        printerDC.GetSize( &w , &h ) ;
        ppiPrinter = printerDC.GetPPI() ;
        paperRect = printerDC.GetPaperRect();
        m_isOk = true ;
    }
    else
    {
        // use some defaults
        w = 8 * 72 ;
        h = 11 * 72 ;
        ww = (wxCoord) (w * 25.4 / ppiPrinter.x) ;
        hh = (wxCoord) (h * 25.4 / ppiPrinter.y) ;
        paperRect = wxRect(0, 0, w, h);
        m_isOk = false ;
    }
    m_pageWidth = w;
    m_pageHeight = h;
    
    m_previewPrintout->SetPageSizePixels(w , h) ;
    m_previewPrintout->SetPageSizeMM(ww, hh);
    m_previewPrintout->SetPaperRectPixels(paperRect);
    m_previewPrintout->SetPPIPrinter( ppiPrinter.x , ppiPrinter.y ) ;

    m_previewScaleX = float(ppiScreen.x) / ppiPrinter.x;
    m_previewScaleY = float(ppiScreen.y) / ppiPrinter.y;
}

#endif
