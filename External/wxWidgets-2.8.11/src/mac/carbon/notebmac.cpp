///////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/notebmac.cpp
// Purpose:     implementation of wxNotebook
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: notebmac.cpp 57849 2009-01-06 09:36:54Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#if wxUSE_NOTEBOOK

#include "wx/notebook.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/image.h"
#endif

#include "wx/string.h"
#include "wx/imaglist.h"
#include "wx/mac/uma.h"


// check that the page index is valid
#define IS_VALID_PAGE(nPage) ((nPage) < GetPageCount())


DEFINE_EVENT_TYPE(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING)

BEGIN_EVENT_TABLE(wxNotebook, wxBookCtrlBase)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, wxNotebook::OnSelChange)

    EVT_SIZE(wxNotebook::OnSize)
    EVT_SET_FOCUS(wxNotebook::OnSetFocus)
    EVT_NAVIGATION_KEY(wxNotebook::OnNavigationKey)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxNotebook, wxBookCtrlBase)
IMPLEMENT_DYNAMIC_CLASS(wxNotebookEvent, wxCommandEvent)


// common part of all ctors
void wxNotebook::Init()
{
    m_nSelection = -1;
}

// default for dynamic class
wxNotebook::wxNotebook()
{
    Init();
}

// the same arguments as for wxControl
wxNotebook::wxNotebook( wxWindow *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name )
{
    Init();

    Create( parent, id, pos, size, style, name );
}

bool wxNotebook::Create( wxWindow *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name )
{
    m_macIsUserPane = false ;

    if (! (style & wxBK_ALIGN_MASK))
        style |= wxBK_TOP;

    if ( !wxNotebookBase::Create( parent, id, pos, size, style, name ) )
        return false;

    Rect bounds = wxMacGetBoundsForControl( this, pos, size );

    if ( bounds.right <= bounds.left )
        bounds.right = bounds.left + 100;
    if ( bounds.bottom <= bounds.top )
        bounds.bottom = bounds.top + 100;

    UInt16 tabstyle = kControlTabDirectionNorth;
    if ( HasFlag(wxBK_LEFT) )
        tabstyle = kControlTabDirectionWest;
    else if ( HasFlag( wxBK_RIGHT ) )
        tabstyle = kControlTabDirectionEast;
    else if ( HasFlag( wxBK_BOTTOM ) )
        tabstyle = kControlTabDirectionSouth;

    ControlTabSize tabsize;
    switch (GetWindowVariant())
    {
        case wxWINDOW_VARIANT_MINI:
            if ( UMAGetSystemVersion() >= 0x1030 )
                tabsize = 3 ;
            else
                tabsize = kControlSizeSmall;
            break;

        case wxWINDOW_VARIANT_SMALL:
            tabsize = kControlTabSizeSmall;
            break;

        default:
            tabsize = kControlTabSizeLarge;
            break;
    }

    m_peer = new wxMacControl( this );
    OSStatus err = CreateTabsControl(
        MAC_WXHWND(parent->MacGetTopLevelWindowRef()), &bounds,
        tabsize, tabstyle, 0, NULL, m_peer->GetControlRefAddr() );
    verify_noerr( err );

    MacPostControlCreate( pos, size );

    return true ;
}

// dtor
wxNotebook::~wxNotebook()
{
}

// ----------------------------------------------------------------------------
// wxNotebook accessors
// ----------------------------------------------------------------------------

void wxNotebook::SetPadding(const wxSize& padding)
{
    // unsupported by OS
}

void wxNotebook::SetTabSize(const wxSize& sz)
{
    // unsupported by OS
}

void wxNotebook::SetPageSize(const wxSize& size)
{
    SetSize( CalcSizeFromPage( size ) );
}

wxSize wxNotebook::CalcSizeFromPage(const wxSize& sizePage) const
{
    return DoGetSizeFromClientSize( sizePage );
}

int wxNotebook::DoSetSelection(size_t nPage, int flags)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("DoSetSelection: invalid notebook page") );

    if ( m_nSelection == wxNOT_FOUND || nPage != (size_t)m_nSelection )
    {
        if ( flags & SetSelection_SendEvent )
        {
            if ( !SendPageChangingEvent(nPage) )
            {
                // vetoed by program
                return m_nSelection;
            }
            //else: program allows the page change

            SendPageChangedEvent(m_nSelection, nPage);
        }

        ChangePage(m_nSelection, nPage);
    }
    //else: no change

    return m_nSelection;
}

