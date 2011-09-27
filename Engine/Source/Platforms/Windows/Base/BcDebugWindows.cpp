/**************************************************************************
*
* File:		BcDebug.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Debug stuff
*		
*
*
* 
**************************************************************************/

#include "BcDebug.h"

#include <windows.h>

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type )
{
	UINT MBType = MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONEXCLAMATION;

	switch( Type )
	{
		case bcMBT_OK:
			MBType |= MB_OK;
			break;
		case bcMBT_OKCANCEL:
			MBType |= MB_OKCANCEL;
			break;
		case bcMBT_YESNO:
			MBType |= MB_YESNO;
			break;
		case bcMBT_YESNOCANCEL:
			MBType |= MB_YESNOCANCEL;
			break;
	}

	// TODO: HWND!
	int RetVal = ::MessageBoxA( NULL, pMessage, pTitle, MBType );

	switch( RetVal )
	{
		case IDOK:
			return bcMBR_OK;
			break;
		case IDYES:
			return bcMBR_YES;
			break;
		case IDNO:
			return bcMBR_NO;
			break;
		case IDCANCEL:
			return bcMBR_CANCEL;
			break;
		default:
			break;
	};

	return bcMBR_OK;
}
