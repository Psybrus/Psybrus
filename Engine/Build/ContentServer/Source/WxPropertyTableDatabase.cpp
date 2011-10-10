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

#include "WxPropertyTableDatabase.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
WxPropertyTableDatabase::WxPropertyTableDatabase()
{
	// Get property table. TEMP TEMP TEMP!
	CsCore::pImpl()->getResourcePropertyTable( "ScnFont", PropertyTable_ );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
WxPropertyTableDatabase::~WxPropertyTableDatabase()
{

}

//////////////////////////////////////////////////////////////////////////
// GetNumberRows
//virtual
int WxPropertyTableDatabase::GetNumberRows()
{
	return (int)PropertyTable_.fieldCount();
}

//////////////////////////////////////////////////////////////////////////
// GetNumberCols
//virtual
int WxPropertyTableDatabase::GetNumberCols()
{
	return 2;
}

//////////////////////////////////////////////////////////////////////////
// IsEmptyCell
//virtual
bool WxPropertyTableDatabase::IsEmptyCell( int row, int col )
{
	if( row < (int)PropertyTable_.fieldCount() && col < 2 )
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// GetValue
//virtual
wxString WxPropertyTableDatabase::GetValue( int Row, int Col )
{
	if( Row < (int)PropertyTable_.fieldCount() )
	{
		switch( Col )
		{
		case 0:
			return wxString( PropertyTable_.getField( Row ).getName().c_str() );
			break;
		case 1:
			return wxString( "#VALUE" );
			break;
		default:
			return wxEmptyString;
			break;
		}
	}

	return wxEmptyString;
}

//////////////////////////////////////////////////////////////////////////
// SetValue
//virtual
void WxPropertyTableDatabase::SetValue( int row, int col, const wxString& value )
{

}
