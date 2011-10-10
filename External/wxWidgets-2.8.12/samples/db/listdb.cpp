///////////////////////////////////////////////////////////////////////////////
// Name:        listdb.cpp
// Purpose:     Data table lookup listbox code
// Author:      George Tasker/Doug Card
// Modified by:
// Created:     1996
// RCS-ID:      $Id: listdb.cpp 40587 2006-08-13 01:17:53Z VZ $
// Copyright:   (c) 1996 Remstar International, Inc.
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/*
// SYNOPSIS START

    Member functions for the classes defined in LISTDB.H

     This class is used to present a generic ListBox lookup window for
     use with any of the object creation/selection choice widgets.   This
     dialog window will present a (possibly) scrolling list of values
     that come from a data table source.  Based on the object type passed
     in the constructor, a ListBox is built to present the user with a
     single selection listbox.

     The string selected from the list box is stored in the Global variable
     "ListDB_Selection", and will remain set until another interation of this
     routine is called.

     For each object (database) type that is to be used, an overridden
     constructor should be written to appropriately link to the proper
     data table/object for building the list.

    The data table record access is all handled through the routines
    in this module, interfacing with the methods defined in wxDbTable.

     All objects which use data table access must be initialized and
     have opened the table prior to passing them in the dialog
     constructor, and the 'where' query should already have been set
     and performed before creating this dialog instance.

// SYNOPSIS STOP
*/

#include  "wx/wxprec.h"

#ifdef    __BORLANDC__
#pragma hdrstop
#endif  //__BORLANDC__

#ifndef WX_PRECOMP
#include  "wx/wx.h"
#endif //WX_PRECOMP

#include "wx/dbtable.h"

extern wxDbList WXDLLEXPORT *PtrBegDbList;    /* from db.cpp, used in getting back error results from db connections */

#include "listdb.h"
//#include "dbtest.h"

// Used for passing the selected listbox selection back to the calling
// routine.  This variable must be declared as 'extern' in the calling
// source module
wxChar ListDB_Selection[LOOKUP_COL_LEN+1];

// If the listbox contains two columns of data, the second column is
// returned in this variable.
wxChar ListDB_Selection2[LOOKUP_COL_LEN+1];

// Constants
const int LISTDB_NO_SPACES_BETWEEN_COLS = 3;


extern wxApp *DatabaseDemoApp;


/*
 * This function will return the exact string(s) from the database engine
 * indicating all error conditions which have just occurred during the
 * last call to the database engine.
 *
 * This demo uses the returned string by displaying it in a wxMessageBox.  The
 * formatting therefore is not the greatest, but this is just a demo, not a
 * finished product.  :-) gt
 *
 * NOTE: The value returned by this function is for temporary use only and
 *       should be copied for long term use
 */
wxString GetExtendedDBErrorMsg2(wxDb *pDb, wxChar *ErrFile, int ErrLine)
{
    static wxString msg;
    msg = wxT("");

    wxString tStr;

    if (ErrFile || ErrLine)
    {
        msg += wxT("File: ");
        msg += ErrFile;
        msg += wxT("   Line: ");
        tStr.Printf(wxT("%d"),ErrLine);
        msg += tStr.c_str();
        msg += wxT("\n");
    }

    msg.Append (wxT("\nODBC errors:\n"));
    msg += wxT("\n");

    // Display errors for this connection
    int i;
    for (i = 0; i < DB_MAX_ERROR_HISTORY; i++)
    {
        if (pDb->errorList[i])
        {
            msg.Append(pDb->errorList[i]);
            if (wxStrcmp(pDb->errorList[i],wxT("")) != 0)
                msg.Append(wxT("\n"));
            // Clear the errmsg buffer so the next error will not
            // end up showing the previous error that have occurred
            wxStrcpy(pDb->errorList[i],wxT(""));
        }
    }
    msg += wxT("\n");

    return msg;
}  // GetExtendedDBErrorMsg


// Clookup constructor
Clookup::Clookup(wxString tblName, wxString colName, wxDb *pDb, const wxString &defDir)
   : wxDbTable(pDb, tblName, 1, (const wxString &)wxEmptyString, !wxDB_QUERY_ONLY,
               defDir)
{

    SetColDefs (0, colName, DB_DATA_TYPE_VARCHAR, lookupCol, SQL_C_WXCHAR, LOOKUP_COL_LEN+1, false, false);

}  // Clookup()


