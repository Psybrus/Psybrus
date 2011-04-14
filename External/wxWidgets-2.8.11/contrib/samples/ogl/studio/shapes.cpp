/////////////////////////////////////////////////////////////////////////////
// Name:        contrib/samples/ogl/studio/shapes.cpp
// Purpose:     Implements Studio shapes
// Author:      Julian Smart
// Modified by:
// Created:     12/07/98
// RCS-ID:      $Id: shapes.cpp 37440 2006-02-10 11:59:52Z ABX $
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

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in wx_setup.h!
#endif

#include "wx/ogl/ogl.h" // base header of OGL, includes and adjusts wx/deprecated/setup.h

#include "studio.h"
#include "doc.h"
#include "shapes.h"
#include "view.h"
#include "cspalette.h"
#include "dialogs.h"

#define csSTANDARD_SHAPE_WIDTH      100

IMPLEMENT_CLASS(csDiagram, wxDiagram)

csDiagram::~csDiagram()
{
    DeleteAllShapes();
}

void csDiagram::Redraw(wxDC& dc)
{
    wxDiagram::Redraw(dc);

    // Draw line crossings
    wxLineCrossings lineCrossings;
    lineCrossings.FindCrossings(*this);
    lineCrossings.DrawCrossings(*this, dc);
}

/*
 * csEvtHandler: an event handler class for all shapes
 */

IMPLEMENT_DYNAMIC_CLASS(csEvtHandler, wxShapeEvtHandler)

csEvtHandler::csEvtHandler(wxShapeEvtHandler *prev, wxShape *shape, const wxString& lab):
  wxShapeEvtHandler(prev, shape)
{
    m_label = lab;
}

csEvtHandler::~csEvtHandler()
{
}

// Copy any event handler data
void csEvtHandler::CopyData(wxShapeEvtHandler& copy)
{
    wxShapeEvtHandler::CopyData(copy);

    csEvtHandler& csCopy = (csEvtHandler&) copy;
    csCopy.m_label = m_label;
}

void csEvtHandler::OnLeftClick(double WXUNUSED(x), double WXUNUSED(y), int keys, int WXUNUSED(attachment))
{
  wxClientDC dc(GetShape()->GetCanvas());
  GetShape()->GetCanvas()->PrepareDC(dc);

  csDiagramView* view = ((csCanvas*)GetShape()->GetCanvas())->GetView();
  view->ReflectPointSize(GetShape()->GetFont()->GetPointSize());

  if (GetShape()->IsKindOf(CLASSINFO(wxLineShape)))
      view->ReflectArrowState((wxLineShape*) GetShape());

  csEditorToolPalette *palette = wxGetApp().GetDiagramPalette();
  if (palette->GetSelection() == PALETTE_TEXT_TOOL)
  {
        view->ReflectPointSize(GetShape()->GetFont()->GetPointSize());

        EditProperties();
#if 0
        csLabelEditingDialog* dialog = new csLabelEditingDialog(GetShape()->GetCanvas()->GetParent());
        dialog->SetShapeLabel(m_label);
        if (dialog->ShowModal() == wxID_CANCEL)
        {
            dialog->Destroy();
            return;
        }

        wxString newLabel = dialog->GetShapeLabel();
        dialog->Destroy();

        wxShape* newShape = GetShape()->CreateNewCopy();

        csEvtHandler* handler = (csEvtHandler *)newShape->GetEventHandler();
        handler->m_label = newLabel;

        view->GetDocument()->GetCommandProcessor()->Submit(new csDiagramCommand("Edit label", (csDiagramDocument*) view->GetDocument(),
            new csCommandState(ID_CS_EDIT_PROPERTIES, newShape, GetShape())));
#endif
        return;
  }

  if (keys == 0)
  {
    // If no shift key, then everything is deselected.
    // If the shape was selected, deselect it and vice versa.
    bool selected = GetShape()->Selected();

    view->SelectAll(false);

    selected = !selected;

    GetShape()->Select(selected, &dc);
    GetShape()->GetCanvas()->Redraw(dc); // Redraw because bits of objects will be missing

    view->SelectShape(GetShape(), selected);
  }
  else if (keys & KEY_SHIFT)
  {
    if (GetShape()->Selected())
    {
        GetShape()->Select(false, &dc);
        view->SelectShape(GetShape(), false);
    }
    else
    {
        GetShape()->Select(true, &dc);
        view->SelectShape(GetShape(), true);
    }
    GetShape()->GetCanvas()->Redraw(dc); // Redraw because bits of objects will be missing
  }
  else if (keys & KEY_CTRL)
  {
    // Do something for CONTROL
  }
  else
  {
#if wxUSE_STATUSBAR
    ((wxFrame*)wxGetApp().GetTopWindow())->SetStatusText(m_label);
#endif // wxUSE_STATUSBAR
  }
}

