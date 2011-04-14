/////////////////////////////////////////////////////////////////////////////
// Name:        contrib/samples/ogl/studio/doc.cpp
// Purpose:     Implements document functionality
// Author:      Julian Smart
// Modified by:
// Created:     12/07/98
// RCS-ID:      $Id: doc.cpp 37440 2006-02-10 11:59:52Z ABX $
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

#include "studio.h"
#include "doc.h"
#include "view.h"
#include "wx/ogl/basicp.h"

IMPLEMENT_DYNAMIC_CLASS(csDiagramDocument, wxDocument)

#ifdef __VISUALC__
#pragma warning(disable:4355)
#endif

csDiagramDocument::csDiagramDocument():m_diagram(this)
{
}

#ifdef __VISUALC__
#pragma warning(default:4355)
#endif

csDiagramDocument::~csDiagramDocument()
{
}

bool csDiagramDocument::OnCloseDocument()
{
  m_diagram.DeleteAllShapes();
  return true;
}

#if wxUSE_PROLOGIO
bool csDiagramDocument::OnSaveDocument(const wxString& file)
{
  if (file == wxEmptyString)
    return false;

  if (!m_diagram.SaveFile(file))
  {
    wxString msgTitle;
    if (wxTheApp->GetAppName() != wxEmptyString)
        msgTitle = wxTheApp->GetAppName();
    else
        msgTitle = wxString(_T("File error"));

    (void)wxMessageBox(_T("Sorry, could not open this file for saving."), msgTitle, wxOK | wxICON_EXCLAMATION,
      GetDocumentWindow());
    return false;
  }

  Modify(false);
  SetFilename(file);
  return true;
}

bool csDiagramDocument::OnOpenDocument(const wxString& file)
{
  if (!OnSaveModified())
    return false;

  wxString msgTitle;
  if (wxTheApp->GetAppName() != wxEmptyString)
    msgTitle = wxTheApp->GetAppName();
  else
    msgTitle = wxString(_T("File error"));

  m_diagram.DeleteAllShapes();
  if (!m_diagram.LoadFile(file))
  {
    (void)wxMessageBox(_T("Sorry, could not open this file."), msgTitle, wxOK|wxICON_EXCLAMATION,
     GetDocumentWindow());
    return false;
  }
  SetFilename(file, true);
  Modify(false);
  UpdateAllViews();

  return true;
}
#endif // wxUSE_PROLOGIO


/*
 * Implementation of drawing command
 */

csDiagramCommand::csDiagramCommand(const wxString& name, csDiagramDocument *doc,
    csCommandState* onlyState):
  wxCommand(true, name)
{
  m_doc = doc;

  if (onlyState)
  {
    AddState(onlyState);
  }
}

csDiagramCommand::~csDiagramCommand()
{
    wxObjectList::compatibility_iterator node = m_states.GetFirst();
    while (node)
    {
        csCommandState* state = (csCommandState*) node->GetData();
        delete state;
        node = node->GetNext();
    }
}

void csDiagramCommand::AddState(csCommandState* state)
{
    state->m_doc = m_doc;
//    state->m_cmd = m_cmd;
    m_states.Append(state);
}

// Insert a state at the beginning of the list
void csDiagramCommand::InsertState(csCommandState* state)
{
    state->m_doc = m_doc;
//    state->m_cmd = m_cmd;
    m_states.Insert(state);
}

// Schedule all lines connected to the states to be cut.
void csDiagramCommand::RemoveLines()
{
    wxObjectList::compatibility_iterator node = m_states.GetFirst();
    while (node)
    {
        csCommandState* state = (csCommandState*) node->GetData();
        wxShape* shape = state->GetShapeOnCanvas();
        wxASSERT( (shape != NULL) );

        wxObjectList::compatibility_iterator node1 = shape->GetLines().GetFirst();
        while (node1)
        {
            wxLineShape *line = (wxLineShape *)node1->GetData();
            if (!FindStateByShape(line))
            {
                csCommandState* newState = new csCommandState(ID_CS_CUT, NULL, line);
                InsertState(newState);
            }

            node1 = node1->GetNext();
        }
        node = node->GetNext();
    }
}