// Clookup2 constructor
Clookup2::Clookup2(wxString tblName, wxString colName1, wxString colName2,
                   wxDb *pDb, const wxString &defDir)
   : wxDbTable(pDb, tblName, (UWORD)(1 + (wxStrlen(colName2) > 0)), (const wxString &)wxEmptyString,
               !wxDB_QUERY_ONLY, defDir)
{
    wxASSERT(pDb);
    wxASSERT(tblName);
    wxASSERT(colName1);
    wxASSERT(colName2);

    int i = 0;

    SetColDefs ((UWORD)i, colName1, DB_DATA_TYPE_VARCHAR, lookupCol1, SQL_C_WXCHAR, LOOKUP_COL_LEN+1, false, false);

    if (wxStrlen(colName2) > 0)
        SetColDefs ((UWORD)(++i), colName2, DB_DATA_TYPE_VARCHAR, lookupCol2, SQL_C_WXCHAR, LOOKUP_COL_LEN+1, false, false);

}  // Clookup2()


BEGIN_EVENT_TABLE(ClookUpDlg, wxDialog)
    EVT_BUTTON(LOOKUP_DIALOG_OK,  ClookUpDlg::OnButton)
    EVT_BUTTON(LOOKUP_DIALOG_CANCEL,  ClookUpDlg::OnButton)
    EVT_CLOSE(ClookUpDlg::OnClose)
    EVT_LISTBOX_DCLICK(LOOKUP_DIALOG_SELECT,  ClookUpDlg::OnDClick)
END_EVENT_TABLE()


// This is a generic lookup constructor that will work with any table and any column
ClookUpDlg::ClookUpDlg(wxWindow *parent, const wxString &windowTitle, const wxString &tableName,
                       const wxString &colName, const wxString &where, const wxString &orderBy,
                       wxDb *pDb, const wxString &defDir)
   : wxDialog (parent, LOOKUP_DIALOG, wxT("Select..."), wxDefaultPosition, wxSize(400, 290))
{
    wxBeginBusyCursor();

    wxStrcpy(ListDB_Selection,wxT(""));
    widgetPtrsSet = false;
    lookup  = 0;
    lookup2 = 0;
    noDisplayCols = 1;
    col1Len = 0;

    pLookUpSelectList = new wxListBox(this, LOOKUP_DIALOG_SELECT,                  wxPoint(  5,  15), wxSize(384, 195), 0, 0, wxLB_SINGLE|wxLB_ALWAYS_SB, wxDefaultValidator, wxT("LookUpSelectList"));
    pLookUpOkBtn      = new wxButton(this,  LOOKUP_DIALOG_OK,      wxT("&Ok"),     wxPoint(113, 222), wxSize( 70,  35), 0, wxDefaultValidator, wxT("LookUpOkBtn"));
    pLookUpCancelBtn  = new wxButton(this,  LOOKUP_DIALOG_CANCEL,  wxT("C&ancel"), wxPoint(212, 222), wxSize( 70,  35), 0, wxDefaultValidator, wxT("LookUpCancelBtn"));

    widgetPtrsSet = true;

    // Query the lookup table and display the result set
    lookup = new Clookup(tableName, colName, pDb, defDir);
    if (!lookup)
    {
        wxMessageBox(wxT("Error allocating memory for 'Clookup' object."),wxT("Error..."));
        Close();
        return;
    }

    if (!lookup->Open())
    {
        wxString tStr;
        tStr.Printf(wxT("Unable to open the table '%s'."), tableName.c_str());
        wxMessageBox(wxDbLogExtendedErrorMsg(tStr.c_str(),lookup->GetDb(),__TFILE__,__LINE__),
                    wxT("ODBC Error..."),wxOK | wxICON_EXCLAMATION);

        Close();
        return;
    }

    lookup->SetOrderByClause(orderBy);
    lookup->SetWhereClause(where);
    if (!lookup->Query())
    {
        wxString tStr;
        tStr = wxT("ODBC error during Query()\n\n");
        wxMessageBox(wxDbLogExtendedErrorMsg(tStr.c_str(),lookup->GetDb(),__TFILE__,__LINE__),
                    wxT("ODBC Error..."),wxOK | wxICON_EXCLAMATION);

        Close();
        return;
    }

    // Fill in the list box from the query result set
    while (lookup->GetNext())
        pLookUpSelectList->Append(lookup->lookupCol);

    // Make the OK activate by pressing Enter
    if (pLookUpSelectList->GetCount())
    {
        pLookUpSelectList->SetSelection(0);
        pLookUpOkBtn->SetDefault();
    }
    else
    {
        pLookUpCancelBtn->SetDefault();
        pLookUpOkBtn->Enable(false);
    }

    // Display the dialog window
    SetTitle(windowTitle);
    Centre(wxBOTH);
    wxEndBusyCursor();
    ShowModal();

}  // Generic lookup constructor


