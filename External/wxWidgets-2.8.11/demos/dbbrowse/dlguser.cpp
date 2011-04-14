//----------------------------------------------------------------------------------------
// Name:        DlgUser.cpp
// Purpose:     Dialog mit Variable Gestaltung durch DlgUser.wxr
// Author:      Mark Johnson
// Modified by: 19991105.mj10777
// Created:     19991105
// Copyright:   (c) Mark Johnson
// Licence:     wxWindows license
// RCS-ID:      $Id: dlguser.cpp 44499 2007-02-13 23:22:24Z VZ $
//----------------------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
//----------------------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif
//----------------------------------------------------------------------------------------
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


//----------------------------------------------------------------------------------------
//-- all #includes that every .cpp needs             ----19990807.mj10777 ----------------
//----------------------------------------------------------------------------------------
#include "std.h"

//----------------------------------------------------------------------------------------
DlgUser::DlgUser(wxWindow *parent, MainDoc *p_Doc, const wxString& title) :
                wxDialog(parent, ID_DIALOG_DSN, title)
{
    int chSize;                          // Height of Font * 1.4 = Height of wxTextCtrl

    float ratio = (float)1.4;
#ifdef __WXMOTIF__
    ratio = (float)2.1;
#endif

    SetBackgroundColour(_T("wheat"));
    pDoc = p_Doc;
    wxLayoutConstraints* layout;
    SetAutoLayout(true);

    m_Label1 = new wxStaticText(this, wxID_ANY, _("User ID:"));
    m_Label1->SetFont(* pDoc->ft_Doc);
    layout = new wxLayoutConstraints;
    layout->left.SameAs(this, wxLeft, 10);
    layout->top.SameAs(this, wxTop, 10);
    layout->height.AsIs();
    layout->width.Absolute(75);
    m_Label1->SetConstraints(layout);

    int w;
    m_Label1->GetSize(&w, &chSize);

    m_UserName = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_UserName->SetFont(* pDoc->ft_Doc);
    chSize = (int) (m_UserName->GetCharHeight()*ratio);

    layout = new wxLayoutConstraints;
    layout->left.SameAs(m_Label1, wxRight, 10);
    layout->centreY.SameAs(m_Label1,wxCentreY);
    layout->width.Absolute(200);
    layout->height.Absolute(chSize);
    // layout->height.AsIs();
    m_UserName->SetConstraints(layout);


    m_Label2 = new wxStaticText(this, wxID_ANY, _("Password:"));
    m_Label2->SetFont(* pDoc->ft_Doc);
    layout = new wxLayoutConstraints;
    layout->left.SameAs(m_Label1, wxLeft);
    layout->top.SameAs(m_Label1, wxBottom, 10);
    layout->height.AsIs();
    layout->width.SameAs(m_Label1, wxWidth);
    m_Label2->SetConstraints(layout);

    m_Password = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    m_Password->SetFont(* pDoc->ft_Doc);
    layout = new wxLayoutConstraints;
    layout->left.SameAs(m_UserName, wxLeft);
    layout->width.SameAs(m_UserName, wxWidth);
    layout->centreY.SameAs(m_Label2,wxCentreY);
    layout->height.Absolute(chSize);
    //layout->height.AsIs();
    m_Password->SetConstraints(layout);

    m_OK = new wxButton(this, wxID_OK);
    m_OK->SetFont(* pDoc->ft_Doc);
    layout = new wxLayoutConstraints;
    layout->left.SameAs(this, wxLeft, 10);
    layout->top.SameAs(m_Label2, wxBottom,10);
    layout->height.AsIs();
    layout->width.Absolute(75);
    m_OK->SetConstraints(layout);

    m_Cancel = new wxButton(this, wxID_CANCEL);
    m_Cancel->SetFont(* pDoc->ft_Doc);
    layout = new wxLayoutConstraints;
    layout->left.SameAs(m_OK, wxRight, 10);
    layout->top.SameAs(m_OK, wxTop);
    layout->height.AsIs();
    layout->width.SameAs(m_OK, wxWidth);
    m_Cancel->SetConstraints(layout);

    m_OK->SetDefault();
    m_UserName->SetFocus();

    s_User = wxEmptyString;
    s_Password = wxEmptyString;
    Layout();
}

//----------------------------------------------------------------------------------------
void DlgUser::OnInit()
{
    wxString Temp; Temp.Printf(_T(">>> %s <<<"),s_DSN.c_str());
    SetTitle(Temp);
    m_UserName->SetLabel(s_User);
    m_Password->SetLabel(s_Password);
}

//----------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(DlgUser, wxDialog)
    EVT_BUTTON(wxID_OK, DlgUser::OnOk)
END_EVENT_TABLE()

//----------------------------------------------------------------------------------------
void DlgUser::OnOk(wxCommandEvent& WXUNUSED(event) )
{
    s_User    = m_UserName->GetValue();
    s_Password = m_Password->GetValue();
    EndModal(wxID_OK);
}

