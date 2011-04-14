/////////////////////////////////////////////////////////////////////////////
// Name:        resource.h
// Purpose:     Dialog resource sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: resource.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// Define a new application
class MyApp: public wxApp
{
public:
    MyApp();

    virtual bool OnInit();

    virtual ~MyApp();
};

class MyPanel: public wxPanel
{
public:
    MyPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos,
            const wxSize& size, int style, const wxString &name);
    void OnClick(wxMouseEvent &event);

private:
    DECLARE_EVENT_TABLE()
};

class MyFrame: public wxFrame
{
public:
    MyFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
            const wxPoint& pos, const wxSize& size);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnTestDialog(wxCommandEvent& event);

    wxWindow *panel;

private:
    DECLARE_EVENT_TABLE()
};

class MyDialog : public wxDialog
{
public:
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

// the values should be the same as in menu.wxr file!
enum
{
    RESOURCE_TESTDIALOG = 2,
    RESOURCE_QUIT = 4,
    RESOURCE_ABOUT = 6
};
