/**************************************************************************
*
* File:		WxPropertyTableDatabase.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Property table database.
*		
*
*
* 
**************************************************************************/

#ifndef __WXPROPERTYTABLEDATABASE_H__
#define __WXPROPERTYTABLEDATABASE_H__

#include "Psybrus.h"

#include <wx/wx.h>
#include <wx/grid.h>

//////////////////////////////////////////////////////////////////////////
// WxPropertyTableDatabase
class WxPropertyTableDatabase:
	public wxGridTableBase
{
public:
	WxPropertyTableDatabase();
	~WxPropertyTableDatabase();

	virtual int GetNumberRows();
	virtual int GetNumberCols();
	virtual bool IsEmptyCell( int row, int col );
	virtual wxString GetValue( int row, int col );
	virtual void SetValue( int row, int col, const wxString& value );
	
private:
	CsPropertyTable PropertyTable_;

};

#endif
