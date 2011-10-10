/////////////////////////////////////////////////////////////////////////////
// Name:        mmboard.cpp
// Purpose:     Multimedia Library sample
// Author:      Guilhem Lavaux (created from minimal by J. Smart)
// Modified by:
// Created:     13/02/2000
// RCS-ID:      $Id: mmboard.cpp 43163 2006-11-07 15:32:27Z VZ $
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// ressources
// ----------------------------------------------------------------------------

// the application icon
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "mondrian.xpm"
#endif

// include multimedia classes
#include "wx/mmedia/sndbase.h"
#ifdef __WIN32__
    #include "wx/mmedia/sndwin.h"
#endif
#ifdef __UNIX__
    #include "wx/mmedia/sndoss.h"
    #include "wx/mmedia/sndesd.h"
#endif

#include "wx/statline.h"
#include "wx/stattext.h"

// include personnal classes
#include "mmboard.h"
#include "mmbman.h"

#include "play.xpm"
#include "stop.xpm"
#include "eject.xpm"
#include "pause.xpm"

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Main Multimedia Board frame
class MMBoardFrame : public wxFrame
{
public:
    // ctor(s)
    MMBoardFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    // dtor
    ~MMBoardFrame();

    // event handlers
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnPlay(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);
    void OnEject(wxCommandEvent& event);
    void OnRefreshInfo(wxEvent& event);
    void OnSetPosition(wxCommandEvent& event);

    void OpenVideoWindow();
    void CloseVideoWindow();

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()

private:
    void UpdateMMedInfo();
    void UpdateInfoText();

    MMBoardFile *m_opened_file;

    wxSlider *m_positionSlider;
    wxBitmapButton *m_playButton, *m_pauseButton, *m_stopButton, *m_ejectButton;
    wxStaticText *m_fileType, *m_infoText;
    wxWindow *m_video_window;

    wxPanel *m_panel;
    wxSizer *m_sizer;

    wxTimer *m_refreshTimer;
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    MMBoard_Quit = 1,
    MMBoard_Open,
    MMBoard_About,
    MMBoard_PositionSlider,
    MMBoard_PlayButton,
    MMBoard_PauseButton,
    MMBoard_ResumeButton,
    MMBoard_StopButton,
    MMBoard_EjectButton,
    MMBoard_RefreshInfo
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MMBoardFrame, wxFrame)
  EVT_MENU(MMBoard_Quit,  MMBoardFrame::OnQuit)
  EVT_MENU(MMBoard_About, MMBoardFrame::OnAbout)
  EVT_MENU(MMBoard_Open, MMBoardFrame::OnOpen)
  EVT_BUTTON(MMBoard_PlayButton, MMBoardFrame::OnPlay)
  EVT_BUTTON(MMBoard_StopButton, MMBoardFrame::OnStop)
  EVT_BUTTON(MMBoard_PauseButton, MMBoardFrame::OnPause)
  EVT_BUTTON(MMBoard_EjectButton, MMBoardFrame::OnEject)
  EVT_SLIDER(MMBoard_PositionSlider, MMBoardFrame::OnSetPosition)
  EVT_CUSTOM(wxEVT_TIMER, MMBoard_RefreshInfo, MMBoardFrame::OnRefreshInfo)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// Main board application launcher
// ---------------------------------------------------------------------------

