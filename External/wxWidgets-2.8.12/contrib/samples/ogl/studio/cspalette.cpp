/////////////////////////////////////////////////////////////////////////////
// Name:        contrib/samples/ogl/studio/cspalette.cpp
// Purpose:     OGLEdit palette
// Author:      Julian Smart
// Modified by:
// Created:     12/07/98
// RCS-ID:      $Id: cspalette.cpp 37440 2006-02-10 11:59:52Z ABX $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/laywin.h"

#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "doc.h"
#include "view.h"
#include "studio.h"
#include "cspalette.h"
#include "symbols.h"

#ifndef __WXMSW__
#include "bitmaps/arrow.xpm"
#include "bitmaps/texttool.xpm"
#endif

/*
 * Object editor tool palette
 *
 */

csEditorToolPalette::csEditorToolPalette(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
            long style):
  TOOLPALETTECLASS(parent, id, pos, size, style)
{
  m_currentlySelected = -1;

  SetMaxRowsCols(1, 1000);
}

bool csEditorToolPalette::OnLeftClick(int toolIndex, bool toggled)
{
  // BEGIN mutual exclusivity code
  if (toggled && (m_currentlySelected != -1) && (toolIndex != m_currentlySelected))
    ToggleTool(m_currentlySelected, false);

  if (toggled)
    m_currentlySelected = toolIndex;
  else if (m_currentlySelected == toolIndex)
    m_currentlySelected = -1;
  //  END mutual exclusivity code

  return true;
}

void csEditorToolPalette::OnMouseEnter(int toolIndex)
{
#if wxUSE_STATUSBAR
    wxString msg = wxEmptyString;
    if (toolIndex == PALETTE_ARROW)
        msg = _T("Pointer");
    else if (toolIndex != -1)
    {
        csSymbol* symbol = wxGetApp().GetSymbolDatabase()->FindSymbol(toolIndex);
        if (symbol)
            msg = symbol->GetName();
    }
    ((wxFrame*) wxGetApp().GetTopWindow())->SetStatusText(msg);
#else
    wxUnusedVar(toolIndex);
#endif // wxUSE_STATUSBAR
}

void csEditorToolPalette::SetSize(int x, int y, int width, int height, int sizeFlags)
{
  TOOLPALETTECLASS::SetSize(x, y, width, height, sizeFlags);
}

void csEditorToolPalette::SetSelection(int sel)
{
    if ((sel != m_currentlySelected) && (m_currentlySelected != -1))
    {
        ToggleTool(m_currentlySelected, false);
    }
    m_currentlySelected = sel;
    ToggleTool(m_currentlySelected, true);
}

bool csApp::CreatePalette(wxFrame *parent)
{
    // First create a layout window
    wxSashLayoutWindow* win = new wxSashLayoutWindow(parent, ID_LAYOUT_WINDOW_PALETTE, wxDefaultPosition, wxSize(200, 30), wxNO_BORDER|wxSW_3D|wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(10000, 40));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    win->SetSashVisible(wxSASH_BOTTOM, true);

    m_diagramPaletteSashWindow = win;

    m_diagramPaletteSashWindow->Show(false);

  // Load palette bitmaps
#ifdef __WXMSW__
    wxBitmap PaletteArrow(_T("arrowtool"));
    wxBitmap TextTool(_T("texttool"));
    wxSize toolBitmapSize(32, 32);
#else // !__WXMSW__
    wxBitmap PaletteArrow(arrow_xpm);
    wxBitmap TextTool(texttool_xpm);
    wxSize toolBitmapSize(22, 22);
#endif

  csEditorToolPalette *palette = new csEditorToolPalette(m_diagramPaletteSashWindow, ID_DIAGRAM_PALETTE, wxPoint(0, 0), wxDefaultSize, wxTB_HORIZONTAL|wxNO_BORDER);

  palette->SetMargins(2, 2);

  palette->SetToolBitmapSize(toolBitmapSize);

  palette->AddTool(PALETTE_ARROW, PaletteArrow, wxNullBitmap, true, 0, wxDefaultCoord, NULL, _T("Pointer"));
  palette->AddTool(PALETTE_TEXT_TOOL, TextTool, wxNullBitmap, true, 0, wxDefaultCoord, NULL, _T("Text"));

  wxChar** symbols = new wxChar*[20];
  int noSymbols = 0;

  symbols[noSymbols] = _T("Wide Rectangle");
  noSymbols ++;

  symbols[noSymbols] =  _T("Thin Rectangle");
  noSymbols ++;

  symbols[noSymbols] =  _T("Triangle");
  noSymbols ++;

  symbols[noSymbols] =  _T("Octagon");
  noSymbols ++;

  // For some reason, we're getting Gdk errors with
  // some shapes, such as ones that use DrawEllipse.
#ifndef __WXGTK__
  symbols[noSymbols] =  _T("Group");
  noSymbols ++;

  symbols[noSymbols] =  _T("Circle");
  noSymbols ++;

  symbols[noSymbols] =  _T("Circle shadow");
  noSymbols ++;

  symbols[noSymbols] =  _T("SemiCircle");
  noSymbols ++;
#endif

  int i;
  for (i = 0; i < noSymbols; i++)
  {
      csSymbol* symbol = GetSymbolDatabase()->FindSymbol(symbols[i]);
      if (symbol)
      {
           wxBitmap* bitmap = GetSymbolDatabase()->CreateToolBitmap(symbol, toolBitmapSize);
           palette->AddTool(symbol->GetToolId(), *bitmap, wxNullBitmap, true, 0, wxDefaultCoord, NULL, symbol->GetName());

           delete bitmap;
      }
  }
  delete[] symbols;

#if 0
  wxNode* node = GetSymbolDatabase()->GetSymbols().First();
  while (node)
  {
    csSymbol* symbol = (csSymbol*) node->Data();

    wxBitmap* bitmap = GetSymbolDatabase()->CreateToolBitmap(symbol, toolBitmapSize);
    palette->AddTool(symbol->GetToolId(), *bitmap, wxNullBitmap, true, 0, wxDefaultCoord, NULL, symbol->GetName());

    delete bitmap;

    node = node->Next();
  }
#endif

  palette->Realize();

  palette->SetSelection(PALETTE_ARROW);
  m_diagramPalette = palette;

  return true;
}