csCommandState* csDiagramCommand::FindStateByShape(wxShape* shape)
{
    wxObjectList::compatibility_iterator node = m_states.GetFirst();
    while (node)
    {
        csCommandState* state = (csCommandState*) node->GetData();
        if (shape == state->GetShapeOnCanvas() || shape == state->GetSavedState())
            return state;
        node = node->GetNext();
    }
    return NULL;
}

bool csDiagramCommand::Do()
{
    wxObjectList::compatibility_iterator node = m_states.GetFirst();
    while (node)
    {
        csCommandState* state = (csCommandState*) node->GetData();
        if (!state->Do())
            return false;
        node = node->GetNext();
    }
    return true;
}

bool csDiagramCommand::Undo()
{
    // Undo in reverse order, so e.g. shapes get added
    // back before the lines do.
    wxObjectList::compatibility_iterator node = m_states.GetLast();
    while (node)
    {
        csCommandState* state = (csCommandState*) node->GetData();
        if (!state->Undo())
            return false;
        node = node->GetPrevious();
    }
    return true;
}

csCommandState::csCommandState(int cmd, wxShape* savedState, wxShape* shapeOnCanvas)
{
    m_cmd = cmd;
    m_doc = NULL;
    m_savedState = savedState;
    m_shapeOnCanvas = shapeOnCanvas;
    m_linePositionFrom = 0;
    m_linePositionTo = 0;
}

csCommandState::~csCommandState()
{
    if (m_savedState)
    {
        m_savedState->SetCanvas(NULL);
        delete m_savedState;
    }
}

