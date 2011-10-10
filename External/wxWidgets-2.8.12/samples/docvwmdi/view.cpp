/////////////////////////////////////////////////////////////////////////////
// Name:        view.cpp
// Purpose:     View classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: view.cpp 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "docview.h"
#include "doc.h"
#include "view.h"

IMPLEMENT_DYNAMIC_CLASS(DrawingView, wxView)

// For drawing lines in a canvas
float xpos = -1;
float ypos = -1;

BEGIN_EVENT_TABLE(DrawingView, wxView)
    EVT_MENU(DOODLE_CUT, DrawingView::OnCut)
END_EVENT_TABLE()

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool DrawingView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
    frame = wxGetApp().CreateChildFrame(doc, this, true);
    frame->SetTitle(_T("DrawingView"));

    canvas = GetMainFrame()->CreateCanvas(this, frame);
#ifdef __X__
    // X seems to require a forced resize
    int x, y;
    frame->GetSize(&x, &y);
    frame->SetSize(wxDefaultCoord, wxDefaultCoord, x, y);
#endif
    frame->Show(true);
    Activate(true);

    return true;
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void DrawingView::OnDraw(wxDC *dc)
{
  dc->SetFont(*wxNORMAL_FONT);
  dc->SetPen(*wxBLACK_PEN);

  wxList::compatibility_iterator node = ((DrawingDocument *)GetDocument())->GetDoodleSegments().GetFirst();
  while (node)
  {
    DoodleSegment *seg = (DoodleSegment *)node->GetData();
    seg->Draw(dc);
    node = node->GetNext();
  }
}

void DrawingView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
  if (canvas)
    canvas->Refresh();

/* Is the following necessary?
#ifdef __WXMSW__
  if (canvas)
    canvas->Refresh();
#else
  if (canvas)
    {
      wxClientDC dc(canvas);
      dc.Clear();
      OnDraw(& dc);
    }
#endif
*/
}

// Clean up windows used for displaying the view.
bool DrawingView::OnClose(bool deleteWindow)
{
  if (!GetDocument()->Close())
    return false;

  // Clear the canvas in  case we're in single-window mode,
  // and the canvas stays.
  canvas->ClearBackground();
  canvas->view = (wxView *) NULL;
  canvas = (MyCanvas *) NULL;

  wxString s(wxTheApp->GetAppName());
  if (frame)
    frame->SetTitle(s);

  SetFrame((wxFrame*)NULL);

  Activate(false);

  if (deleteWindow)
  {
    delete frame;
    return true;
  }
  return true;
}

void DrawingView::OnCut(wxCommandEvent& WXUNUSED(event) )
{
    DrawingDocument *doc = (DrawingDocument *)GetDocument();
    doc->GetCommandProcessor()->Submit(new DrawingCommand(_T("Cut Last Segment"), DOODLE_CUT, doc, (DoodleSegment *) NULL));
}

IMPLEMENT_DYNAMIC_CLASS(TextEditView, wxView)

bool TextEditView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
  frame = wxGetApp().CreateChildFrame(doc, this, false);

  int width, height;
  frame->GetClientSize(&width, &height);
  textsw = new MyTextWindow(this, frame, wxPoint(0, 0), wxSize(width, height), wxTE_MULTILINE);
  frame->SetTitle(_T("TextEditView"));

#ifdef __X__
  // X seems to require a forced resize
  int x, y;
  frame->GetSize(&x, &y);
  frame->SetSize(wxDefaultCoord, wxDefaultCoord, x, y);
#endif

  frame->Show(true);
  Activate(true);

  return true;
}

// Handled by wxTextWindow
void TextEditView::OnDraw(wxDC *WXUNUSED(dc) )
{
}

void TextEditView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) )
{
}

bool TextEditView::OnClose(bool deleteWindow)
{
  if (!GetDocument()->Close())
    return false;

  Activate(false);

  if (deleteWindow)
  {
    delete frame;
    return true;
  }
  return true;
}

/*
 * Window implementations
 */

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
    EVT_MOUSE_EVENTS(MyCanvas::OnMouseEvent)
END_EVENT_TABLE()

// Define a constructor for my canvas
MyCanvas::MyCanvas(wxView *v, wxMDIChildFrame *frame, const wxPoint& pos, const wxSize& size, long style):
 wxScrolledWindow(frame, wxID_ANY, pos, size, style)
{
  view = v;
}

// Define the repainting behaviour
void MyCanvas::OnDraw(wxDC& dc)
{
  if (view)
    view->OnDraw(& dc);
}

// This implements a tiny doodling program. Drag the mouse using
// the left button.
void MyCanvas::OnMouseEvent(wxMouseEvent& event)
{
  if (!view)
    return;

  static DoodleSegment *currentSegment = (DoodleSegment *) NULL;

  wxClientDC dc(this);
  PrepareDC(dc);

  dc.SetPen(*wxBLACK_PEN);

  wxPoint pt(event.GetLogicalPosition(dc));

  if (currentSegment && event.LeftUp())
  {
    if (currentSegment->lines.GetCount() == 0)
    {
      delete currentSegment;
      currentSegment = (DoodleSegment *) NULL;
    }
    else
    {
      // We've got a valid segment on mouse left up, so store it.
      DrawingDocument *doc = (DrawingDocument *)view->GetDocument();

      doc->GetCommandProcessor()->Submit(new DrawingCommand(_T("Add Segment"), DOODLE_ADD, doc, currentSegment));

      view->GetDocument()->Modify(true);
      currentSegment = (DoodleSegment *) NULL;
    }
  }

  if (xpos > -1 && ypos > -1 && event.Dragging())
  {
    if (!currentSegment)
      currentSegment = new DoodleSegment;

    DoodleLine *newLine = new DoodleLine;
    newLine->x1 = (long)xpos;
    newLine->y1 = (long)ypos;
    newLine->x2 = pt.x;
    newLine->y2 = pt.y;
    currentSegment->lines.Append(newLine);

    dc.DrawLine( (long)xpos, (long)ypos, pt.x, pt.y);
  }
  xpos = pt.x;
  ypos = pt.y;
}

// Define a constructor for my text subwindow
MyTextWindow::MyTextWindow(wxView *v, wxMDIChildFrame *frame, const wxPoint& pos, const wxSize& size, long style):
 wxTextCtrl(frame, wxID_ANY, _T(""), pos, size, style)
{
  view = v;
}


