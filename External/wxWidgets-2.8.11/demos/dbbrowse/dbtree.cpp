//----------------------------------------------------------------------------------------
// Name:        DBTree.cpp/.h
// Purpose:     Tree with Table and Views, branches show Field information
// Author:      Mark Johnson
// Modified by:
// Created:     19991129
// RCS-ID:      $Id: dbtree.cpp 44499 2007-02-13 23:22:24Z VZ $
// Copyright:   (c) Mark Johnson, Berlin Germany
// Licence:     wxWindows license
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
#ifndef __WXMSW__
#endif
//----------------------------------------------------------------------------------------
//-- all #includes that every .cpp needs             --- 19990807.mj10777 ----------------
//----------------------------------------------------------------------------------------
#include "std.h"    // sorgsam Pflegen !

//----------------------------------------------------------------------------------------
//-- Global functions --------------------------------------------------------------------
//----------------------------------------------------------------------------------------
static inline const wxChar *bool2String(bool b)
{
    return b ? wxEmptyString : _T("not ");
}

//----------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(DBTree, wxTreeCtrl)
    EVT_MOTION (DBTree::OnMouseMove)
    EVT_TREE_SEL_CHANGED(TREE_CTRL_DB, DBTree::OnSelChanged)
    EVT_TREE_ITEM_RIGHT_CLICK(TREE_CTRL_DB,DBTree::OnRightSelect)
    EVT_MENU(DATA_SHOW,DBTree::OnDBGrid)
    EVT_MENU(DATA_DB,DBTree::OnDBClass)
    EVT_MENU(DATA_TABLE,DBTree::OnTableClass)
    EVT_MENU(DATA_TABLE_ALL,DBTree::OnTableClassAll)
END_EVENT_TABLE()
//----------------------------------------------------------------------------------------
// DBTree implementation
//----------------------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(DBTree, wxTreeCtrl)

//----------------------------------------------------------------------------------------
DBTree::DBTree(wxWindow *parent)  : wxTreeCtrl(parent)
{
}

//----------------------------------------------------------------------------------------
DBTree::DBTree(wxWindow *parent, const wxWindowID id,const wxPoint& pos, const wxSize& size, long style)
: wxTreeCtrl(parent, id, pos, size, style)
{
    const int image_size = 16;
    // Make an image list containing small icons
    p_imageListNormal = new wxImageList(image_size, image_size, true);
    // should correspond to TreeIc_xxx enum
#if !defined(__WXMSW__)
#include "bitmaps/logo.xpm"
#include "bitmaps/dsnclose.xpm"
#include "bitmaps/dsnopen.xpm"
#include "bitmaps/tab.xpm"
#include "bitmaps/view.xpm"
#include "bitmaps/col.xpm"
#include "bitmaps/key.xpm"
#include "bitmaps/keyf.xpm"
#include "bitmaps/d_open.xpm"
#include "bitmaps/d_closed.xpm"
#endif
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(aLogo)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(DsnClosed)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(DsnOpen)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(TAB)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(VIEW)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(COL)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(KEY)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(KEYF)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(DocOpen)).ConvertToImage().Rescale(image_size, image_size)));
    p_imageListNormal->Add(wxBitmap(wxBitmap(wxICON(DocOpen)).ConvertToImage().Rescale(image_size, image_size)));
    SetImageList(p_imageListNormal);
    ct_BrowserDB = NULL;
    popupMenu1   = NULL;
    popupMenu2   = NULL;
}

//----------------------------------------------------------------------------------------
DBTree::~DBTree()
{
    delete ct_BrowserDB;

    //  delete (pDoc->db_Br+i_Which);
    // wxLogMessage("DBTree::~DBTree() - Vor  OnCloseDB()");
    (pDoc->db_Br+i_Which)->OnCloseDB(false);
    // wxLogMessage("DBTree::~DBTree() - Nach OnCloseDB()");
    (pDoc->db_Br+i_Which)->db_BrowserDB = NULL;
    (pDoc->db_Br+i_Which)->ct_BrowserDB = NULL;
    (pDoc->db_Br+i_Which)->cl_BrowserDB = NULL;

    delete p_imageListNormal;
    if (popupMenu1)       // If the DSN has no Tables, then no delete should be done !
        delete popupMenu1;
    if (popupMenu1)       // If the DSN has no Tables, then no delete should be done !
        delete popupMenu2;
}