bool csCommandState::Do()
{
  switch (m_cmd)
  {
    case ID_CS_CUT:
    {
        // New state is 'nothing' - maybe pass shape ID to state so we know what
        // we're talking about.
        // Then save old shape in m_savedState (actually swap pointers)

        wxASSERT( (m_shapeOnCanvas != NULL) );
        wxASSERT( (m_savedState == NULL) ); // new state will be 'nothing'
        wxASSERT( (m_doc != NULL) );

        wxShapeCanvas* canvas = m_shapeOnCanvas->GetCanvas();

        // In case this is a line
        wxShape* lineFrom = NULL;
        wxShape* lineTo = NULL;
        int attachmentFrom = 0, attachmentTo = 0;

        if (m_shapeOnCanvas->IsKindOf(CLASSINFO(wxLineShape)))
        {
            // Store the from/to info to save in the line shape
            wxLineShape* lineShape = (wxLineShape*) m_shapeOnCanvas;
            lineFrom = lineShape->GetFrom();
            lineTo = lineShape->GetTo();
            attachmentFrom = lineShape->GetAttachmentFrom();
            attachmentTo = lineShape->GetAttachmentTo();

            m_linePositionFrom = lineFrom->GetLinePosition(lineShape);
            m_linePositionTo = lineTo->GetLinePosition(lineShape);
        }

        m_shapeOnCanvas->Select(false);
        ((csDiagramView*) m_doc->GetFirstView())->SelectShape(m_shapeOnCanvas, false);

        m_shapeOnCanvas->Unlink();

        m_doc->GetDiagram()->RemoveShape(m_shapeOnCanvas);

        m_savedState = m_shapeOnCanvas;

        if (m_savedState->IsKindOf(CLASSINFO(wxLineShape)))
        {
            // Restore the from/to info for future reference
            wxLineShape* lineShape = (wxLineShape*) m_savedState;
            lineShape->SetFrom(lineFrom);
            lineShape->SetTo(lineTo);
            lineShape->SetAttachments(attachmentFrom, attachmentTo);

            wxClientDC dc(canvas);
            canvas->PrepareDC(dc);

            lineFrom->MoveLinks(dc);
            lineTo->MoveLinks(dc);
        }

        m_doc->Modify(true);
        m_doc->UpdateAllViews();
        break;
    }
    case ID_CS_ADD_SHAPE:
    case ID_CS_ADD_SHAPE_SELECT:
    {
        // The app has given the command state a new m_savedState
        // shape, which is the new shape to add to the canvas (but
        // not actually added until this point).
        // The new 'saved state' is therefore 'nothing' since there
        // was nothing there before.

        wxASSERT( (m_shapeOnCanvas == NULL) );
        wxASSERT( (m_savedState != NULL) );
        wxASSERT( (m_doc != NULL) );

        m_shapeOnCanvas = m_savedState;
        m_savedState = NULL;

        m_doc->GetDiagram()->AddShape(m_shapeOnCanvas);
        m_shapeOnCanvas->Show(true);

        wxClientDC dc(m_shapeOnCanvas->GetCanvas());
        m_shapeOnCanvas->GetCanvas()->PrepareDC(dc);

        csEvtHandler *handler = (csEvtHandler *)m_shapeOnCanvas->GetEventHandler();
        m_shapeOnCanvas->FormatText(dc, handler->m_label);

        m_shapeOnCanvas->Move(dc, m_shapeOnCanvas->GetX(), m_shapeOnCanvas->GetY());

        if (m_cmd == ID_CS_ADD_SHAPE_SELECT)
        {
            m_shapeOnCanvas->Select(true, &dc);
            ((csDiagramView*) m_doc->GetFirstView())->SelectShape(m_shapeOnCanvas, true);
        }

        m_doc->Modify(true);
        m_doc->UpdateAllViews();
        break;
    }
    case ID_CS_ADD_LINE:
    case ID_CS_ADD_LINE_SELECT:
    {
        wxASSERT( (m_shapeOnCanvas == NULL) );
        wxASSERT( (m_savedState != NULL) );
        wxASSERT( (m_doc != NULL) );

        wxLineShape *lineShape = (wxLineShape *)m_savedState;
        wxASSERT( (lineShape->GetFrom() != NULL) );
        wxASSERT( (lineShape->GetTo() != NULL) );

        m_shapeOnCanvas = m_savedState;
        m_savedState = NULL;

        m_doc->GetDiagram()->AddShape(lineShape);

        lineShape->GetFrom()->AddLine(lineShape, lineShape->GetTo(),
            lineShape->GetAttachmentFrom(), lineShape->GetAttachmentTo());

        lineShape->Show(true);

        wxClientDC dc(lineShape->GetCanvas());
        lineShape->GetCanvas()->PrepareDC(dc);

        // It won't get drawn properly unless you move both
        // connected images
        lineShape->GetFrom()->Move(dc, lineShape->GetFrom()->GetX(), lineShape->GetFrom()->GetY());
        lineShape->GetTo()->Move(dc, lineShape->GetTo()->GetX(), lineShape->GetTo()->GetY());

        if (m_cmd == ID_CS_ADD_LINE_SELECT)
        {
            lineShape->Select(true, &dc);
            ((csDiagramView*) m_doc->GetFirstView())->SelectShape(m_shapeOnCanvas, true);
        }

        m_doc->Modify(true);
        m_doc->UpdateAllViews();
        break;
    }
    case ID_CS_CHANGE_BACKGROUND_COLOUR:
    case ID_CS_MOVE:
    case ID_CS_SIZE:
    case ID_CS_EDIT_PROPERTIES:
    case ID_CS_FONT_CHANGE:
    case ID_CS_ARROW_CHANGE:
    case ID_CS_ROTATE_CLOCKWISE:
    case ID_CS_ROTATE_ANTICLOCKWISE:
    case ID_CS_CHANGE_LINE_ORDERING:
    case ID_CS_CHANGE_LINE_ATTACHMENT:
    case ID_CS_ALIGN:
    case ID_CS_NEW_POINT:
    case ID_CS_CUT_POINT:
    case ID_CS_MOVE_LINE_POINT:
    case ID_CS_STRAIGHTEN:
    case ID_CS_MOVE_LABEL:
    {
        // At this point we have been given a new shape
        // just like the old one but with a changed colour.
        // It's now time to apply that change to the
        // shape on the canvas, saving the old state.
        // NOTE: this is general enough to work with MOST attribute
        // changes!

        wxASSERT( (m_shapeOnCanvas != NULL) );
        wxASSERT( (m_savedState != NULL) ); // This is the new shape with changed colour
        wxASSERT( (m_doc != NULL) );

        wxClientDC dc(m_shapeOnCanvas->GetCanvas());
        m_shapeOnCanvas->GetCanvas()->PrepareDC(dc);

        bool isSelected = m_shapeOnCanvas->Selected();
        if (isSelected)
            m_shapeOnCanvas->Select(false, & dc);

        if (m_cmd == ID_CS_SIZE || m_cmd == ID_CS_ROTATE_CLOCKWISE || m_cmd == ID_CS_ROTATE_ANTICLOCKWISE ||
            m_cmd == ID_CS_CHANGE_LINE_ORDERING || m_cmd == ID_CS_CHANGE_LINE_ATTACHMENT)
        {
            m_shapeOnCanvas->Erase(dc);
        }

        // TODO: make sure the ID is the same. Or, when applying the new state,
        // don't change the original ID.
        wxShape* tempShape = m_shapeOnCanvas->CreateNewCopy();

        // Apply the saved state to the shape on the canvas, by copying.
        m_savedState->CopyWithHandler(*m_shapeOnCanvas);

        // Delete this state now it's been used (m_shapeOnCanvas currently holds this state)
        delete m_savedState;

        // Remember the previous state
        m_savedState = tempShape;

        // Redraw the shape

        if (m_cmd == ID_CS_MOVE || m_cmd == ID_CS_ROTATE_CLOCKWISE || m_cmd == ID_CS_ROTATE_ANTICLOCKWISE ||
            m_cmd == ID_CS_ALIGN)
        {
            m_shapeOnCanvas->Move(dc, m_shapeOnCanvas->GetX(), m_shapeOnCanvas->GetY());

            csEvtHandler *handler = (csEvtHandler *)m_shapeOnCanvas->GetEventHandler();
            m_shapeOnCanvas->FormatText(dc, handler->m_label);
            m_shapeOnCanvas->Draw(dc);
        }
        else if (m_cmd == ID_CS_CHANGE_LINE_ORDERING)
        {
            m_shapeOnCanvas->MoveLinks(dc);
        }
        else if (m_cmd == ID_CS_CHANGE_LINE_ATTACHMENT)
        {
            wxLineShape *lineShape = (wxLineShape *)m_shapeOnCanvas;

            // Have to move both sets of links since we don't know which links
            // have been affected (unless we compared before and after states).
            lineShape->GetFrom()->MoveLinks(dc);
            lineShape->GetTo()->MoveLinks(dc);
        }
        else if (m_cmd == ID_CS_SIZE)
        {
            double width, height;
            m_shapeOnCanvas->GetBoundingBoxMax(&width, &height);

            m_shapeOnCanvas->SetSize(width, height);
            m_shapeOnCanvas->Move(dc, m_shapeOnCanvas->GetX(), m_shapeOnCanvas->GetY());

            m_shapeOnCanvas->Show(true);

            // Recursively redraw links if we have a composite.
            if (m_shapeOnCanvas->GetChildren().GetCount() > 0)
                m_shapeOnCanvas->DrawLinks(dc, -1, true);

            m_shapeOnCanvas->GetEventHandler()->OnEndSize(width, height);
        }
        else if (m_cmd == ID_CS_EDIT_PROPERTIES || m_cmd == ID_CS_FONT_CHANGE)
        {
            csEvtHandler *handler = (csEvtHandler *)m_shapeOnCanvas->GetEventHandler();
            m_shapeOnCanvas->FormatText(dc, handler->m_label);
            m_shapeOnCanvas->Draw(dc);
        }
        else
        {
            m_shapeOnCanvas->Draw(dc);
        }

        if (isSelected)
            m_shapeOnCanvas->Select(true, & dc);

        m_doc->Modify(true);
        m_doc->UpdateAllViews();

        break;
    }
  }
  return true;
}

