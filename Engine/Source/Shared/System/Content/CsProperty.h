/**************************************************************************
*
* File:		CsProperty.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSPROPERTY_H__
#define __CSPROPERTY_H__

#include "CsTypes.h"

//////////////////////////////////////////////////////////////////////////
// CsPropertyValueType
enum CsPropertyValueType
{
	csPVT_NULL = 0,			// No type.
	csPVT_BOOL,				// Boolean.
	csPVT_UINT,				// Unsigned integer.
	csPVT_INT,				// Signed integer.
	csPVT_FLOAT,			// Floating point.
	csPVT_STRING,			// String.
	csPVT_ENUM,				// Enum.
	csPVT_FILE,				// File.
	csPVT_RESOURCE,			// Resource.
};

//////////////////////////////////////////////////////////////////////////
// CsPropertyValueType
enum CsPropertyContainerType
{
	csPCT_NULL,				// No container.
	csPCT_VALUE,			// Single value.
	csPCT_LIST,				// List of values.
	csPCT_MAP				// Map of values.
};

//////////////////////////////////////////////////////////////////////////
// CsPropertyField
class CsPropertyField
{
public:
	/**
	 * Constructor.
	 */
	CsPropertyField();
	 
	/**
	 * Construct a field.
	 */
	CsPropertyField( const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType );
	
	/**
	 * Copy constructor.
	 */
	CsPropertyField( const CsPropertyField& Other );
	
	/**
	 * Destructor.
	 */
	~CsPropertyField();

	/**
	 * Get name.
	 */
	const std::string& getName() const;
	
	/**
	 * Get value type.
	 */
	CsPropertyValueType getValueType() const;
	
	/**
	 * Get container type.
	 */
	CsPropertyContainerType getContainerType() const;
	
private:
	std::string					Name_;				// Name of property.
	CsPropertyValueType			ValueType_;			// Value type.
	CsPropertyContainerType		ContainerType_;		// Container type.			
};

//////////////////////////////////////////////////////////////////////////
// Typedefs.
typedef std::vector< CsPropertyField > CsPropertyFieldList;
typedef CsPropertyFieldList::iterator CsPropertyFieldListIterator;

//////////////////////////////////////////////////////////////////////////
// CsPropertyTable
class CsPropertyTable
{
public:
	CsPropertyTable();
	~CsPropertyTable();
	
	/**
	 * Begin.
	 */
	CsPropertyTable& begin();
	
	/**
	 * End.
	 */
	void end();
	
	/**
	 * Field.
	 */
	CsPropertyTable& field( const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType );
	
	/**
	 * Get number of fields.
	 */
	BcU32 fieldCount() const;
	
	/**
	 * Get field.
	 */
	const CsPropertyField& getField( BcU32 FieldIdx ) const;
	
private:
	CsPropertyFieldList			FieldList_;			// Set of fields.

};

#endif