void csEvtHandler::OnRightClick(double x, double y, int WXUNUSED(keys), int WXUNUSED(attachment))
{
    // Have to convert back to physical coordinates from logical coordinates.

    int viewStartX, viewStartY;
    int unitX, unitY;
    GetShape()->GetCanvas()->GetViewStart(& viewStartX, & viewStartY);
    GetShape()->GetCanvas()->GetScrollPixelsPerUnit(& unitX, & unitY);

    int x1 = (int)(x * GetShape()->GetCanvas()->GetScaleX());
    int y1 = (int)(y * GetShape()->GetCanvas()->GetScaleY());

    int menuX = (int) (x1 - (viewStartX * unitX)) ;
    int menuY = (int) (y1 - (viewStartY * unitY));

    wxGetApp().GetShapeEditMenu()->SetClientData((char*) GetShape());
    wxGetApp().GetShapeEditMenu()->Enable(ID_CS_ROTATE_CLOCKWISE, !GetShape()->IsKindOf(CLASSINFO(wxLineShape)));
    wxGetApp().GetShapeEditMenu()->Enable(ID_CS_ROTATE_ANTICLOCKWISE, !GetShape()->IsKindOf(CLASSINFO(wxLineShape)));

    GetShape()->GetCanvas()->PopupMenu(wxGetApp().GetShapeEditMenu(), menuX, menuY);
}

/*
 * Implement connection of two shapes by right-dragging between them.
 */

void csEvtHandler::OnBeginDragRight(double x, double y, int WXUNUSED(keys), int attachment)
{
  wxClientDC dc(GetShape()->GetCanvas());
  GetShape()->GetCanvas()->PrepareDC(dc);

  wxPen dottedPen(*wxBLACK, 1, wxDOT);
  dc.SetLogicalFunction(OGLRBLF);
  dc.SetPen(dottedPen);
  double xp, yp;
  GetShape()->GetAttachmentPositionEdge(attachment, &xp, &yp);
  dc.DrawLine((wxCoord)xp, (wxCoord)yp, (wxCoord)x, (wxCoord)y);
  GetShape()->GetCanvas()->CaptureMouse();
}

void csEvtHandler::OnDragRight(bool WXUNUSED(draw), double x, double y, int WXUNUSED(keys), int attachment)
{
  wxClientDC dc(GetShape()->GetCanvas());
  GetShape()->GetCanvas()->PrepareDC(dc);

  wxPen dottedPen(*wxBLACK, 1, wxDOT);
  dc.SetLogicalFunction(OGLRBLF);
  dc.SetPen(dottedPen);
  double xp, yp;
  GetShape()->GetAttachmentPositionEdge(attachment, &xp, &yp);
  dc.DrawLine((wxCoord)xp, (wxCoord)yp, (wxCoord)x, (wxCoord)y);
}

void csEvtHandler::OnEndDragRight(double x, double y, int WXUNUSED(keys), int attachment)
{
  GetShape()->GetCanvas()->ReleaseMouse();
  csCanvas *canvas = (csCanvas *)GetShape()->GetCanvas();

  // Check if we're on an object
  int new_attachment;
  wxShape *otherShape = canvas->FindFirstSensitiveShape(x, y, &new_attachment, OP_DRAG_RIGHT);

  if (otherShape && !otherShape->IsKindOf(CLASSINFO(wxLineShape)))
  {
        wxLineShape* theShape = new csLineShape;

        theShape->AssignNewIds();
        theShape->SetEventHandler(new csEvtHandler(theShape, theShape, wxEmptyString));
        theShape->SetPen(wxBLACK_PEN);
        theShape->SetBrush(wxRED_BRUSH);

        wxToolBar* toolbar = wxGetApp().GetDiagramToolBar();
        bool haveArrow = toolbar->GetToolState(DIAGRAM_TOOLBAR_LINE_ARROW);

        wxLineShape *lineShape = (wxLineShape *)theShape;

        // Yes, you can have more than 2 control points, in which case
        // it becomes a multi-segment line.
        lineShape->MakeLineControlPoints(2);

        if (haveArrow)
            lineShape->AddArrow(ARROW_ARROW, ARROW_POSITION_MIDDLE, 10.0, 0.0, _T("Normal arrowhead"));

        lineShape->SetFrom(GetShape());
        lineShape->SetTo(otherShape);
        lineShape->SetAttachments(attachment, new_attachment);

        canvas->GetView()->GetDocument()->GetCommandProcessor()->Submit(
            new csDiagramCommand(_T("Line"), (csDiagramDocument *)canvas->GetView()->GetDocument(),
                    new csCommandState(ID_CS_ADD_LINE, lineShape, NULL)));
  }
}

static double g_DragOffsetX = 0.0;
static double g_DragOffsetY = 0.0;
static double g_DragStartX = 0.0;
static double g_DragStartY = 0.0;

void csEvtHandler::OnDragLeft(bool draw, double x, double y, int keys, int attachment)
{
  if ((GetShape()->GetSensitivityFilter() & OP_DRAG_LEFT) != OP_DRAG_LEFT)
  {
    attachment = 0;
    double dist;
    if (GetShape()->GetParent())
    {
      GetShape()->GetParent()->HitTest(x, y, &attachment, &dist);
      GetShape()->GetParent()->GetEventHandler()->OnDragLeft(draw, x, y, keys, attachment);
    }
    return;
  }

  wxClientDC dc(GetShape()->GetCanvas());
  GetShape()->GetCanvas()->PrepareDC(dc);

  dc.SetLogicalFunction(OGLRBLF);

  wxPen dottedPen(*wxBLACK, 1, wxDOT);
  dc.SetPen(dottedPen);
  dc.SetBrush(* wxTRANSPARENT_BRUSH);

  double xx, yy;
  xx = x + g_DragOffsetX;
  yy = y + g_DragOffsetY;

  GetShape()->GetCanvas()->Snap(&xx, &yy);

  double offsetX = xx - g_DragStartX;
  double offsetY = yy - g_DragStartY;

//  m_xpos = xx; m_ypos = yy;
  double w, h;
  GetShape()->GetBoundingBoxMax(&w, &h);
  GetShape()->GetEventHandler()->OnDrawOutline(dc, xx, yy, w, h);

  // Draw bounding box for other selected shapes
  wxObjectList::compatibility_iterator node = GetShape()->GetCanvas()->GetDiagram()->GetShapeList()->GetFirst();
  while (node)
  {
     wxShape* shape = (wxShape*) node->GetData();
     if (shape->Selected() && !shape->IsKindOf(CLASSINFO(wxLineShape)) && (shape != GetShape()))
     {
        shape->GetBoundingBoxMax(&w, &h);
        shape->OnDrawOutline(dc, shape->GetX() + offsetX, shape->GetY() + offsetY, w, h);
     }
     node = node->GetNext();
  }
}