bool csCommandState::Undo()
{
  switch (m_cmd)
  {
    case ID_CS_CUT:
    {
        wxASSERT( (m_savedState != NULL) );
        wxASSERT( (m_doc != NULL) );

        m_doc->GetDiagram()->AddShape(m_savedState);
        m_shapeOnCanvas = m_savedState;
        m_savedState = NULL;

        if (m_shapeOnCanvas->IsKindOf(CLASSINFO(wxLineShape)))
        {
            wxLineShape* lineShape = (wxLineShape*) m_shapeOnCanvas;
            lineShape->GetFrom()->AddLine(lineShape, lineShape->GetTo(),
                lineShape->GetAttachmentFrom(), lineShape->GetAttachmentTo(),
                m_linePositionFrom, m_linePositionTo);

            wxShapeCanvas* canvas = lineShape->GetFrom()->GetCanvas();

            wxClientDC dc(canvas);
            canvas->PrepareDC(dc);

            lineShape->GetFrom()->MoveLinks(dc);
            lineShape->GetTo()->MoveLinks(dc);

        }
        m_shapeOnCanvas->Show(true);

        m_doc->Modify(true);
        m_doc->UpdateAllViews();
        break;
    }
    case ID_CS_ADD_SHAPE:
    case ID_CS_ADD_LINE:
    case ID_CS_ADD_SHAPE_SELECT:
    case ID_CS_ADD_LINE_SELECT:
    {
        wxASSERT( (m_shapeOnCanvas != NULL) );
        wxASSERT( (m_savedState == NULL) );
        wxASSERT( (m_doc != NULL) );

        // In case this is a line
        wxShape* lineFrom = NULL;
        wxShape* lineTo = NULL;
        int attachmentFrom = 0, attachmentTo = 0;

        if (m_shapeOnCanvas->IsKindOf(CLASSINFO(wxLineShape)))
        {
            // Store the from/to info to save in the line shape
            wxLineShape* lineShape = (wxLineShape*) m_shapeOnCanvas;
            lineFrom = lineShape->GetFrom();
            lineTo = lineShape->GetTo();
            attachmentFrom = lineShape->GetAttachmentFrom();
            attachmentTo = lineShape->GetAttachmentTo();
        }

        wxClientDC dc(m_shapeOnCanvas->GetCanvas());
        m_shapeOnCanvas->GetCanvas()->PrepareDC(dc);

        m_shapeOnCanvas->Select(false, &dc);
        ((csDiagramView*) m_doc->GetFirstView())->SelectShape(m_shapeOnCanvas, false);
        m_doc->GetDiagram()->RemoveShape(m_shapeOnCanvas);
        m_shapeOnCanvas->Unlink(); // Unlinks the line, if it is a line

        if (m_shapeOnCanvas->IsKindOf(CLASSINFO(wxLineShape)))
        {
            // Restore the from/to info for future reference
            wxLineShape* lineShape = (wxLineShape*) m_shapeOnCanvas;
            lineShape->SetFrom(lineFrom);
            lineShape->SetTo(lineTo);
            lineShape->SetAttachments(attachmentFrom, attachmentTo);
        }

        m_savedState = m_shapeOnCanvas;
        m_shapeOnCanvas = NULL;

        m_doc->Modify(true);
        m_doc->UpdateAllViews();
        break;
    }
    case ID_CS_CHANGE_BACKGROUND_COLOUR:
    case ID_CS_MOVE:
    case ID_CS_SIZE:
    case ID_CS_EDIT_PROPERTIES:
    case ID_CS_FONT_CHANGE:
    case ID_CS_ARROW_CHANGE:
    case ID_CS_ROTATE_CLOCKWISE:
    case ID_CS_ROTATE_ANTICLOCKWISE:
    case ID_CS_CHANGE_LINE_ORDERING:
    case ID_CS_CHANGE_LINE_ATTACHMENT:
    case ID_CS_ALIGN:
    case ID_CS_NEW_POINT:
    case ID_CS_CUT_POINT:
    case ID_CS_MOVE_LINE_POINT:
    case ID_CS_STRAIGHTEN:
    case ID_CS_MOVE_LABEL:
    {
        // Exactly like the Do case; we're just swapping states.
        Do();
        break;
    }
  }

    return true;
}