bool wxNotebook::SetPageText(size_t nPage, const wxString& strText)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), false, wxT("SetPageText: invalid notebook page") );

    wxNotebookPage *page = m_pages[nPage];
    page->SetLabel(wxStripMenuCodes(strText));
    MacSetupTabs();

    return true;
}

wxString wxNotebook::GetPageText(size_t nPage) const
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxEmptyString, wxT("GetPageText: invalid notebook page") );

    wxNotebookPage *page = m_pages[nPage];

    return page->GetLabel();
}

int wxNotebook::GetPageImage(size_t nPage) const
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("GetPageImage: invalid notebook page") );

    return m_images[nPage];
}

bool wxNotebook::SetPageImage(size_t nPage, int nImage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), false,
        wxT("SetPageImage: invalid notebook page") );
    wxCHECK_MSG( m_imageList && nImage < m_imageList->GetImageCount(), false,
        wxT("SetPageImage: invalid image index") );

    if ( nImage != m_images[nPage] )
    {
        // if the item didn't have an icon before or, on the contrary, did have
        // it but has lost it now, its size will change - but if the icon just
        // changes, it won't
        m_images[nPage] = nImage;

        MacSetupTabs() ;
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxNotebook operations
// ----------------------------------------------------------------------------

// remove one page from the notebook, without deleting the window
wxNotebookPage* wxNotebook::DoRemovePage(size_t nPage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), NULL,
        wxT("DoRemovePage: invalid notebook page") );

    wxNotebookPage* page = m_pages[nPage] ;
    m_pages.RemoveAt(nPage);

    MacSetupTabs();

    if (m_nSelection >= (int)GetPageCount())
        m_nSelection = GetPageCount() - 1;

    if (m_nSelection >= 0)
        m_pages[m_nSelection]->Show(true);

    InvalidateBestSize();

    return page;
}

// remove all pages
bool wxNotebook::DeleteAllPages()
{
    WX_CLEAR_ARRAY(m_pages) ;
    MacSetupTabs();
    m_nSelection = -1 ;
    InvalidateBestSize();

    return true;
}

// same as AddPage() but does it at given position
bool wxNotebook::InsertPage(size_t nPage,
    wxNotebookPage *pPage,
    const wxString& strText,
    bool bSelect,
    int imageId )
{
    if ( !wxNotebookBase::InsertPage( nPage, pPage, strText, bSelect, imageId ) )
        return false;

    wxASSERT_MSG( pPage->GetParent() == this, wxT("notebook pages must have notebook as parent") );

    // don't show pages by default (we'll need to adjust their size first)
    pPage->Show( false ) ;

    pPage->SetLabel( wxStripMenuCodes(strText) );

    m_images.Insert( imageId, nPage );

    MacSetupTabs();

    wxRect rect = GetPageRect() ;
    pPage->SetSize( rect );
    if ( pPage->GetAutoLayout() )
        pPage->Layout();

    // now deal with the selection
    // ---------------------------

    // if the inserted page is before the selected one, we must update the
    // index of the selected page

    if ( int(nPage) <= m_nSelection )
    {
        m_nSelection++;

        // while this still is the same page showing, we need to update the tabs
        m_peer->SetValue( m_nSelection + 1 ) ;
    }

    // some page should be selected: either this one or the first one if there
    // is still no selection
    int selNew = -1;
    if ( bSelect )
        selNew = nPage;
    else if ( m_nSelection == -1 )
        selNew = 0;

    if ( selNew != -1 )
        SetSelection( selNew );

    InvalidateBestSize();

    return true;
}