void csEvtHandler::OnBeginDragLeft(double x, double y, int keys, int attachment)
{
  if ((GetShape()->GetSensitivityFilter() & OP_DRAG_LEFT) != OP_DRAG_LEFT)
  {
    attachment = 0;
    double dist;
    if (GetShape()->GetParent())
    {
      GetShape()->GetParent()->HitTest(x, y, &attachment, &dist);
      GetShape()->GetParent()->GetEventHandler()->OnBeginDragLeft(x, y, keys, attachment);
    }
    return;
  }

  wxClientDC dc(GetShape()->GetCanvas());
  GetShape()->GetCanvas()->PrepareDC(dc);

  // New policy: don't erase shape until end of drag.
//  Erase(dc);

  g_DragOffsetX = GetShape()->GetX() - x;
  g_DragOffsetY = GetShape()->GetY() - y;

  double xx, yy;
  xx = x + g_DragOffsetX;
  yy = y + g_DragOffsetY;

  GetShape()->GetCanvas()->Snap(&xx, &yy);

  g_DragStartX = GetShape()->GetX();
  g_DragStartY = GetShape()->GetY();

  double offsetX = xx - g_DragStartX;
  double offsetY = yy - g_DragStartY;

  dc.SetLogicalFunction(OGLRBLF);

  wxPen dottedPen(*wxBLACK, 1, wxDOT);
  dc.SetPen(dottedPen);
  dc.SetBrush((* wxTRANSPARENT_BRUSH));

  double w, h;
  GetShape()->GetBoundingBoxMax(&w, &h);
  GetShape()->GetEventHandler()->OnDrawOutline(dc, xx, yy, w, h);

  // Draw bounding box for other selected shapes
  wxObjectList::compatibility_iterator node = GetShape()->GetCanvas()->GetDiagram()->GetShapeList()->GetFirst();
  while (node)
  {
     wxShape* shape = (wxShape*) node->GetData();
     if (shape->Selected() && !shape->IsKindOf(CLASSINFO(wxLineShape)) && (shape != GetShape()))
     {
        shape->GetBoundingBoxMax(&w, &h);
        shape->OnDrawOutline(dc, shape->GetX() + offsetX, shape->GetY() + offsetY, w, h);
     }
     node = node->GetNext();
  }

  GetShape()->GetCanvas()->CaptureMouse();
}


void csEvtHandler::OnEndDragLeft(double x, double y, int keys, int attachment)
{
  csCanvas *canvas = (csCanvas *)GetShape()->GetCanvas();

  canvas->ReleaseMouse();
  if ((GetShape()->GetSensitivityFilter() & OP_DRAG_LEFT) != OP_DRAG_LEFT)
  {
    attachment = 0;
    double dist;
    if (GetShape()->GetParent())
    {
      GetShape()->GetParent()->HitTest(x, y, &attachment, &dist);
      GetShape()->GetParent()->GetEventHandler()->OnEndDragLeft(x, y, keys, attachment);
    }
    return;
  }

  wxClientDC dc(canvas);
  canvas->PrepareDC(dc);

  dc.SetLogicalFunction(wxCOPY);

  double xx = x + g_DragOffsetX;
  double yy = y + g_DragOffsetY;

  canvas->Snap(&xx, &yy);

  double offsetX = xx - g_DragStartX;
  double offsetY = yy - g_DragStartY;

  wxShape* newShape = GetShape()->CreateNewCopy();

  newShape->SetX(xx);
  newShape->SetY(yy);

  csDiagramCommand* cmd = new csDiagramCommand(_T("Move"), (csDiagramDocument*)canvas->GetView()->GetDocument(),
                new csCommandState(ID_CS_MOVE, newShape, GetShape()));

  // Move line points
  wxObjectList::compatibility_iterator node = GetShape()->GetCanvas()->GetDiagram()->GetShapeList()->GetFirst();
  while (node)
  {
     wxShape* shape = (wxShape*) node->GetData();
     // Only move the line point(s) if both ends move too
     if (shape->IsKindOf(CLASSINFO(wxLineShape)) &&
           ((wxLineShape*)shape)->GetTo()->Selected() && ((wxLineShape*)shape)->GetFrom()->Selected())
     {
        wxLineShape* lineShape = (wxLineShape*) shape;

        if (lineShape->GetLineControlPoints()->GetCount() > 2)
        {
            wxLineShape* newLineShape = (wxLineShape*) lineShape->CreateNewCopy();

            wxObjectList::compatibility_iterator node1 = newLineShape->GetLineControlPoints()->GetFirst();
            while (node1)
            {
                wxRealPoint *point = (wxRealPoint *)node1->GetData();
                point->x += offsetX;
                point->y += offsetY;
                node1 = node1->GetNext();
            }
            cmd->AddState(new csCommandState(ID_CS_MOVE_LINE_POINT, newLineShape, lineShape));
            lineShape->Erase(dc);
        }
     }
     node = node->GetNext();
  }

  // Add other selected node shapes, if any
  node = GetShape()->GetCanvas()->GetDiagram()->GetShapeList()->GetFirst();
  while (node)
  {
     wxShape* shape = (wxShape*) node->GetData();
     if (shape->Selected() && !shape->IsKindOf(CLASSINFO(wxLineShape)) && (shape != GetShape()))
     {
        wxShape* newShape2 = shape->CreateNewCopy();
        newShape2->SetX(shape->GetX() + offsetX);
        newShape2->SetY(shape->GetY() + offsetY);
        cmd->AddState(new csCommandState(ID_CS_MOVE, newShape2, shape));
     }
     node = node->GetNext();
  }

  canvas->GetView()->GetDocument()->GetCommandProcessor()->Submit(cmd);
}

