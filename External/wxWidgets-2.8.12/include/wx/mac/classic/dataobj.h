///////////////////////////////////////////////////////////////////////////////
// Name:        mac/dataobj.h
// Purpose:     declaration of the wxDataObject
// Author:      Stefan Csomor (adapted from Robert Roebling's gtk port)
// Modified by:
// Created:     10/21/99
// RCS-ID:      $Id: dataobj.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) 1998, 1999 Vadim Zeitlin, Robert Roebling
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MAC_DATAOBJ_H_
#define _WX_MAC_DATAOBJ_H_

// ----------------------------------------------------------------------------
// wxDataObject is the same as wxDataObjectBase under wxGTK
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxDataObject : public wxDataObjectBase
{
public:
    wxDataObject();
#ifdef __DARWIN__
    virtual ~wxDataObject() { }
#endif

    virtual bool IsSupportedFormat( const wxDataFormat& format, Direction dir = Get ) const;
};

#endif // _WX_MAC_DATAOBJ_H_