IMPLEMENT_APP(MMBoardApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

bool MMBoardApp::OnInit()
{
    // create the main application window
    MMBoardFrame *frame = new MMBoardFrame(_T("Multimedia Board"),
                                 wxPoint(50, 50), wxSize(450, 340));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show();

    m_caps = TestMultimediaCaps();

    if (!m_caps) {
      wxMessageBox(_T("Your system has no multimedia capabilities. We are exiting now."), _T("Major error !"), wxOK | wxICON_ERROR, NULL);
      return false;
    }

    wxString msg;
    msg.Printf(_T("Detected : %s%s%s"), (m_caps & MM_SOUND_OSS) ? _T("OSS ") : _T(""),
                                (m_caps & MM_SOUND_ESD) ? _T("ESD ") : _T(""),
                                (m_caps & MM_SOUND_WIN) ? _T("WIN") : _T(""));

    wxMessageBox(msg, _T("Good !"), wxOK | wxICON_INFORMATION, NULL);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

wxUint8 MMBoardApp::TestMultimediaCaps()
{
    wxSoundStream *dev;
    wxUint8 caps;

    caps = 0;

#ifdef __WIN32__
    // We test the Windows sound support.

    dev = new wxSoundStreamWin();
    if (dev->GetError() == wxSOUND_NOERROR)
        caps |= MM_SOUND_WIN;
    delete dev;

#elif defined __UNIX__
    // We now test the ESD support

    dev = new wxSoundStreamESD();
    if (dev->GetError() == wxSOUND_NOERROR)
        caps |= MM_SOUND_ESD;
    delete dev;

    // We test the OSS (Open Sound System) support.
    // WARNING: There is a conflict between ESD and ALSA. We may be interrested
    // in disabling the auto detection of OSS is ESD has been detected.
#if 1
    if (!(caps & MM_SOUND_ESD)) {
#endif

    dev = new wxSoundStreamOSS();
    if (dev->GetError() == wxSOUND_NOERROR)
        caps |= MM_SOUND_OSS;
    delete dev;
#if 1
    }
#endif

#endif

    return caps;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MMBoardFrame::MMBoardFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size)
{
#ifdef __WXMAC__
    // we need this in order to allow the about menu relocation, since ABOUT is
    // not the default id of the about menu
    wxApp::s_macAboutMenuItemId = MMBoard_About;
#endif

    // set the frame icon
    SetIcon(wxICON(mondrian));

    // create a menu bar
    wxMenu *menuFile = new wxMenu(wxEmptyString, wxMENU_TEAROFF);

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(MMBoard_About, wxT("&About...\tCtrl-A"), wxT("Show about dialog"));

    menuFile->Append(MMBoard_Open, wxT("&Open\tAlt-O"), wxT("Open file"));
    menuFile->AppendSeparator();
    menuFile->Append(MMBoard_Quit, wxT("E&xit\tAlt-X"), wxT("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(menuFile, wxT("&File"));
    menuBar->Append(helpMenu, wxT("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(3);
    SetStatusText(wxT("Welcome to wxWidgets!"));
#endif // wxUSE_STATUSBAR

    // Misc variables
    m_opened_file = NULL;

    m_panel = new wxPanel(this, wxID_ANY);

    // Initialize main slider
    m_positionSlider = new wxSlider( m_panel, MMBoard_PositionSlider, 0, 0, 60,
                     wxDefaultPosition, wxSize(300, wxDefaultCoord),
                     wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    m_positionSlider->SetPageSize(60);  // 60 secs
    m_positionSlider->Disable();

    // Initialize info panel
    wxPanel *infoPanel = new wxPanel( m_panel, wxID_ANY);
    infoPanel->SetBackgroundColour(*wxBLACK);
    infoPanel->SetForegroundColour(*wxWHITE);

    wxBoxSizer *infoSizer = new wxBoxSizer(wxVERTICAL);

    m_fileType = new wxStaticText(infoPanel, wxID_ANY, wxEmptyString);
#if wxUSE_STATLINE
    wxStaticLine *line = new wxStaticLine(infoPanel, wxID_ANY);
#endif // wxUSE_STATLINE
    m_infoText = new wxStaticText(infoPanel, wxID_ANY, wxEmptyString);

    UpdateInfoText();

    infoSizer->Add(m_fileType, 0, wxGROW | wxALL, 1);
#if wxUSE_STATLINE
    infoSizer->Add(line, 0, wxGROW | wxCENTRE, 20);
#endif // wxUSE_STATLINE
    infoSizer->Add(m_infoText, 0, wxGROW | wxALL, 1);

    infoPanel->SetSizer(infoSizer);

    // Bitmap button panel
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap play_bmp(play_back_xpm);
    wxBitmap stop_bmp(stop_back_xpm);
    wxBitmap eject_bmp(eject_xpm);
    wxBitmap pause_bmp(pause_xpm);

    m_playButton = new wxBitmapButton(m_panel, MMBoard_PlayButton, play_bmp);
    m_playButton->Disable();
    m_pauseButton = new wxBitmapButton(m_panel, MMBoard_PauseButton, pause_bmp);
    m_pauseButton->Disable();
    m_stopButton = new wxBitmapButton(m_panel, MMBoard_StopButton, stop_bmp);
    m_stopButton->Disable();
    m_ejectButton = new wxBitmapButton(m_panel, MMBoard_EjectButton, eject_bmp);
    m_ejectButton->Disable();

    buttonSizer->Add(m_playButton, 0, wxALL, 2);
    buttonSizer->Add(m_pauseButton, 0, wxALL, 2);
    buttonSizer->Add(m_stopButton, 0, wxALL, 2);
    buttonSizer->Add(m_ejectButton, 0, wxALL, 2);

    // Top sizer
    m_sizer = new wxBoxSizer(wxVERTICAL);
#if wxUSE_STATLINE
    m_sizer->Add(new wxStaticLine(m_panel, wxID_ANY), 0, wxGROW | wxCENTRE, 0);
#endif // wxUSE_STATLINE
    m_sizer->Add(m_positionSlider, 0, wxCENTRE | wxGROW | wxALL, 2);
#if wxUSE_STATLINE
    m_sizer->Add(new wxStaticLine(m_panel, wxID_ANY), 0, wxGROW | wxCENTRE, 0);
#endif // wxUSE_STATLINE
    m_sizer->Add(buttonSizer, 0, wxALL, 0);
#if wxUSE_STATLINE
    m_sizer->Add(new wxStaticLine(m_panel, wxID_ANY), 0, wxGROW | wxCENTRE, 0);
#endif // wxUSE_STATLINE
    m_sizer->Add(infoPanel, 1, wxCENTRE | wxGROW, 0);

    m_panel->SetSizer(m_sizer);
    m_sizer->Fit(this);
    m_sizer->SetSizeHints(this);

    // Timer
    m_refreshTimer = new wxTimer(this, MMBoard_RefreshInfo);

    // Video window
    m_video_window = NULL;

    // Multimedia file
    m_opened_file = NULL;
}

MMBoardFrame::~MMBoardFrame()
{
    if (m_opened_file)
        delete m_opened_file;

    delete m_refreshTimer;
}

void MMBoardFrame::OpenVideoWindow()
{
  if (m_video_window)
    return;

  m_video_window = new wxWindow(m_panel, wxID_ANY, wxDefaultPosition, wxSize(200, 200));
  m_video_window->SetBackgroundColour(*wxBLACK);
  m_sizer->Prepend(m_video_window, 2, wxGROW | wxSHRINK | wxCENTRE, 1);

  m_sizer->Fit(this);
}

void MMBoardFrame::CloseVideoWindow()
{
    if (!m_video_window)
        return;

    m_sizer->Detach( m_video_window );
    delete m_video_window;
    m_video_window = NULL;

    m_sizer->Fit(this);
}

// event handlers

void MMBoardFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MMBoardFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf( wxT("wxWidgets Multimedia board v1.0a, wxMMedia v2.0a:\n")
                wxT("an example of the capabilities of the wxWidgets multimedia classes.\n")
        wxT("Copyright 1999, 2000, Guilhem Lavaux.\n"));

    wxMessageBox(msg, _T("About MMBoard"), wxOK | wxICON_INFORMATION, this);
}

void MMBoardFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxString selected_file;

    if (m_opened_file) {
        if (!m_opened_file->IsStopped()) {
            wxCommandEvent event2;
            OnStop(event2);
        }
        delete m_opened_file;
    }

    // select a file to be opened
#if wxUSE_FILEDLG
    selected_file = wxLoadFileSelector(_T("multimedia"), _T("*"), NULL, this);
#endif // wxUSE_FILEDLG
    if (selected_file.empty())
        return;

    m_opened_file = MMBoardManager::Open(selected_file);

    // Change the range values of the slider.
    MMBoardTime length;

    length = m_opened_file->GetLength();
    m_positionSlider->SetRange(0, length.hours * 3600 + length.minutes * 60 + length.seconds);

    // Update misc info
    UpdateMMedInfo();

#if wxUSE_STATUSBAR
    SetStatusText(selected_file, 2);
#endif // wxUSE_STATUSBAR

    // Update info text
    UpdateInfoText();

    // Enable a few buttons
    m_playButton->Enable();
    m_ejectButton->Enable();
    m_positionSlider->Enable();

    if (m_opened_file->NeedWindow()) {
        OpenVideoWindow();
        m_opened_file->SetWindow(m_video_window);
    } else
        CloseVideoWindow();
}

void MMBoardFrame::UpdateInfoText()
{
    wxString infotext1, infotext2;

    if (m_opened_file) {
        infotext1 = wxT("File type:\n\t");
        infotext1 += m_opened_file->GetStringType() + wxT("\n");

        infotext2 = wxT("File informations:\n\n");
        infotext2 += m_opened_file->GetStringInformation();
    } else {
        infotext1 = wxT("File type: \n\tNo file opened");
        infotext2 = wxT("File informations:\nNo information\n\n\n\n\n");
    }

    m_fileType->SetLabel(infotext1);
    m_infoText->SetLabel(infotext2);
}

void MMBoardFrame::UpdateMMedInfo()
{
    MMBoardTime current, length;

    if (m_opened_file) {
        current = m_opened_file->GetPosition();
        length  = m_opened_file->GetLength();
    } else {
        current.hours = current.minutes = current.seconds = 0;
        length = current;
    }

#if wxUSE_STATUSBAR
    // We refresh the status bar
    wxString temp_string;
    temp_string.Printf(wxT("%02d:%02d / %02d:%02d"), current.hours * 60 + current.minutes,
                       current.seconds, length.hours * 60 + length.minutes, length.seconds);
    SetStatusText(temp_string, 1);
#else
    wxUnusedVar(length);
#endif // wxUSE_STATUSBAR

    // We set the slider position
    m_positionSlider->SetValue(current.hours * 3600 + current.minutes * 60 + current.seconds);
}

// ----------------------------------------------------------------------------
// Playing management, refreshers, ...

void MMBoardFrame::OnRefreshInfo(wxEvent& WXUNUSED(event))
{
    UpdateMMedInfo();

    if (m_opened_file->IsStopped())
    {
        m_refreshTimer->Stop();
        m_playButton->Enable();
        m_stopButton->Disable();
        m_pauseButton->Disable();
    }
}

void MMBoardFrame::OnPlay(wxCommandEvent& WXUNUSED(event))
{
    m_stopButton->Enable();
    m_pauseButton->Enable();
    m_playButton->Disable();

    if (m_opened_file->IsPaused())
    {
        m_opened_file->Resume();
        return;
    }

    m_refreshTimer->Start(1000, false);

    m_opened_file->Play();

    m_stopButton->Enable();
    m_pauseButton->Enable();
    m_playButton->Disable();
}

void MMBoardFrame::OnStop(wxCommandEvent& WXUNUSED(event))
{
    m_opened_file->Stop();
    m_refreshTimer->Stop();

    m_stopButton->Disable();
    m_playButton->Enable();

    UpdateMMedInfo();
}

void MMBoardFrame::OnPause(wxCommandEvent& WXUNUSED(event))
{
    m_opened_file->Pause();

    m_playButton->Enable();
    m_pauseButton->Disable();
}

void MMBoardFrame::OnEject(wxCommandEvent& WXUNUSED(event))
{
    m_opened_file->Stop();

    delete m_opened_file;
    m_opened_file = NULL;

    m_playButton->Disable();
    m_pauseButton->Disable();
    m_stopButton->Disable();
    m_ejectButton->Disable();
    m_positionSlider->Disable();

    UpdateInfoText();
    UpdateMMedInfo();
}

void MMBoardFrame::OnSetPosition(wxCommandEvent& WXUNUSED(event))
{
    wxUint32 itime;
    MMBoardTime btime;

    itime = m_positionSlider->GetValue();
    btime.seconds = itime % 60;
    btime.minutes = (itime / 60) % 60;
    btime.hours = itime / 3600;
    m_opened_file->SetPosition(btime);

    UpdateMMedInfo();
}