void csEvtHandler::OnSizingEndDragLeft(wxControlPoint* pt, double x, double y, int keys, int attachment)
{
  wxShape* shape = GetShape();
  csCanvas *canvas = (csCanvas *)GetShape()->GetCanvas();

  if (shape->IsKindOf(CLASSINFO(wxLineShape)))
  {
    // TODO: Do/Undo support for line operations
    ((wxLineShape*)shape)->wxLineShape::OnSizingEndDragLeft(pt, x, y, keys, attachment);
#if 0
        wxLineShape* lineShape = (wxLineShape*) shape;

        wxLineControlPoint* lpt = (wxLineControlPoint*) pt;

        wxClientDC dc(canvas);
        canvas->PrepareDC(dc);

        shape->SetDisableLabel(false);

        if (lpt->m_type == CONTROL_POINT_LINE)
        {
            canvas->Snap(&x, &y);

            dc.SetLogicalFunction(wxCOPY);
            lpt->SetX(x); lpt->SetY(y);
            lpt->m_point->x = x; lpt->m_point->y = y;

            this->OnMoveLink(dc);
        }
        if (lpt->m_type == CONTROL_POINT_ENDPOINT_FROM)
        {
            if (lpt->m_oldCursor)
                canvas->SetCursor(lpt->m_oldCursor);
            lineShape->Erase(dc);

            lpt->SetX(x); lpt->SetY(y);

            if (lineShape->GetFrom())
            {
                lineShape->GetFrom()->MoveLineToNewAttachment(dc, lineShape, x, y);
            }
        }
        if (lpt->m_type == CONTROL_POINT_ENDPOINT_TO)
        {
            if (lpt->m_oldCursor)
                canvas->SetCursor(lpt->m_oldCursor);

            lpt->SetX(x); lpt->SetY(y);

            if (lineShape->GetTo())
            {
                lineShape->GetTo()->MoveLineToNewAttachment(dc, lineShape, x, y);
            }
        }
#endif
        return;
  }

  wxClientDC dc(canvas);
  canvas->PrepareDC(dc);

  canvas->ReleaseMouse();
  dc.SetLogicalFunction(wxCOPY);

//  shape->Erase(dc);
/*
  shape->Recompute();
  shape->ResetControlPoints();
  if (!pt->m_eraseObject)
    shape->Show(false);
*/

  wxShape* newShape = shape->CreateNewCopy();

  if (newShape->IsKindOf(CLASSINFO(wxPolygonShape)))
  {
    wxPolygonControlPoint* ppt = (wxPolygonControlPoint*) pt;
    newShape->SetSize(ppt->GetNewSize().x, ppt->GetNewSize().y);

    ((wxPolygonShape *)newShape)->CalculateBoundingBox();
    ((wxPolygonShape *)newShape)->CalculatePolygonCentre();
    newShape->ResetControlPoints();
  }
  else
  {
    newShape->SetSize(pt->sm_controlPointDragEndWidth, pt->sm_controlPointDragEndHeight);
    if (shape->GetCentreResize())
    {
      // Old position is fine
    }
    else
    {
      newShape->SetX(pt->sm_controlPointDragPosX);
      newShape->SetY(pt->sm_controlPointDragPosY);
    }
  }

  csDiagramCommand* cmd = new csDiagramCommand(_T("Size"), (csDiagramDocument*)canvas->GetView()->GetDocument(),
                new csCommandState(ID_CS_SIZE, newShape, shape));

  canvas->GetView()->GetDocument()->GetCommandProcessor()->Submit(cmd);

}

void csEvtHandler::OnEndSize(double WXUNUSED(x), double WXUNUSED(y))
{
  wxClientDC dc(GetShape()->GetCanvas());
  GetShape()->GetCanvas()->PrepareDC(dc);

  GetShape()->FormatText(dc, m_label);
}

void csEvtHandler::OnChangeAttachment(int attachment, wxLineShape* line, wxList& ordering)
{
    csCanvas *canvas = (csCanvas *)GetShape()->GetCanvas();

    // We actually submit two different states: one to change the ordering, and another
    // to change the attachment for the line.
    // Problem. If we refresh after the attachment change, we'll get a flicker.
    // We really want to do both in a oner.

    csDiagramCommand* cmd = new csDiagramCommand(_T("Change attachment"), (csDiagramDocument*)canvas->GetView()->GetDocument());

    wxLineShape* newLine = (wxLineShape*) line->CreateNewCopy();
    if (line->GetTo() == GetShape())
        newLine->SetAttachmentTo(attachment);
    else
        newLine->SetAttachmentFrom(attachment);

    cmd->AddState(new csCommandState(ID_CS_CHANGE_LINE_ATTACHMENT, newLine, line));

    // Change ordering
    wxShape* newShape = GetShape()->CreateNewCopy();
    newShape->ApplyAttachmentOrdering(ordering);

    cmd->AddState(new csCommandState(ID_CS_CHANGE_LINE_ORDERING, newShape, GetShape()));

    canvas->GetView()->GetDocument()->GetCommandProcessor()->Submit(cmd);
}