//
// This is a generic lookup constructor that will work with any table and any column.
// It extends the capabilites of the lookup dialog in the following ways:
//
//    1) 2 columns rather than one
// 2) The ability to select DISTINCT column values
//
// Only set distinctValues equal to true if necessary.  In many cases, the constraints
// of the index(es) will enforce this uniqueness.  Selecting DISTINCT does require
// overhead by the database to ensure that all values returned are distinct.  Therefore,
// use this ONLY when you need it.
//
// For complicated queries, you can pass in the sql select statement.  This would be
// necessary if joins are involved since by default both columns must come from the
// same table.
//
// If you do query by sql statement, you must pass in the maximum length of column1,
// since it cannot be derived when you query using your own sql statement.
//
// The optional database connection can be used if you'd like the lookup class
// to use a database pointer other than wxGetApp().READONLY_DB.  This is necessary if
// records are being saved, but not committed to the db, yet should be included
// in the lookup window.
//
ClookUpDlg::ClookUpDlg(wxWindow *parent, const wxString &windowTitle, const wxString &tableName,
                       const wxString &dispCol1, const wxString &dispCol2,
                       const wxString &where, const wxString &orderBy,
                       wxDb *pDb, const wxString &defDir, bool distinctValues,
                       const wxString &selectStmt, int maxLenCol1, bool allowOk)
   : wxDialog (parent, LOOKUP_DIALOG, wxT("Select..."), wxDefaultPosition, wxSize(400, 290))
{
    wxBeginBusyCursor();

    wxStrcpy(ListDB_Selection,wxT(""));
    wxStrcpy(ListDB_Selection2,wxT(""));
    widgetPtrsSet = false;
    lookup  = 0;
    lookup2 = 0;

    noDisplayCols = (dispCol2.Length() == 0 ? 1 : 2);
    col1Len = 0;

    wxFont fixedFont(12, wxMODERN, wxNORMAL, wxNORMAL);

    // this is done with fixed font so that the second column (if any) will be left
    // justified in the second column
    pLookUpSelectList        = new wxListBox(this, LOOKUP_DIALOG_SELECT, wxPoint(5, 15), wxSize(384, 195), 0, 0, wxLB_SINGLE|wxLB_ALWAYS_SB, wxDefaultValidator, wxT("LookUpSelectList"));

    pLookUpSelectList->SetFont(fixedFont);

    pLookUpOkBtn            = new wxButton(this, LOOKUP_DIALOG_OK,      wxT("&Ok"),        wxPoint(113, 222), wxSize(70, 35), 0, wxDefaultValidator, wxT("LookUpOkBtn"));
    pLookUpCancelBtn        = new wxButton(this, LOOKUP_DIALOG_CANCEL,  wxT("C&ancel"),    wxPoint(212, 222), wxSize(70, 35), 0, wxDefaultValidator, wxT("LookUpCancelBtn"));

    widgetPtrsSet = true;

    // Query the lookup table and display the result set
    lookup2 = new Clookup2(tableName, dispCol1, dispCol2, pDb, defDir);
    if (!lookup2)
    {
        wxMessageBox(wxT("Error allocating memory for 'Clookup2' object."),wxT("Error..."));
        Close();
        return;
    }

    if (!lookup2->Open())
    {
        wxString tStr;
        tStr.Printf(wxT("Unable to open the table '%s'."),tableName.c_str());
        tStr += GetExtendedDBErrorMsg2(pDb,__TFILE__,__LINE__);
        wxMessageBox(tStr,wxT("ODBC Error..."));
        Close();
        return;
    }

    // If displaying 2 columns, determine the maximum length of column1
    int maxColLen;
    if (maxLenCol1 > 0)
        maxColLen = col1Len = maxLenCol1;  // user passed in max col length for column 1
    else
    {
        // NOTE: Some databases (Firebird/Interbase) cannot handle the "fn" and "MAX()" functions

        maxColLen = LOOKUP_COL_LEN;
        if (wxStrlen(dispCol2))
        {
            wxString q = wxT("SELECT MAX({fn LENGTH(");
            q += dispCol1;
            q += wxT(")}), NULL");
            q += wxT(" FROM ");
            q += tableName;
            if (wxStrlen(where))
            {
                q += wxT(" WHERE ");
                q += where;
            }
            if (!lookup2->QueryBySqlStmt(q))
            {
                wxString tStr;
                tStr = wxT("ODBC error during QueryBySqlStmt()\n\n");
                wxMessageBox(wxDbLogExtendedErrorMsg(tStr.c_str(),lookup2->GetDb(),__TFILE__,__LINE__),
                            wxT("ODBC Error..."),wxOK | wxICON_EXCLAMATION);

                Close();
                return;
            }
            if (lookup2->GetNext())
                maxColLen = col1Len = wxAtoi(lookup2->lookupCol1);
            else
            {
                wxString tStr;
                tStr = wxT("ODBC error during GetNext()\n\n");
                wxMessageBox(wxDbLogExtendedErrorMsg(tStr.c_str(),lookup2->GetDb(),__TFILE__,__LINE__),
                            wxT("ODBC Error..."),wxOK | wxICON_EXCLAMATION);
            }
        }
    }

    // Query the actual record set
    if (selectStmt && wxStrlen(selectStmt))    // Query by sql stmt passed in
    {
        if (!lookup2->QueryBySqlStmt(selectStmt))
        {
            wxString tStr;
            tStr = wxT("ODBC error during QueryBySqlStmt()\n\n");
            wxMessageBox(wxDbLogExtendedErrorMsg(tStr.c_str(),lookup2->GetDb(),__TFILE__,__LINE__),
                        wxT("ODBC Error..."),wxOK | wxICON_EXCLAMATION);

            Close();
            return;
        }
    }
    else    // Query using where and order by clauses
    {
        lookup2->SetOrderByClause(orderBy);
        lookup2->SetWhereClause(where);
        if (!lookup2->Query(false, distinctValues))
        {
            wxString tStr;
            tStr = wxT("ODBC error during Query()\n\n");
            wxMessageBox(wxDbLogExtendedErrorMsg(tStr.c_str(),lookup2->GetDb(),__TFILE__,__LINE__),
                        wxT("ODBC Error..."),wxOK | wxICON_EXCLAMATION);

            Close();
            return;
        }
    }

    // Fill in the list box from the query result set
    wxString s;
    while (lookup2->GetNext())
    {
        s = lookup2->lookupCol1;
        if (wxStrlen(dispCol2))        // Append the optional column 2
        {
            s.Append(wxT(' '), (maxColLen + LISTDB_NO_SPACES_BETWEEN_COLS - wxStrlen(lookup2->lookupCol1)));
            s.Append(lookup2->lookupCol2);
        }
        pLookUpSelectList->Append(s);
    }

    // Make the OK activate by pressing Enter
    if (pLookUpSelectList->GetCount())
    {
        pLookUpSelectList->SetSelection(0);
        pLookUpOkBtn->SetDefault();
    }
    else
    {
        pLookUpCancelBtn->SetDefault();
        pLookUpOkBtn->Enable(false);
    }

    pLookUpOkBtn->Enable(allowOk);

    // Display the dialog window
    SetTitle(windowTitle);
    Centre(wxBOTH);
    wxEndBusyCursor();
    ShowModal();

}  // Generic lookup constructor 2


