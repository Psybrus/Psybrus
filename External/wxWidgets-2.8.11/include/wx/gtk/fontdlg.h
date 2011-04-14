/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/fontdlgg.h
// Purpose:     wxFontDialog
// Author:      Robert Roebling
// Created:
// RCS-ID:      $Id: fontdlg.h 38448 2006-03-30 14:04:17Z ABX $
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GTK_FONTDLGH__
#define __GTK_FONTDLGH__

//-----------------------------------------------------------------------------
// wxFontDialog
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFontDialog : public wxFontDialogBase
{
public:
    wxFontDialog() : wxFontDialogBase() { /* must be Create()d later */ }
    wxFontDialog(wxWindow *parent)
        : wxFontDialogBase(parent) { Create(parent); }
    wxFontDialog(wxWindow *parent, const wxFontData& data)
        : wxFontDialogBase(parent, data) { Create(parent, data); }

    virtual ~wxFontDialog();

    // implementation only
    void SetChosenFont(const char *name);

#if WXWIN_COMPATIBILITY_2_6
    // deprecated interface, don't use
    wxDEPRECATED( wxFontDialog(wxWindow *parent, const wxFontData *data) );
#endif // WXWIN_COMPATIBILITY_2_6

protected:
    // create the GTK dialog
    virtual bool DoCreate(wxWindow *parent);

private:
    DECLARE_DYNAMIC_CLASS(wxFontDialog)
};

#if WXWIN_COMPATIBILITY_2_6
    // deprecated interface, don't use
inline wxFontDialog::wxFontDialog(wxWindow *parent, const wxFontData *data)
        : wxFontDialogBase(parent) { InitFontData(data); Create(parent); }
#endif // WXWIN_COMPATIBILITY_2_6

#endif
