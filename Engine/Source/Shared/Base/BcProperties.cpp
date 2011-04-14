/**************************************************************************
*
* File:		BcProperties.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*  
*
*
* 
**************************************************************************/

#include "BcProperties.h"

#if 0

class TestPropertyObject
{
	BCPROPERTIES_DECLARE();

public:
	BcReal 			TestReal_;
	BcU32 			TestInt_;
	std::string 	TestString_;
	BcMat4d			TestMat4d_;
	BcVec2d			TestVec2d_;
	BcVec3d			TestVec3d_;
	BcVec4d			TestVec4d_;
};

BCPROPERTIES_BEGIN( TestPropertyObject )
	BCPROPERTY_ENTRY( TestPropertyObject, "TestReal",		TestReal_ ),
	BCPROPERTY_ENTRY( TestPropertyObject, "TestInt",		TestInt_ ),
	BCPROPERTY_ENTRY( TestPropertyObject, "TestString",		TestString_ ),
	BCPROPERTY_ENTRY( TestPropertyObject, "TestMat4d",		TestMat4d_ ),
	BCPROPERTY_ENTRY( TestPropertyObject, "TestVec2d",		TestVec2d_ ),
	BCPROPERTY_ENTRY( TestPropertyObject, "TestVec3d",		TestVec3d_ ),
	BCPROPERTY_ENTRY( TestPropertyObject, "TestVec4d",		TestVec4d_ ),
BCPROPERTIES_END();

void DumpTestPropertyObject()
{
	BcProperty* pProperty = &TestPropertyObject::Properties_[0];
		
	while( pProperty->Type_ != bcPT_NULL )
	{
		BcPrintf( "Name: %s, Offset: %x, Type: %u\n", pProperty->pName_, pProperty->Offset_, pProperty->Type_ );	
		++pProperty;
	}	
}
#endif
