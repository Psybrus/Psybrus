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

	BcName ResourceType( "ScnMaterial" );

	wxPropertyCategory* pProperty = new wxPropertyCategory( "Resource" );
	pPropertyGrid_->Append( pProperty );
	CsPropertyTable NewPropertyTable;
	CsCore::pImpl()->getResourcePropertyTable( ResourceType, NewPropertyTable );
	setupPropertyTable( pProperty, NewPropertyTable );

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

//////////////////////////////////////////////////////////////////////////
// setupPropertyTable
void WxPropertyTablePanel::setupPropertyTable( wxPGProperty* pParent, const CsPropertyTable& PropertyTable )
{
	for( BcU32 CatIdx = 0; CatIdx < PropertyTable.getCatagoryCount(); ++CatIdx )
	{
		const CsPropertyCatagory& PropertyCatagory = PropertyTable.getCatagory( CatIdx );

		wxPropertyCategory* pCatagory = new wxPropertyCategory( PropertyCatagory.getName(), PropertyCatagory.getName() );
		pParent->AppendChild( pCatagory );

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
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_UINT:
					{
						wxUIntProperty* pProperty = new wxUIntProperty( Name, Name, 0 );
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_INT:
					{
						wxIntProperty* pProperty = new wxIntProperty( Name, Name, 0 );
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_REAL:
					{
						wxFloatProperty* pProperty = new wxFloatProperty( Name, Name, 0.0 );
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_STRING:
					{
						wxStringProperty* pProperty = new wxStringProperty( Name, Name, wxEmptyString );
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_ENUM:
					{
						wxEnumProperty* pProperty = new wxEnumProperty( Name, Name );
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_FILE:
					{
						wxFileProperty* pProperty = new wxFileProperty( Name, Name, wxEmptyString );
						pCatagory->AppendChild( pProperty );
					}
					break;
				case csPVT_RESOURCE:
					{
						/*
						wxPropertyCategory* pProperty = new wxPropertyCategory( Name, Name );
						pCatagory->AppendChild( pProperty );

						CsPropertyTable NewPropertyTable;
						CsCore::pImpl()->getResourcePropertyTable( PropertyField.getAdditionalInfo(), NewPropertyTable );
						setupPropertyTable( pProperty, NewPropertyTable );
						*/
						wxStringProperty* pProperty = new wxStringProperty( Name, Name, wxEmptyString );
						pCatagory->AppendChild( pProperty );
					}
					break;
				}
			}
		}
	}
}