int wxNotebook::HitTest(const wxPoint& pt, long * flags) const
{
    int resultV = wxNOT_FOUND;

#if TARGET_API_MAC_OSX
    const int countPages = GetPageCount();

    // we have to convert from Client to Window relative coordinates
    wxPoint adjustedPt = pt + GetClientAreaOrigin();
    // and now to HIView native ones
    adjustedPt.x -= MacGetLeftBorderSize() ;
    adjustedPt.y -= MacGetTopBorderSize() ;

    HIPoint hipoint= { adjustedPt.x , adjustedPt.y } ;
    HIViewPartCode outPart = 0 ;
    OSStatus err = HIViewGetPartHit( m_peer->GetControlRef(), &hipoint, &outPart );

    int max = m_peer->GetMaximum() ;
    if ( outPart == 0 && max > 0 )
    {
        // this is a hack, as unfortunately a hit on an already selected tab returns 0,
        // so we have to go some extra miles to make sure we select something different
        // and try again ..
        int val = m_peer->GetValue() ;
        int maxval = max ;
        if ( max == 1 )
        {
            m_peer->SetMaximum( 2 ) ;
            maxval = 2 ;
        }

        if ( val == 1 )
            m_peer->SetValue( maxval ) ;
        else
             m_peer->SetValue( 1 ) ;

        err = HIViewGetPartHit( m_peer->GetControlRef(), &hipoint, &outPart );

        m_peer->SetValue( val ) ;
        if ( max == 1 )
            m_peer->SetMaximum( 1 ) ;
    }

    if ( outPart >= 1 && outPart <= countPages )
        resultV = outPart - 1 ;
#endif // TARGET_API_MAC_OSX

    if (flags != NULL)
    {
        *flags = 0;

        // we cannot differentiate better
        if (resultV >= 0)
            *flags |= wxBK_HITTEST_ONLABEL;
        else
            *flags |= wxBK_HITTEST_NOWHERE;
    }

    return resultV;
}

// Added by Mark Newsam
// When a page is added or deleted to the notebook this function updates
// information held in the control so that it matches the order
// the user would expect.
//
void wxNotebook::MacSetupTabs()
{
    m_peer->SetMaximum( GetPageCount() ) ;

    wxNotebookPage *page;
    ControlTabInfoRecV1 info;

    const size_t countPages = GetPageCount();
    for (size_t ii = 0; ii < countPages; ii++)
    {
        page = m_pages[ii];
        info.version = kControlTabInfoVersionOne;
        info.iconSuiteID = 0;
        wxMacCFStringHolder cflabel( page->GetLabel(), GetFont().GetEncoding() ) ;
        info.name = cflabel ;
        m_peer->SetData<ControlTabInfoRecV1>( ii + 1, kControlTabInfoTag, &info ) ;

        if ( GetImageList() && GetPageImage(ii) >= 0 && UMAGetSystemVersion() >= 0x1020 )
        {
            const wxBitmap bmap = GetImageList()->GetBitmap( GetPageImage( ii ) ) ;
            if ( bmap.Ok() )
            {
                ControlButtonContentInfo info ;

                wxMacCreateBitmapButton( &info, bmap ) ;

                OSStatus err = m_peer->SetData<ControlButtonContentInfo>( ii + 1, kControlTabImageContentTag, &info );
                wxASSERT_MSG( err == noErr , wxT("Error when setting icon on tab") ) ;

                wxMacReleaseBitmapButton( &info ) ;
            }
        }

        m_peer->SetTabEnabled( ii + 1, true ) ;
    }

#if wxMAC_USE_CORE_GRAPHICS
    Refresh();
#else
    Rect bounds;
    m_peer->GetRectInWindowCoords( &bounds ) ;
    InvalWindowRect( (WindowRef)MacGetTopLevelWindowRef(), &bounds );
#endif
}

wxRect wxNotebook::GetPageRect() const
{
    wxSize size = GetClientSize() ;

    return wxRect( 0 , 0 , size.x , size.y ) ;
}

// ----------------------------------------------------------------------------
// wxNotebook callbacks
// ----------------------------------------------------------------------------

// @@@ OnSize() is used for setting the font when it's called for the first
//     time because doing it in ::Create() doesn't work (for unknown reasons)
void wxNotebook::OnSize(wxSizeEvent& event)
{
    unsigned int nCount = m_pages.Count();
    wxRect rect = GetPageRect() ;

    for ( unsigned int nPage = 0; nPage < nCount; nPage++ )
    {
        wxNotebookPage *pPage = m_pages[nPage];
        pPage->SetSize(rect);
        if ( pPage->GetAutoLayout() )
            pPage->Layout();
    }

    // Processing continues to next OnSize
    event.Skip();
}

