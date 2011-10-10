/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal portion of wxPropertyGrid sample
// Author:      Jaakko Salli
// Modified by:
// Created:     Aug-23-2008
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/advprops.h"

#include <wx/datectrl.h>  // For the occasional wxDateProperty testing


class MyFrame : public wxFrame 
{ 
public: 
    MyFrame(wxWindow* parent); 
 
    void OnAction(wxCommandEvent& event); 
    void OnPropertyGridChange(wxPropertyGridEvent& event); 
    void OnPropertyGridChanging(wxPropertyGridEvent& event); 

private:
    wxPropertyGrid* m_pg;
    DECLARE_EVENT_TABLE()
};

//
// Called from propgridsample.cpp
//
void DisplayMinimalFrame(wxWindow* parent)
{
    MyFrame *frame = new MyFrame(parent);
    frame->Show(true);
}

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_HIGHEST+1, MyFrame::OnAction)
    EVT_PG_CHANGED( -1, MyFrame::OnPropertyGridChange )
    EVT_PG_CHANGING( -1, MyFrame::OnPropertyGridChanging )
END_EVENT_TABLE()

MyFrame::MyFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT("PropertyGrid Test"))
{
    wxMenu *Menu = new wxMenu; 
    Menu->Append(wxID_HIGHEST+1, wxT("Action")); 
    wxMenuBar *MenuBar = new wxMenuBar(); 
    MenuBar->Append(Menu, wxT("Action")); 
    SetMenuBar(MenuBar); 

    wxPropertyGrid *pg = new wxPropertyGrid(this,-1,wxDefaultPosition,wxSize(400,400),
                        wxPG_SPLITTER_AUTO_CENTER |
                        wxPG_BOLD_MODIFIED );
    m_pg = pg;

    //
    // Insert test code here
    //


    SetSize(400, 600);
}

void MyFrame::OnPropertyGridChange(wxPropertyGridEvent &event)
{
    wxPGProperty* p = event.GetProperty();

    if ( p )
        wxLogDebug(wxT("OnPropertyGridChange(%s, value=%s)"), 
                   p->GetName().c_str(), p->GetValueAsString().c_str());
    else
        wxLogDebug(wxT("OnPropertyGridChange(NULL)"));
}

void MyFrame::OnPropertyGridChanging(wxPropertyGridEvent &event)
{
    wxPGProperty* p = event.GetProperty();

    wxLogDebug(wxT("OnPropertyGridChanging(%s)"), p->GetName().c_str());
}

void MyFrame::OnAction(wxCommandEvent &) 
{
}