void ClookUpDlg::OnClose(wxCloseEvent& event)
{
    widgetPtrsSet = false;
    GetParent()->Enable(true);

    if (lookup)
        delete lookup;
    if (lookup2)
        delete lookup2;

     SetReturnCode(1);

    while (wxIsBusy()) wxEndBusyCursor();
    event.Skip();

//    return true;

}  // ClookUpDlg::OnClose


void ClookUpDlg::OnDClick( wxCommandEvent &event )
{
  wxWindow *win = (wxWindow*) event.GetEventObject();
  OnCommand( *win, event );
}


void ClookUpDlg::OnButton( wxCommandEvent &event )
{
  wxWindow *win = (wxWindow*) event.GetEventObject();
  OnCommand( *win, event );
}


void ClookUpDlg::OnCommand(wxWindow& win, wxCommandEvent& WXUNUSED(event))
{
    wxString widgetName = win.GetName();

    if (widgetPtrsSet)
    {
        bool doubleclick = false;
        // Double click
        if (widgetName == pLookUpSelectList->GetName())
        {
            doubleclick = true;
        }  // Double click

        // OK Button
        if (widgetName == pLookUpOkBtn->GetName() || doubleclick)
        {
            if (pLookUpSelectList->GetSelection() != -1)
            {
                if (noDisplayCols == 1)
                    wxStrcpy (ListDB_Selection, pLookUpSelectList->GetStringSelection());
                else  // 2 display columns
                {
                    wxString s = pLookUpSelectList->GetStringSelection();
                    // Column 1
                    s = s.SubString(0, col1Len-1);
                    s = s.Strip();
                    wxStrcpy(ListDB_Selection, s);
                    // Column 2
                    s = pLookUpSelectList->GetStringSelection();
                    s = s.Mid(col1Len + LISTDB_NO_SPACES_BETWEEN_COLS);
                    s = s.Strip();
                    wxStrcpy(ListDB_Selection2, s);
                }
            }
            else
            {
                wxStrcpy(ListDB_Selection,wxT(""));
                wxStrcpy(ListDB_Selection2,wxT(""));
            }
            Close();
        }  // OK Button

        // Cancel Button
        if (widgetName == pLookUpCancelBtn->GetName())
        {
            wxStrcpy (ListDB_Selection,wxT(""));
            wxStrcpy (ListDB_Selection2,wxT(""));
            Close();
        }  // Cancel Button

    }

}  // ClookUpDlg::OnCommand

// *********************************** listdb.cpp **********************************