void csEvtHandler::OnLeftDoubleClick(double WXUNUSED(x), double WXUNUSED(y), int WXUNUSED(keys), int WXUNUSED(attachment))
{
    EditProperties();
}

// Popup up a property dialog
bool csEvtHandler::EditProperties()
{
    wxShape* shape = GetShape();

    // For now, no line property editing
    if (shape->IsKindOf(CLASSINFO(wxLineShape)))
        return false;

    csDiagramView* view = ((csCanvas*)shape->GetCanvas())->GetView();

    wxPanel* attributeDialog;
    wxString attributeDialogName;
    wxString title;

    if (shape->IsKindOf(CLASSINFO(csThinRectangleShape)))
    {
        attributeDialog = new csThinRectangleDialog;
        attributeDialogName = _T("thin_rectangle");
        title = _T("Thin Rectangle Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csWideRectangleShape)))
    {
        attributeDialog = new csWideRectangleDialog;
        attributeDialogName = _T("wide_rectangle");
        title = _T("Wide Rectangle Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csTriangleShape)))
    {
        attributeDialog = new csTriangleDialog;
        attributeDialogName = _T("triangle");
        title = _T("Triangle Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csSemiCircleShape)))
    {
        attributeDialog = new csSemiCircleDialog;
        attributeDialogName = _T("semi_circle");
        title = _T("Semicircle Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csCircleShape)))
    {
        attributeDialog = new csCircleDialog;
        attributeDialogName = _T("circle");
        title = _T("Circle Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csCircleShadowShape)))
    {
        attributeDialog = new csCircleShadowDialog;
        attributeDialogName = _T("circle_shadow");
        title = _T("Circle Shadow Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csTextBoxShape)))
    {
        attributeDialog = new csTextBoxDialog;
        attributeDialogName = _T("text_box");
        title = _T("Text Box Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csGroupShape)))
    {
        attributeDialog = new csGroupDialog;
        attributeDialogName = _T("group");
        title = _T("Group Properties");
    }
    else if (shape->IsKindOf(CLASSINFO(csOctagonShape)))
    {
        attributeDialog = new csOctagonDialog;
        attributeDialogName = _T("octagon");
        title = _T("Octagon Properties");
    }
    else
    {
        wxMessageBox(_T("Unrecognised shape."), _T("Studio"), wxICON_EXCLAMATION);
        return false;
    }

    wxString newLabel(m_label);

#if wxUSE_WX_RESOURCES
    csShapePropertiesDialog* dialog = new csShapePropertiesDialog(shape->GetCanvas()->GetParent(), title, attributeDialog, attributeDialogName);
    dialog->GetGeneralPropertiesDialog()->SetShapeLabel(m_label);
    if (dialog->ShowModal() == wxID_CANCEL)
    {
        dialog->Destroy();
        return false;
    }

    newLabel = dialog->GetGeneralPropertiesDialog()->GetShapeLabel();
    dialog->Destroy();
#else
    wxUnusedVar(attributeDialog);
#endif // wxUSE_WX_RESOURCES

    wxShape* newShape = shape->CreateNewCopy();

    csEvtHandler* handler2 = (csEvtHandler *)newShape->GetEventHandler();
    handler2->m_label = newLabel;

    view->GetDocument()->GetCommandProcessor()->Submit(new csDiagramCommand(_T("Edit properties"), (csDiagramDocument*) view->GetDocument(),
                new csCommandState(ID_CS_EDIT_PROPERTIES, newShape, shape)));

    return true;
}

/*
 * Diagram
 */

#if wxUSE_PROLOGIO
bool csDiagram::OnShapeSave(wxExprDatabase& db, wxShape& shape, wxExpr& expr)
{
  wxDiagram::OnShapeSave(db, shape, expr);
  csEvtHandler *handler = (csEvtHandler *)shape.GetEventHandler();
  expr.AddAttributeValueString(_T("label"), handler->m_label);
  return true;
}

bool csDiagram::OnShapeLoad(wxExprDatabase& db, wxShape& shape, wxExpr& expr)
{
  wxDiagram::OnShapeLoad(db, shape, expr);
  wxString label = wxEmptyString;
  expr.GetAttributeValue(_T("label"), label);
  csEvtHandler *handler = new csEvtHandler(&shape, &shape, label);
  shape.SetEventHandler(handler);

  return true;
}
#endif // wxUSE_PROLOGIO

IMPLEMENT_DYNAMIC_CLASS(csThinRectangleShape, wxDrawnShape)

csThinRectangleShape::csThinRectangleShape()
{
    SetDrawnPen(wxBLACK_PEN);
    wxBrush* brush = wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID);
    SetDrawnBrush(brush);

    double w = csSTANDARD_SHAPE_WIDTH/2;
    double h = csSTANDARD_SHAPE_WIDTH;

    DrawRectangle(wxRect((int)(- w/2), (int)(- h/2), (int)(w), (int)(h)));
    CalculateSize();

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

IMPLEMENT_DYNAMIC_CLASS(csWideRectangleShape, wxDrawnShape)

csWideRectangleShape::csWideRectangleShape()
{
    SetDrawnPen(wxBLACK_PEN);
    wxBrush* brush = wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID);
    SetDrawnBrush(brush);

    double w = csSTANDARD_SHAPE_WIDTH;
    double h = w/2.0;

    DrawRoundedRectangle(wxRect((int)(- w/2), (int)(- h/2), (int)(w), (int)(h)), -0.3);
    CalculateSize();

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

IMPLEMENT_DYNAMIC_CLASS(csTriangleShape, wxDrawnShape)

csTriangleShape::csTriangleShape()
{
    SetDrawnPen(wxBLACK_PEN);
    wxBrush* brush = wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID);
    SetDrawnBrush(brush);

    double w = csSTANDARD_SHAPE_WIDTH;
    double h = (csSTANDARD_SHAPE_WIDTH*2.0)/3.0;

    // Triangle, from top vertex
    wxPoint* points = new wxPoint[3];


    points[0] = wxPoint( 0 ,  (int)(- h / 2) );
    points[1] = wxPoint( (int)(w / 2) ,  (int)(h / 2) );
    points[2] = wxPoint( (int)(-w / 2),  (int)(h / 2) );

    DrawPolygon(3, points, oglMETAFLAGS_OUTLINE);

    delete[] points;

    // Add another triangle at the top for the black bit
    SetDrawnBrush(wxBLACK_BRUSH);

    points = new wxPoint[3];

    // Calculate where the new points will be, using the proportions
    // of the triangle.
    double h1 = 8; // Height of little triangle.

    /*
        Formula: ((w/2) / h) = w1 / h1
        w1 = ((w/2) / h) * h1;
    */
    double ratio = ((w/2.0) / h) ;
    double w1 = ratio * h1;

    points[0] = wxPoint(0  ,  (int) (- h / 2 ));
    points[1] = wxPoint( (int) w1,  (int) (- h / 2 + h1));
    points[2] = wxPoint( (int) -w1, (int) (- h / 2 + h1));

    DrawPolygon(3, points);

    delete[] points;

    CalculateSize();

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

IMPLEMENT_DYNAMIC_CLASS(csSemiCircleShape, wxDrawnShape)

csSemiCircleShape::csSemiCircleShape()
{
    // Zero degrees
    DrawAtAngle(oglDRAWN_ANGLE_0);

    double w = csSTANDARD_SHAPE_WIDTH;
    double h = w/2.0;

    SetDrawnPen(wxTRANSPARENT_PEN);
    SetDrawnBrush(wxTRANSPARENT_BRUSH);

    // Draw a dummy rectangle that will be used for calculating the
    // bounding box, since we can't calculate the bounding box for
    // an arbitrary arc (not implemented)

    DrawRectangle(wxRect((int)(-w/2.0), (int)(-h/2.0), (int)(w), (int)(h)));

    SetDrawnPen(wxBLACK_PEN);
    wxBrush* brush = wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID);
    SetDrawnBrush(brush);

    DrawEllipticArc(wxRect((int)(-w/2), (int)(-h/2), (int)(w), (int)(2*h)), 0.0, 180.0);
    DrawLine(wxPoint((int)(-w/2), (int)(h/2)), wxPoint((int)(w/2), (int)(h/2)));

    CalculateSize();

    /// 90 degrees

    w = csSTANDARD_SHAPE_WIDTH/2;
    h = csSTANDARD_SHAPE_WIDTH;

    DrawAtAngle(oglDRAWN_ANGLE_90);

    SetDrawnPen(wxTRANSPARENT_PEN);
    SetDrawnBrush(wxTRANSPARENT_BRUSH);

    DrawRectangle(wxRect((int)(-w/2), (int)(-h/2), (int)(w), (int)(h)));

    SetDrawnPen(wxBLACK_PEN);
    SetDrawnBrush(brush);

    DrawEllipticArc(wxRect((int)(-w/2 - w), (int)(-h/2), (int)(2*w), (int)(h)), 270.0, 90.0);
    DrawLine(wxPoint((int)(-w/2), (int)(-h/2)), wxPoint((int)(-w/2), (int)(h/2)));

    CalculateSize();

    /// 180 degrees

    DrawAtAngle(oglDRAWN_ANGLE_180);

    w = csSTANDARD_SHAPE_WIDTH;
    h = csSTANDARD_SHAPE_WIDTH/2;

    SetDrawnPen(wxTRANSPARENT_PEN);
    SetDrawnBrush(wxTRANSPARENT_BRUSH);

    DrawRectangle(wxRect((int)(-w/2), (int)(-h/2), (int)(w), (int)(h)));

    SetDrawnPen(wxBLACK_PEN);
    SetDrawnBrush(brush);

    DrawEllipticArc(wxRect((int)(-w/2), (int)(-h/2 - h), (int)(w), (int)(2*h)), 180.0, 0.0);
    DrawLine(wxPoint((int)(-w/2), (int)(-h/2)), wxPoint((int)(w/2), (int)(-h/2)));

    CalculateSize();

    /// 270 degrees

    DrawAtAngle(oglDRAWN_ANGLE_270);

    w = csSTANDARD_SHAPE_WIDTH/2;
    h = csSTANDARD_SHAPE_WIDTH;

    SetDrawnPen(wxTRANSPARENT_PEN);
    SetDrawnBrush(wxTRANSPARENT_BRUSH);

    DrawRectangle(wxRect((int)(-w/2), (int)(-h/2), (int)(w), (int)(h)));

    SetDrawnPen(wxBLACK_PEN);
    SetDrawnBrush(brush);

    DrawEllipticArc(wxRect((int)(-w/2), (int)(-h/2), (int)(2*w), (int)(h)), 90.0, 270.0);
    DrawLine(wxPoint((int)(w/2),(int)(-h/2)), wxPoint((int)(w/2), (int)(h/2)));

    CalculateSize();

    // Reset to zero
    DrawAtAngle(oglDRAWN_ANGLE_0);
    CalculateSize();

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

IMPLEMENT_DYNAMIC_CLASS(csCircleShape, wxCircleShape)

csCircleShape::csCircleShape()
{
    SetPen(wxBLACK_PEN);
    wxBrush* brush = wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID);
    SetBrush(brush);

    SetSize(csSTANDARD_SHAPE_WIDTH*0.6, csSTANDARD_SHAPE_WIDTH*0.6);

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

IMPLEMENT_DYNAMIC_CLASS(csCircleShadowShape, wxCircleShape)

csCircleShadowShape::csCircleShadowShape()
{
    SetPen(wxBLACK_PEN);
    wxBrush* brush = wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID);
    SetBrush(brush);

    SetSize(csSTANDARD_SHAPE_WIDTH*0.6, csSTANDARD_SHAPE_WIDTH*0.6);

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
    SetShadowMode(SHADOW_RIGHT);
}

IMPLEMENT_DYNAMIC_CLASS(csOctagonShape, wxPolygonShape)

csOctagonShape::csOctagonShape()
{
    SetPen(wxBLACK_PEN);
    SetBrush(wxTheBrushList->FindOrCreateBrush(wxColour(220, 220, 220), wxSOLID));

    double w = csSTANDARD_SHAPE_WIDTH*0.5;
    double h = csSTANDARD_SHAPE_WIDTH*0.5;

    double prop = h/3.0;

    wxList* points = new wxList;
    points->Append((wxObject*) new wxRealPoint(-w/2.0 + prop, -h/2.0));
    points->Append((wxObject*) new wxRealPoint(w/2.0 - prop, -h/2.0));
    points->Append((wxObject*) new wxRealPoint(w/2.0, -h/2.0 + prop));
    points->Append((wxObject*) new wxRealPoint(w/2.0, h/2.0 - prop));
    points->Append((wxObject*) new wxRealPoint(w/2.0 - prop, h/2.0));
    points->Append((wxObject*) new wxRealPoint(-w/2.0 + prop, h/2.0));
    points->Append((wxObject*) new wxRealPoint(-w/2.0, h/2.0 - prop));
    points->Append((wxObject*) new wxRealPoint(-w/2.0, -h/2.0 + prop));

    Create(points);

    SetAttachmentMode(ATTACHMENT_MODE_BRANCHING);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

// This is a transparent shape for drawing around other shapes.
IMPLEMENT_DYNAMIC_CLASS(csGroupShape, wxRectangleShape)

csGroupShape::csGroupShape()
{
    SetPen(wxThePenList->FindOrCreatePen(_T("BLACK"), 1, wxDOT));
    SetBrush(wxTRANSPARENT_BRUSH);

    SetSize(csSTANDARD_SHAPE_WIDTH, csSTANDARD_SHAPE_WIDTH);
    SetCentreResize(false);
}

void csGroupShape::OnDraw(wxDC& dc)
{
    wxRectangleShape::OnDraw(dc);
}

// Must modify the hit-test so it doesn't obscure shapes that are inside.
bool csGroupShape::HitTest(double x, double y, int* attachment, double* distance)
{
    *attachment = 0;
    *distance = 0.0;

    double width = 0.0, height = 0.0;
    GetBoundingBoxMin(&width, &height);

    double x1 = GetX() - (width/2.0);
    double y1 = GetY() - (height/2.0);
    double x2 = GetX() + (width/2.0);
    double y2 = GetY() + (height/2.0);

    double edgeTolerance = 4.0;

    // Test each edge in turn

    // Top/bottom edges
    if (x >= x1 && x <= x2)
    {
        if ((y >= y1 - edgeTolerance) && (y <= y1 + edgeTolerance))
            return true;
        if ((y <= y2 + edgeTolerance) && (y >= y2 - edgeTolerance))
            return true;
    }
    // Left/right edges
    if (y >= y1 && y <= y2)
    {
        if ((x >= x1 - edgeTolerance) && (x <= x1 + edgeTolerance))
            return true;
        if ((x <= x2 + edgeTolerance) && (x >= x2 - edgeTolerance))
            return true;
    }

    return false;
}

IMPLEMENT_DYNAMIC_CLASS(csTextBoxShape, wxRectangleShape)

csTextBoxShape::csTextBoxShape()
{
    SetPen(wxTRANSPARENT_PEN);
    SetBrush(wxTRANSPARENT_BRUSH);

    SetSize(csSTANDARD_SHAPE_WIDTH, csSTANDARD_SHAPE_WIDTH/2.0);

    SetAttachmentMode(ATTACHMENT_MODE_NONE);
    SetBranchStyle(BRANCHING_ATTACHMENT_NORMAL|BRANCHING_ATTACHMENT_BLOB);
    SetCentreResize(false);
}

IMPLEMENT_DYNAMIC_CLASS(csLineShape, wxLineShape)

csLineShape::csLineShape()
{
}

bool csLineShape::OnMoveMiddleControlPoint(wxDC& WXUNUSED(dc), wxLineControlPoint* lpt, const wxRealPoint& pt)
{
    csDiagramView* view = ((csCanvas*)GetCanvas())->GetView();

    // Temporarily set the new shape properties so we can copy it
    lpt->SetX(pt.x); lpt->SetY(pt.y);
    lpt->m_point->x = pt.x; lpt->m_point->y = pt.y;

    wxLineShape* newShape = (wxLineShape*) this->CreateNewCopy();

    // Now set them back again
    lpt->SetX(lpt->m_originalPos.x); lpt->SetY(lpt->m_originalPos.y);
    lpt->m_point->x = lpt->m_originalPos.x; lpt->m_point->y = lpt->m_originalPos.y;

    view->GetDocument()->GetCommandProcessor()->Submit(new csDiagramCommand(_T("Move line point"), (csDiagramDocument*) view->GetDocument(),
                new csCommandState(ID_CS_MOVE_LINE_POINT, newShape, this)));

    return true;
}

wxLabelShape* csLineShape::OnCreateLabelShape(wxLineShape *parent, wxShapeRegion *region, double w, double h)
{
    return new csLabelShape(parent, region, w, h);
}

#if 0
bool csLineShape::OnLabelMovePre(wxDC& dc, wxLabelShape* labelShape, double x, double y, double old_x, double old_y, bool display)
{
    csDiagramView* view = ((csCanvas*)GetCanvas())->GetView();

    wxLineShape* newShape = (wxLineShape*) this->CreateNewCopy();

    wxLineShape::OnLabelMovePre(dc, labelShape, x, y, old_x, old_y, display);

    view->GetDocument()->GetCommandProcessor()->Submit(new csDiagramCommand("Move label", (csDiagramDocument*) view->GetDocument(),
                new csCommandState(ID_CS_MOVE_LABEL, newShape, this)));
  return true;
}
#endif

IMPLEMENT_DYNAMIC_CLASS(csLabelShape, wxLabelShape)

csLabelShape::csLabelShape(wxLineShape *parent, wxShapeRegion *region, double w, double h):
  wxLabelShape(parent, region, w, h)
{
}

// TODO: not sure how intercept normal behaviour (OnMovePre) to make
// label movement undo-able.
void csLabelShape::OnEndDragLeft(double x, double y, int keys, int attachment)
{
    wxLabelShape::OnEndDragLeft(x, y, keys, attachment);
}


// Menu for editing shapes
void studioShapeEditProc(wxMenu& menu, wxCommandEvent& event)
{
    wxShape* shape = (wxShape*) menu.GetClientData();
    csDiagramView* view = ((csCanvas*)shape->GetCanvas())->GetView();

    switch (event.GetId())
    {
        case ID_CS_EDIT_PROPERTIES:
        {
            csEvtHandler* handler1 = (csEvtHandler *)shape->GetEventHandler();
            handler1->EditProperties();
#if 0
            csEvtHandler* handler1 = (csEvtHandler *)shape->GetEventHandler();
            csLabelEditingDialog* dialog = new csLabelEditingDialog(shape->GetCanvas()->GetParent());
            dialog->SetShapeLabel(handler1->m_label);
            if (dialog->ShowModal() == wxID_CANCEL)
            {
                dialog->Destroy();
                return;
            }

            wxString newLabel = dialog->GetShapeLabel();
            dialog->Destroy();

            wxShape* newShape = shape->CreateNewCopy();

            csEvtHandler* handler2 = (csEvtHandler *)newShape->GetEventHandler();
            handler2->m_label = newLabel;

            view->GetDocument()->GetCommandProcessor()->Submit(new csDiagramCommand("Edit label", (csDiagramDocument*) view->GetDocument(),
                new csCommandState(ID_CS_EDIT_LABEL, newShape, shape)));
#endif
            break;
        }
        case wxID_CUT:
        {
            wxList list;
            list.Append(shape);
            view->DoCut(list);
            break;
        }
        case ID_CS_ROTATE_CLOCKWISE:
        case ID_CS_ROTATE_ANTICLOCKWISE:
        {
            if (shape->IsKindOf(CLASSINFO(wxLineShape)))
                break;

            double theta = shape->GetRotation();
            const double myPi = M_PI;
            double ninetyDegrees = myPi/2.0;

            wxString opStr;
            if (event.GetId() == ID_CS_ROTATE_CLOCKWISE)
            {
                theta += ninetyDegrees;
                opStr = _T("Rotate clockwise");
            }
            else
            {
                theta -= ninetyDegrees;
                opStr = _T("Rotate anticlockwise");
            }

            if (theta >= 2.0*myPi || theta < 0.0)
                theta = 0.0;
            wxShape* newShape = shape->CreateNewCopy();
            newShape->Rotate(0.0, 0.0, theta);
            wxList newShapes;
            wxList oldShapes;
            newShapes.Append(newShape);
            oldShapes.Append(shape);
            view->DoCmd(newShapes, oldShapes, event.GetId(), opStr);
            break;
        }
        default:
            break;
    }
}

BEGIN_EVENT_TABLE(ShapeEditMenu, wxMenu)
    EVT_COMMAND_RANGE(1, 65000, wxEVT_COMMAND_MENU_SELECTED, ShapeEditMenu::OnCommand)
END_EVENT_TABLE()

void ShapeEditMenu::OnCommand(wxCommandEvent& event)
{
    studioShapeEditProc(*this, event);
}