void wxNotebook::OnSelChange(wxNotebookEvent& event)
{
    // is it our tab control?
    if ( event.GetEventObject() == this )
        ChangePage(event.GetOldSelection(), event.GetSelection());

    // we want to give others a chance to process this message as well
    event.Skip();
}

void wxNotebook::OnSetFocus(wxFocusEvent& event)
{
    // set focus to the currently selected page if any
    if ( m_nSelection != -1 )
        m_pages[m_nSelection]->SetFocus();

    event.Skip();
}

void wxNotebook::OnNavigationKey(wxNavigationKeyEvent& event)
{
    if ( event.IsWindowChange() )
    {
        // change pages
        AdvanceSelection( event.GetDirection() );
    }
    else
    {
        // we get this event in 2 cases
        //
        // a) one of our pages might have generated it because the user TABbed
        // out from it in which case we should propagate the event upwards and
        // our parent will take care of setting the focus to prev/next sibling
        //
        // or
        //
        // b) the parent panel wants to give the focus to us so that we
        // forward it to our selected page. We can't deal with this in
        // OnSetFocus() because we don't know which direction the focus came
        // from in this case and so can't choose between setting the focus to
        // first or last panel child
        wxWindow *parent = GetParent();

        // the cast is here to fix a GCC ICE
        if ( ((wxWindow*)event.GetEventObject()) == parent )
        {
            // no, it doesn't come from child, case (b): forward to a page
            if ( m_nSelection != -1 )
            {
                // so that the page knows that the event comes from it's parent
                // and is being propagated downwards
                event.SetEventObject( this );

                wxWindow *page = m_pages[m_nSelection];
                if ( !page->GetEventHandler()->ProcessEvent( event ) )
                {
                    page->SetFocus();
                }
                //else: page manages focus inside it itself
            }
            else
            {
                // we have no pages - still have to give focus to _something_
                SetFocus();
            }
        }
        else
        {
            // it comes from our child, case (a), pass to the parent
            if ( parent )
            {
                event.SetCurrentFocus( this );
                parent->GetEventHandler()->ProcessEvent( event );
            }
        }
    }
}

// ----------------------------------------------------------------------------
// wxNotebook base class virtuals
// ----------------------------------------------------------------------------

#if wxUSE_CONSTRAINTS

// override these 2 functions to do nothing: everything is done in OnSize

void wxNotebook::SetConstraintSizes(bool WXUNUSED(recurse))
{
    // don't set the sizes of the pages - their correct size is not yet known
    wxControl::SetConstraintSizes( false );
}

bool wxNotebook::DoPhase(int WXUNUSED(nPhase))
{
    return true;
}

#endif // wxUSE_CONSTRAINTS

void wxNotebook::Command(wxCommandEvent& event)
{
    wxFAIL_MSG(wxT("wxNotebook::Command not implemented"));
}

// ----------------------------------------------------------------------------
// wxNotebook helper functions
// ----------------------------------------------------------------------------

// hide the currently active panel and show the new one
void wxNotebook::ChangePage(int nOldSel, int nSel)
{
    if (nOldSel == nSel)
        return;

    if ( nOldSel != -1 )
        m_pages[nOldSel]->Show( false );

    if ( nSel != -1 )
    {
        wxNotebookPage *pPage = m_pages[nSel];
        pPage->Show( true );
        pPage->SetFocus();
    }

    m_nSelection = nSel;
    m_peer->SetValue( m_nSelection + 1 ) ;
}

wxInt32 wxNotebook::MacControlHit(WXEVENTHANDLERREF WXUNUSED(handler) , WXEVENTREF WXUNUSED(event) )
{
    OSStatus status = eventNotHandledErr ;

    SInt32 newSel = m_peer->GetValue() - 1 ;
    if ( newSel != m_nSelection )
    {
        wxNotebookEvent changing(
            wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING, m_windowId,
            newSel , m_nSelection );
        changing.SetEventObject( this );
        GetEventHandler()->ProcessEvent( changing );

        if ( changing.IsAllowed() )
        {
            wxNotebookEvent event(
                wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, m_windowId,
                newSel, m_nSelection );
            event.SetEventObject( this );
            GetEventHandler()->ProcessEvent( event );
        }
        else
        {
            m_peer->SetValue( m_nSelection + 1 ) ;
        }

        status = noErr ;
    }

    return (wxInt32)status ;
}

#endif
