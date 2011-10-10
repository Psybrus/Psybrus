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
// CsPropertyCatagory
class CsPropertyCatagory
{
public:
	/**
	 * Constructor.
	 */
	CsPropertyCatagory();

	/**
	 * Construct a catagory.
	 */
	CsPropertyCatagory( const std::string& Name );

	/**
	 * Copy constructor.
	 */
	CsPropertyCatagory( const CsPropertyCatagory& Other );

	/**
	 * Destructor.
	 */
	~CsPropertyCatagory();

	/**
	 * Get name.
	 */
	const std::string&			getName() const;

private:
	std::string					Name_;
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
	CsPropertyField( BcU32 CatagoryIdx, const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType );
	
	/**
	 * Copy constructor.
	 */
	CsPropertyField( const CsPropertyField& Other );
	
	/**
	 * Destructor.
	 */
	~CsPropertyField();

	/**
	 * Get catagory index.
	 */
	BcU32						getCatagoryIdx() const;

	/**
	 * Get name.
	 */
	const std::string&			getName() const;
	
	/**
	 * Get value type.
	 */
	CsPropertyValueType			getValueType() const;
	
	/**
	 * Get container type.
	 */
	CsPropertyContainerType		getContainerType() const;
	
private:
	BcU32						CatagoryIdx_;		// Catagory of property.
	std::string					Name_;				// Name of property.
	CsPropertyValueType			ValueType_;			// Value type.
	CsPropertyContainerType		ContainerType_;		// Container type.			
};

//////////////////////////////////////////////////////////////////////////
// Typedefs.
typedef std::vector< CsPropertyField > CsPropertyFieldList;
typedef CsPropertyFieldList::iterator CsPropertyFieldListIterator;
typedef std::vector< CsPropertyCatagory > CsPropertyCatagoryList;
typedef CsPropertyCatagoryList::iterator CsPropertyCatagoryListIterator;

//////////////////////////////////////////////////////////////////////////
// CsPropertyTable
class CsPropertyTable
{
public:
	CsPropertyTable();
	~CsPropertyTable();
	
	/**
	 * Begin catagory.
	 */
	CsPropertyTable& beginCatagory( const std::string& Catagory );
	
	/**
	 * End catagory.
	 */
	CsPropertyTable& endCatagory();
	
	/**
	 * Field.
	 */
	CsPropertyTable& field( const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType );
	
	/**
	 * Get number of catagories.
	 */
	BcU32 getCatagoryCount() const;

	/**
	 * Get catagory.
	 */
	const CsPropertyCatagory& getCatagory( BcU32 CatagoryIdx ) const;

	/**
	 * Get number of fields.
	 */
	BcU32 getFieldCount() const;
	
	/**
	 * Get field.
	 */
	const CsPropertyField& getField( BcU32 FieldIdx ) const;
	
private:
	BcBool						BeganCatagory_;

	CsPropertyCatagoryList		CatagoryList_;		// List of catagories.
	CsPropertyFieldList			FieldList_;			// List of fields.

};

#endif
