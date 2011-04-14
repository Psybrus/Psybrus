///////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/apptrait.h
// Purpose:     class implementing wxAppTraits for Palm OS
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id: apptrait.h 40599 2006-08-13 21:00:32Z VZ $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALM_APPTRAIT_H_
#define _WX_PALM_APPTRAIT_H_

// ----------------------------------------------------------------------------
// wxGUI/ConsoleAppTraits: must derive from wxAppTraits, not wxAppTraitsBase
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxConsoleAppTraits : public wxConsoleAppTraitsBase
{
public:
    virtual void *BeforeChildWaitLoop();
    virtual void AlwaysYield();
    virtual void AfterChildWaitLoop(void *data);

    virtual bool DoMessageFromThreadWait();
};

#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxGUIAppTraits : public wxGUIAppTraitsBase
{
public:
    virtual void *BeforeChildWaitLoop();
    virtual void AlwaysYield();
    virtual void AfterChildWaitLoop(void *data);

    virtual bool DoMessageFromThreadWait();
    virtual wxPortId GetToolkitVersion(int *majVer, int *minVer) const;
};

#endif // wxUSE_GUI

#endif // _WX_PALM_APPTRAIT_H_

