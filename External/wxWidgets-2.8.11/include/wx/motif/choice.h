/////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/choice.h
// Purpose:     wxChoice class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: choice.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CHOICE_H_
#define _WX_CHOICE_H_

#include "wx/clntdata.h"

#ifndef wxWIDGET_ARRAY_DEFINED
    #define wxWIDGET_ARRAY_DEFINED

    #include "wx/dynarray.h"
    WX_DEFINE_ARRAY_PTR(WXWidget, wxWidgetArray);
#endif

// Choice item
class WXDLLEXPORT wxChoice: public wxChoiceBase
{
    DECLARE_DYNAMIC_CLASS(wxChoice)

public:
    wxChoice();
    virtual ~wxChoice();

    wxChoice(wxWindow *parent, wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        int n = 0, const wxString choices[] = NULL,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxChoiceNameStr)
    {
        Init();
        Create(parent, id, pos, size, n, choices, style, validator, name);
    }

    wxChoice(wxWindow *parent, wxWindowID id,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxChoiceNameStr)
    {
        Init();
        Create(parent, id, pos, size, choices, style, validator, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        int n = 0, const wxString choices[] = NULL,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxChoiceNameStr);

    bool Create(wxWindow *parent, wxWindowID id,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxChoiceNameStr);

    // implementation of wxControlWithItems
    virtual unsigned int GetCount() const;
    virtual int DoAppend(const wxString& item);
    virtual int DoInsert(const wxString& item, unsigned int pos);
    virtual void DoSetItemClientData(unsigned int n, void* clientData);
    virtual void* DoGetItemClientData(unsigned int n) const;
    virtual void DoSetItemClientObject(unsigned int n, wxClientData* clientData);
    virtual wxClientData* DoGetItemClientObject(unsigned int n) const;
    virtual int GetSelection() const;
    virtual void Delete(unsigned int n);
    virtual void Clear();
    virtual void SetString(unsigned int n, const wxString& s);
    virtual wxString GetString(unsigned int n) const;

    // implementation of wxChoiceBase
    virtual void SetSelection(int n);
    virtual void SetColumns(int n = 1 );
    virtual int GetColumns() const ;

    // Original API
    virtual void Command(wxCommandEvent& event);

    void SetFocus();

    // Implementation
    virtual void ChangeFont(bool keepOriginalSize = true);
    virtual void ChangeBackgroundColour();
    virtual void ChangeForegroundColour();
    WXWidget GetTopWidget() const { return m_formWidget; }
    WXWidget GetMainWidget() const { return m_buttonWidget; }

    virtual wxSize DoGetBestSize() const;

    // implementation, for wxChoiceCallback
    const wxWidgetArray& GetWidgets() const { return m_widgetArray; }
    const wxStringList&  GetStrings() const { return m_stringList; }
protected:
    // minimum size for the text ctrl
    wxSize GetItemsSize() const;
    // common part of all contructors
    void Init();

    unsigned int  m_noStrings;
    WXWidget      m_menuWidget;
    WXWidget      m_buttonWidget;
    wxWidgetArray m_widgetArray;
    WXWidget      m_formWidget;
    wxStringList  m_stringList;
    wxClientDataDictionary m_clientDataDict;

    virtual void DoSetSize(int x, int y,
        int width, int height,
        int sizeFlags = wxSIZE_AUTO);
};

#endif
// _WX_CHOICE_H_