//----------------------------------------------------------------------------------------
#undef TREE_EVENT_HANDLER

//----------------------------------------------------------------------------------------
int DBTree::OnPopulate()
{
    wxTreeItemId Root, Folder, Docu, Funkt;
    int i,x,y,z=0, TableType;
    wxString SQL_TYPE, DB_TYPE;
    SetFont(* pDoc->ft_Doc);
    //---------------------------------------------------------------------------------------
    if ((pDoc->db_Br+i_Which)->Initialize(false))
    {
        wxStopWatch sw;
        wxBeginBusyCursor();
        ct_BrowserDB = (pDoc->db_Br+i_Which)->OnGetCatalog(false);
        if (ct_BrowserDB)
        { // Use the wxDatabase Information
            Temp0.Printf(_T("%s - (%s) (%s)"), s_DSN.c_str(),ct_BrowserDB->catalog, ct_BrowserDB->schema);
            Root = AddRoot(Temp0,TreeIc_DsnOpen,TreeIc_DsnOpen,new DBTreeData(_T("Root")));
            for (x=0;x<ct_BrowserDB->numTables;x++)
            {
                wxYield();
                TableType = 0; // TABLE = 1 ; VIEW = 2 ; 0 We are not interested in
                if (!wxStrcmp((ct_BrowserDB->pTableInf+x)->tableType,_T("TABLE")))    // only TABLES
                    TableType = 1;
                if (!wxStrcmp((ct_BrowserDB->pTableInf+x)->tableType,_T("VIEW")))     // and  VIEWS
                    TableType = 2;
                if (TableType)    // only TABLES or Views
                {
                    Temp1.Printf(_T("TN(%s"),(ct_BrowserDB->pTableInf+x)->tableName);
                    //----
                    (ct_BrowserDB->pTableInf+x)->pColInf = (pDoc->db_Br+i_Which)->OnGetColumns((ct_BrowserDB->pTableInf+x)->tableName,(ct_BrowserDB->pTableInf+x)->numCols,false);
                    //----
                    if ((ct_BrowserDB->pTableInf+x)->pColInf)
                    {
                        if (TableType == 1)  // Table
                        {
                            Temp0.Printf(_("Table-Name(%s) with (%d)Columns ; Remarks(%s)"),  (ct_BrowserDB->pTableInf+x)->tableName,
                                (ct_BrowserDB->pTableInf+x)->numCols,(ct_BrowserDB->pTableInf+x)->tableRemarks);
                            Folder = AppendItem(Root,Temp0,TreeIc_TAB,TreeIc_TAB, new DBTreeData(Temp1));
                        }
                        if (TableType == 2)  // View
                        {
                            Temp0.Printf(_("View-Name(%s) with (%d)Columns ; Remarks(%s)"),  (ct_BrowserDB->pTableInf+x)->tableName,
                                (ct_BrowserDB->pTableInf+x)->numCols,(ct_BrowserDB->pTableInf+x)->tableRemarks);
                            Folder = AppendItem(Root,Temp0,TreeIc_VIEW,TreeIc_VIEW, new DBTreeData(Temp1));
                        }
                        for (y=0;y<(ct_BrowserDB->pTableInf+x)->numCols;y++)
                        {
                            Temp1.Printf(_T("FN(%s"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->colName);
                            // Here is where we find out if the Column is a Primary / Foreign Key
                            if (((ct_BrowserDB->pTableInf+x)->pColInf+y)->PkCol != 0)  // Primary Key
                            {
                                Temp2.Printf(_T("(%d) - %s"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->PkCol,((ct_BrowserDB->pTableInf+x)->pColInf+y)->colName);
                                Docu = AppendItem(Folder,Temp2,TreeIc_KEY,TreeIc_KEY,new DBTreeData(Temp1));
                                Temp2 = ((ct_BrowserDB->pTableInf+x)->pColInf+y)->PkTableName;
                                if (Temp2.empty())
                                    Temp2 = _("None");
                                Temp2.Printf(_("This Primary Key is used in the following Tables : %s"),Temp2.c_str());
                                Funkt = AppendItem(Docu,Temp2,TreeIc_DocClosed,TreeIc_DocOpen,new DBTreeData(_T("KEY")));
                            }
                            else
                            {
                                if (((ct_BrowserDB->pTableInf+x)->pColInf+y)->FkCol != 0) // Foreign Key
                                {
                                    Temp2.Printf(_T("(%d) - %s"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->FkCol,((ct_BrowserDB->pTableInf+x)->pColInf+y)->colName);
                                    Docu = AppendItem(Folder,Temp2,TreeIc_KEYF,TreeIc_KEYF,new DBTreeData(Temp1));
                                    Temp2.Printf(_("This Foreign Key comes from the following Table : %s"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->FkTableName);
                                    Funkt = AppendItem(Docu,Temp2,TreeIc_DocClosed,TreeIc_DocOpen,new DBTreeData(_T("KEYF")));
                                }
                                else
                                    Docu = AppendItem(Folder,((ct_BrowserDB->pTableInf+x)->pColInf+y)->colName,TreeIc_COL,TreeIc_COL,new DBTreeData(Temp1));
                            }
                            SQL_TYPE.Printf(_T("SQL_C_???? (%d)"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->sqlDataType);
                            DB_TYPE.Printf(_T("DB_DATA_TYPE_???? (%d)"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->dbDataType);
                            for (i=1;i<=(pDoc->db_Br+i_Which)->i_SqlTyp[0];i++)
                            {
                                if (((ct_BrowserDB->pTableInf+x)->pColInf+y)->sqlDataType == (pDoc->db_Br+i_Which)->i_SqlTyp[i])
                                {
                                    SQL_TYPE.Printf(_T("%s(%d) ; "),(pDoc->db_Br+i_Which)->s_SqlTyp[i].c_str(),(pDoc->db_Br+i_Which)->i_SqlTyp[i]);
                                }
                            } // for (i=1;i<=i_SqlTyp[0];i++)
                            wxYield();
                            for (i=1;i<=(pDoc->db_Br+i_Which)->i_dbTyp[0];i++)
                            {
                                if (((ct_BrowserDB->pTableInf+x)->pColInf+y)->dbDataType == (pDoc->db_Br+i_Which)->i_dbTyp[i])
                                {
                                    DB_TYPE.Printf(_T("%s(%d)"),(pDoc->db_Br+i_Which)->s_dbTyp[i].c_str(),(pDoc->db_Br+i_Which)->i_dbTyp[i]);
                                }
                            } // for (i=1;i<=i_dbTyp[0];i++)
                            wxYield();
                            SQL_TYPE += DB_TYPE;
                            Funkt = AppendItem(Docu,SQL_TYPE,TreeIc_DocClosed,TreeIc_DocOpen,new DBTreeData(SQL_TYPE));
                            SQL_TYPE.Printf(_T("%10s %d,%d"),((ct_BrowserDB->pTableInf+x)->pColInf+y)->typeName,
                                ((ct_BrowserDB->pTableInf+x)->pColInf+y)->columnLength,((ct_BrowserDB->pTableInf+x)->pColInf+y)->decimalDigits);
                            Funkt = AppendItem(Docu,SQL_TYPE,TreeIc_DocClosed,TreeIc_DocOpen,new DBTreeData(SQL_TYPE));
                        }  // for (y=0;y<(ct_BrowserDB->pTableInf+x)->numCols;y++)
                    }   // if ((ct_BrowserDB->pTableInf+x)->pColInf)
                    else
                        Folder = AppendItem(Root,Temp0,TreeIc_FolderClosed,TreeIc_FolderOpen, new DBTreeData(Temp1));
                    z++;
//                    if (z % 10 == 0)
                    {
#if wxUSE_STATUSBAR
                        Temp0.Printf(_("-I-> DBTree::OnPopulate(%s) - Table %6d (from %d) has been read."),(ct_BrowserDB->pTableInf+x)->tableName,z,ct_BrowserDB->numTables);
                        pDoc->p_MainFrame->SetStatusText(Temp0, 0);
#endif // wxUSE_STATUSBAR
                    }
                    wxYield();
                }    // if ((ct_BrowserDB->pTableInf+x)->tableType == "TABLE" or VIEW)
                // else
                //  wxLogMessage(_("\n-I-> if ! TABLE or VIEW  >%s<"),(ct_BrowserDB->pTableInf+x)->tableType);
            }     // for (x=0;x<ct_BrowserDB->numTables;x++)
        }      // if (ct_BrowserDB)
        else
            wxLogMessage(_("\n-E-> DBTree::OnPopulate() : Invalid Catalog Pointer : Failed"));
        wxEndBusyCursor();
        Temp0.Printf(_("-I-> DBTree::OnPopulate() - %6d Tables have been read. - Time needed : %ld ms"),z,sw.Time());
        wxLogMessage(Temp0);
#if wxUSE_STATUSBAR
        pDoc->p_MainFrame->SetStatusText(Temp0, 0);
#endif // wxUSE_STATUSBAR
    }       // if((pDoc->db_Br+i_Which)->Initialize(false))
    else
    {
        wxLogMessage(_("\n-E-> DBTree::OnPopulate() : A valid Pointer could not be created : Failed"));
        return 0;
    }
    //---------------------------------------------------------------------------------------
    Expand(Root);
    //---------------------------------------------------------------------------------------
    popupMenu1 = NULL;
    popupMenu1 = new wxMenu;
    popupMenu1->Append(DATA_DB, _("Make wxDB.cpp/h "));
    popupMenu1->AppendSeparator();
    popupMenu1->Append(DATA_TABLE_ALL, _("Make all wxTable.cpp/h classes"));
    popupMenu2 = NULL;
    popupMenu2 = new wxMenu;
    popupMenu2->Append(DATA_SHOW, _("Show Data"));
    popupMenu2->AppendSeparator();
    popupMenu2->Append(DATA_TABLE, _("Make wxTable.cpp/h "));
    //---------------------------------------------------------------------------------------
    return 0;
}  // DBTree::OnPopulate()

//----------------------------------------------------------------------------------------
void DBTree::OnSelChanged(wxTreeEvent& WXUNUSED(event))
{
    int i;
    Temp0.Empty();
#if wxUSE_STATUSBAR
    pDoc->p_MainFrame->SetStatusText(Temp0,0);
#endif // wxUSE_STATUSBAR
    // Get the Information that we need
    wxTreeItemId itemId = GetSelection();
    DBTreeData *item = (DBTreeData *)GetItemData(itemId);
    if ( item != NULL )
    {
        int Treffer = 0;
        Temp1.Printf(_T("%s"),item->m_desc.c_str());
        //-------------------------------------------------------------------------------------
        if (Temp1.Contains(_T("ODBC-")))
        {
            Temp1 = Temp1.Mid(5);
            for (i=0;i<pDoc->i_DSN;i++)
            {
                if (Temp1 == (pDoc->p_DSN+i)->Dsn)
                {
                    // pDoc->OnChosenDSN(i);
                }
            }
            Treffer++;
        }
        //--------------------------------------------------------------------------------------
        if (Treffer == 0)
        {
            //-------------------------------------------------------------------------------------
            /*
            Temp0.Printf(_("Item '%s': %sselected, %sexpanded, %sbold,"
            "%u children (%u immediately under this item)."),
            item->m_desc.c_str(),
            bool2String(IsSelected(itemId)),
            bool2String(IsExpanded(itemId)),
            bool2String(IsBold(itemId)),
            GetChildrenCount(itemId),
            GetChildrenCount(itemId));
            LogBuf.Printf("-I-> DBTree::OnSelChanged - %s",Temp0.c_str());
            wxLogMessage( "%s", LogBuf.c_str() );
            */
            //-------------------------------------------------------------------------------------
        }
    }
}

//----------------------------------------------------------------------------------------
void DBTree::OnRightSelect(wxTreeEvent& WXUNUSED(event))
{
    int i;
    Temp0.Empty();
    // Get the Information that we need
    wxTreeItemId itemId = GetSelection();
    DBTreeData *item = (DBTreeData *)GetItemData(itemId);
    if ( item != NULL )
    {
        int Treffer = 0;
        Temp1.Printf(_T("%s"),item->m_desc.c_str());
        //--------------------------------------------------------------------------------------
        if (!wxStrcmp(_T("Root"),Temp1))
        {
            PopupMenu(popupMenu1,TreePos.x,TreePos.y);
            Treffer++;
        }
        for (i=0;i<ct_BrowserDB->numTables;i++)
        {
            Temp2.Printf(_T("TN(%s"),(ct_BrowserDB->pTableInf+i)->tableName);
            if (!wxStrcmp(Temp2,Temp1))
            {
                PopupMenu(popupMenu2,TreePos.x,TreePos.y);
                Treffer++;
            }
        }
        //--------------------------------------------------------------------------------------
        if (Treffer == 0)
        {
            //-------------------------------------------------------------------------------------
            /*
            Temp0.Printf(_("Item '%s': %sselected, %sexpanded, %sbold,"
            "%u children (%u immediately under this item)."),
            item->m_desc.c_str(),
            bool2String(IsSelected(itemId)),
            bool2String(IsExpanded(itemId)),
            bool2String(IsBold(itemId)),
            GetChildrenCount(itemId),
            GetChildrenCount(itemId));
            LogBuf.Printf("-I-> DBTree::OnSelChanged - %s",Temp0.c_str());
            wxLogMessage( "%s", LogBuf.c_str() );
            */
            //-------------------------------------------------------------------------------------
        }
    }
}

//----------------------------------------------------------------------------------------
void DBTree::OnDBGrid(wxCommandEvent& WXUNUSED(event))
{
    int i;
    // Get the Information that we need
    wxTreeItemId itemId = GetSelection();
    DBTreeData *item = (DBTreeData *)GetItemData(itemId);
    if ( item != NULL )
    {
        Temp1.Printf(_T("%s"),item->m_desc.c_str());
        for (i=0;i<ct_BrowserDB->numTables;i++)
        {
            Temp2.Printf(_T("TN(%s"),(ct_BrowserDB->pTableInf+i)->tableName);
            if (!wxStrcmp(Temp2,Temp1))
            {
                // Temp0.Printf("(%d) Here is where a GridCtrl for >%s< will be called! ",i,(ct_BrowserDB->pTableInf+i)->tableName);
                pDoc->OnChosenTbl(1,(ct_BrowserDB->pTableInf+i)->tableName);
                // wxMessageBox(Temp0);
            }
        }
    }
}

//----------------------------------------------------------------------------------------
void DBTree::OnDBClass(wxCommandEvent& WXUNUSED(event))
{
    // int i;
    // Get the Information that we need
    wxTreeItemId itemId = GetSelection();
    DBTreeData *item = (DBTreeData *)GetItemData(itemId);
    if ( item != NULL )
    {
        Temp0.Printf(_("Here is where a wxDB Class for >%s< will be made! "),s_DSN.c_str());
        wxMessageBox(Temp0);
    }
}

//----------------------------------------------------------------------------------------
void DBTree::OnTableClass(wxCommandEvent& WXUNUSED(event))
{
    int i;
    // Get the Information that we need
    wxTreeItemId itemId = GetSelection();
    DBTreeData *item = (DBTreeData *)GetItemData(itemId);
    if ( item != NULL )
    {
        Temp1.Printf(_T("%s"),item->m_desc.c_str());
        for (i=0;i<ct_BrowserDB->numTables;i++)
        {
            Temp2.Printf(_T("TN(%s"),(ct_BrowserDB->pTableInf+i)->tableName);
            if (!wxStrcmp(Temp2,Temp1))
            {
                Temp0.Printf(_("(%d) Here is where a wxTable Class for >%s< will be made! "),i,(ct_BrowserDB->pTableInf+i)->tableName);
                wxMessageBox(Temp0);
            }
        }
    }
}

//----------------------------------------------------------------------------------------
void DBTree::OnTableClassAll(wxCommandEvent& WXUNUSED(event))
{
    // int i;
    // Get the Information that we need
    wxTreeItemId itemId = GetSelection();
    DBTreeData *item = (DBTreeData *)GetItemData(itemId);
    if ( item != NULL )
    {
        Temp0.Printf(_("Here is where all wxTable Classes in >%s< will be made! "),s_DSN.c_str());
        wxMessageBox(Temp0);
    }
}

//----------------------------------------------------------------------------------------
void DBTree::OnMouseMove(wxMouseEvent &event)
{
    TreePos = event.GetPosition();
}
//----------------------------------------------------------------------------------------
