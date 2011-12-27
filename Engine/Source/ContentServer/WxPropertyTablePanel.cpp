/**************************************************************************
*
* File:		WxPropertyTablePanel.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Property table panel.
*		
*
*
* 
**************************************************************************/

#include "WxPropertyTablePanel.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
WxPropertyTablePanel::WxPropertyTablePanel( wxWindow* pParent ):
	wxPanel( pParent )
{
	wxBoxSizer* pSizer = new wxBoxSizer( wxHORIZONTAL );
	pPropertyGrid_ = new wxPropertyGrid( this );

	CsPropertyTable PropertyTable;
	ScnFont::StaticPropertyTable( PropertyTable );

	for( BcU32 CatIdx = 0; CatIdx < PropertyTable.getCatagoryCount(); ++CatIdx )
	{
		const CsPropertyCatagory& PropertyCatagory = PropertyTable.getCatagory( CatIdx );

		pPropertyGrid_->AppendCategory( PropertyCatagory.getName(), PropertyCatagory.getName() );

		for( BcU32 FieldIdx = 0; FieldIdx < PropertyTable.getFieldCount(); ++FieldIdx )
		{
			const CsPropertyField& PropertyField = PropertyTable.getField( FieldIdx );

			if( PropertyField.getCatagoryIdx() == CatIdx )
			{
				wxString Name = PropertyField.getName();
				switch( PropertyField.getValueType() )
				{
				case csPVT_NULL:
					{
					}
					break;
				case csPVT_BOOL:
					{
						wxBoolProperty* pProperty = new wxBoolProperty( Name, Name, false );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_UINT:
					{
						wxUIntProperty* pProperty = new wxUIntProperty( Name, Name, 0 );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_INT:
					{
						wxIntProperty* pProperty = new wxIntProperty( Name, Name, 0 );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_REAL:
					{
						wxFloatProperty* pProperty = new wxFloatProperty( Name, Name, 0.0 );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_STRING:
					{
						wxStringProperty* pProperty = new wxStringProperty( Name, Name, wxEmptyString );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_ENUM:
					{
						wxEnumProperty* pProperty = new wxEnumProperty( Name, Name );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_FILE:
					{
						wxFileProperty* pProperty = new wxFileProperty( Name, Name, wxEmptyString );
						pPropertyGrid_->Append( pProperty );
					}
					break;
				case csPVT_RESOURCE:
					{
					}
					break;
				}
			}
		}
	}


	pSizer->Add( pPropertyGrid_, 5, wxEXPAND  );
	
	// Layout.
	SetSizer( pSizer );
	Layout();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
WxPropertyTablePanel::~WxPropertyTablePanel()
{
	
}
