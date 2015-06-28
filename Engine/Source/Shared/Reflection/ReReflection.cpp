#include "Reflection/ReReflection.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Object copying.
class ObjectCopyContext
{
private:
	typedef std::vector< ReFieldAccessor > FieldAccessors;
	struct FieldCopyInfo
	{
		FieldCopyInfo():
			SrcData_( nullptr ),
			DstData_( nullptr ),
			SrcClass_( nullptr )
		{
		}

		void* SrcData_;
		void* DstData_;
		const ReClass* SrcClass_;
	};

	typedef std::vector< FieldCopyInfo > FieldCopyInfoList;
	typedef std::vector< void* > GatheredObjectList;
	
	GatheredObjectList GatheredObjectList_;
	FieldCopyInfoList FieldCopyInfoList_;

public:
	/**
		* @brief Get field copy info for source data pointer.
		* Will create new info if there is no info.
		*/
	FieldCopyInfo* getFieldCopyInfo( void* SrcData, void* DstData, const ReClass* InClass )
	{
		PSY_LOG( "getFieldCopyInfo: %s",
			(*InClass->getName()).c_str() );
		PSY_LOGSCOPEDINDENT;

		// Search for matching source data.
		auto Iter = std::find_if( FieldCopyInfoList_.begin(), FieldCopyInfoList_.end(), [ &SrcData ]( FieldCopyInfo& FieldCopyInfo )
		{
			return FieldCopyInfo.SrcData_ == SrcData;
		} );

		FieldCopyInfo* FoundFieldCopyInfo = nullptr;

		// If we don't find one, create a new copy + the destination object.
		if( Iter == FieldCopyInfoList_.end() )
		{
			PSY_LOG( "INFO`: not found match for %p", SrcData );
			PSY_LOGSCOPEDINDENT;
			if( InClass->getTypeSerialiser() )
			{
				PSY_LOG( "create new" );
				FieldCopyInfoList_.push_back( FieldCopyInfo() );
				FoundFieldCopyInfo = &FieldCopyInfoList_.back();
				FoundFieldCopyInfo->DstData_ = DstData == nullptr ? InClass->create< void >() : DstData;
				FoundFieldCopyInfo->SrcData_ = SrcData;
				FoundFieldCopyInfo->SrcClass_ = InClass;
			}
		}
		else
		{
			PSY_LOG( "SUCCESS: found match for %p", SrcData );
			FoundFieldCopyInfo = &(*Iter);
			BcAssert( FoundFieldCopyInfo->SrcClass_ == InClass );
		}

			
		return FoundFieldCopyInfo;
	}

	/**
		* @brief Gather from a pointer.
		*/
	void gatherFieldPointer( const ReFieldAccessor& SrcFieldAccessor )
	{
		BcAssert( SrcFieldAccessor.isPointerType() );

		void* Data = SrcFieldAccessor.getData();
		FieldCopyInfo* FoundFieldCopyInfo = getFieldCopyInfo( Data, nullptr, SrcFieldAccessor.getUpperClass() );

		// Recurse into object and gather fields.
		if( FoundFieldCopyInfo != nullptr )
		{
			gatherFields( SrcFieldAccessor.getData(), FoundFieldCopyInfo->SrcClass_ );
		}
	}

	/**
		* @brief Gather from a container.
		*/
	void gatherFieldContainer( const ReFieldAccessor& SrcFieldAccessor )
	{
		BcAssert( SrcFieldAccessor.isContainerType() );
		BcAssert( SrcFieldAccessor.isContainerOfPointerValues() || SrcFieldAccessor.isContainerOfPointerKeys() );

		auto SrcIter = SrcFieldAccessor.newReadIterator();
		while( SrcIter->isValid() )
		{
			if( SrcFieldAccessor.isContainerOfPointerKeys() )
			{
				void* Data = SrcIter->getKey();
				Data = *reinterpret_cast< void** >( Data );
				FieldCopyInfo* FoundFieldCopyInfo = getFieldCopyInfo( Data, nullptr, SrcFieldAccessor.getKeyUpperClass( Data ) );

				// Recurse into object and gather fields.
				if( FoundFieldCopyInfo != nullptr )
				{
					gatherFields( Data, FoundFieldCopyInfo->SrcClass_ );
				}
			}

			if( SrcFieldAccessor.isContainerOfPointerValues() )
			{
				void* Data = SrcIter->getValue();
				Data = *reinterpret_cast< void** >( Data );
				FieldCopyInfo* FoundFieldCopyInfo = getFieldCopyInfo( Data, nullptr, SrcFieldAccessor.getValueUpperClass( Data ) );

				// Recurse into object and gather fields.
				if( FoundFieldCopyInfo != nullptr )
				{
					gatherFields( Data, FoundFieldCopyInfo->SrcClass_ );
				}
			}

			SrcIter->next();
		}

		delete SrcIter;
	}

	/**
		* @brief Gather fields from an object.
		* Recurses downwards and gathers all field accessors for pointer types
		* so we can correctly copy duplicated pointers.
		*/
	void gatherFields( void* SrcObject, const ReClass* InClass )
	{
		// Check we've not gathered it.
		// TODO: Maybe need a faster method to do this? Could mark the objects...but requires
		//       all pointers to be to Object types. Fair restriction perhaps?
		if( std::find( GatheredObjectList_.begin(), GatheredObjectList_.end(), SrcObject ) == GatheredObjectList_.end() )
		{
			GatheredObjectList_.push_back( SrcObject );

			const ReClass* Class = InClass;
			while( Class != nullptr )
			{
				for( BcU32 Idx = 0; Idx < Class->getNoofFields(); ++Idx )
				{
					ReFieldAccessor SrcFieldAccessor( SrcObject, Class->getField( Idx ) );

					// Ignore null pointers, transients, and shallow copies.
					BcBool IsNullPtr = SrcFieldAccessor.isNullptr();
					BcBool IsTransient = SrcFieldAccessor.isTransient();
					BcBool IsShallowCopy = SrcFieldAccessor.isShallowCopy();

					if( !IsNullPtr && 
						!IsTransient &&
						!IsShallowCopy )
					{
						if( SrcFieldAccessor.isPointerType() )
						{
							gatherFieldPointer( SrcFieldAccessor );
						}
						else if( SrcFieldAccessor.isContainerType() && SrcFieldAccessor.isContainerOfPointerValues() )
						{
							gatherFieldContainer( SrcFieldAccessor );
						}
					}
				}

				Class = Class->getSuper();
			}
		}
	}

	/**
		* @brief Copy class data from one to another.
		* Will create new objects based on ones that were previously gathered.
		*/
	void copyClassData( void* DstObject, void* SrcObject, const ReClass* InClass, const ReField* InField )
	{
		PSY_LOG( "%p->%p",
					SrcObject, DstObject );
		PSY_LOGSCOPEDINDENT;

		// Grab basis.
		ReObject* Basis = InClass->hasBaseClass( ReObject::StaticGetClass() ) ?
			reinterpret_cast< ReObject* >( SrcObject ) : nullptr;

		// Trivial copy if we can:
		// - No fields in class.
		// - Not a pointer type.
		// - Class size must either be 0, or match exactly the field (do blind copy if not registered)
		if( InField != nullptr &&
			InClass->getNoofFields() == 0 &&
			( InField->getFlags() & bcRFF_POD ) != 0 &&
			( InField->getFlags() & bcRFF_ANY_POINTER_TYPE ) == 0 )
		{
			PSY_LOG( "copyClassData: %p->%p, %s (trivial POD)", 
				SrcObject, DstObject,
				(*InClass->getName()).c_str() );
			BcAssert( InClass->getSize() == 0 || InClass->getSize() == InField->getSize() );
			BcMemCopy( DstObject, SrcObject, InField->getSize() );
			return;
		}

		// Slow copy. Copy each field individually, we can validate here initially.
		// TODO: Perhaps do a memcpy, then mark up pointer/ref fields individually?
		// TODO: Do the copy in stages.
		//       - Recurse and gather pointer types.
		const ReClass* CopyingClass = InClass;
		while( CopyingClass != nullptr )
		{
			for( BcU32 Idx = 0; Idx < CopyingClass->getNoofFields(); ++Idx )
			{
				auto Field = CopyingClass->getField( Idx );
				ReFieldAccessor SrcFieldAccessor( SrcObject, Field );
					
				PSY_LOG( "copyClassData: Field %s::%s",
					(*Field->getType()->getName()).c_str(),
					(*Field->getName()).c_str() );
				PSY_LOGSCOPEDINDENT;

				// Only copy non-transient fields.
				if( !SrcFieldAccessor.isTransient() )
				{
					ReFieldAccessor DstFieldAccessor( DstObject, Field );

					// Is it a container?
					if( !Field->isContainer() )
					{
						const ReClass* FieldClass = SrcFieldAccessor.getUpperClass();

						// Only copy data if it's not a shallow copy.
						if( !SrcFieldAccessor.isNullptr() )
						{
							// Check in the field info if it's a pointer type, and set in the destination.
							if( Field->isPointerType() )
							{
								if( !SrcFieldAccessor.isShallowCopy() )
								{
									FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( SrcFieldAccessor.getData(), nullptr, SrcFieldAccessor.getUpperClass() );
									BcAssertMsg( FieldCopyInfo != nullptr, "No field info for %s %s::%s. Is the type registered?", 
										(*FieldClass->getName()).c_str(), 
										(*InClass->getName()).c_str(),
										(*Field->getName()).c_str() );
									DstFieldAccessor.setData( FieldCopyInfo->DstData_ );
								}
								else
								{
									DstFieldAccessor.setData( SrcFieldAccessor.getData() );
								}
							}

							// Only do copy if it's not a shallow copy.
							if( !SrcFieldAccessor.isShallowCopy() )
							{
								// Create a copy and recurse down.
								if( !DstFieldAccessor.copy( SrcFieldAccessor.getData() ) )
								{
									copyClassData( DstFieldAccessor.getData(), SrcFieldAccessor.getData(), FieldClass, Field );
								}
							}
						}
						// If basis, set.
						if( DstFieldAccessor.isBasis() )
						{
							DstFieldAccessor.setData( Basis );
						}						
					}
					else
					{
						// Container copying.
						auto DstIter = DstFieldAccessor.newWriteIterator();
						auto SrcIter = SrcFieldAccessor.newReadIterator();

						// TODO: Remove cast. Will eventually be no ReClass.
						const ReClass* KeyType = static_cast< const ReClass* >( Field->getKeyType() );
						const ReClass* ValueType = static_cast< const ReClass* > ( Field->getValueType() );

						if( KeyType == nullptr )
						{
							while( SrcIter->isValid() )
							{
								void* Value = SrcIter->getValue();

								// Check in the field info if it's a pointer type, and set in the destination.
								if( SrcFieldAccessor.isContainerOfPointerValues() &&
									!SrcFieldAccessor.isShallowCopy() )
								{
									Value = *reinterpret_cast< void** >( Value );
									ValueType = SrcFieldAccessor.getValueUpperClass( Value );
									FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( Value, nullptr, ValueType );
									if( !ValueType->getTypeSerialiser()->copy( FieldCopyInfo->DstData_, Value ) )
									{
										copyClassData( FieldCopyInfo->DstData_, Value, static_cast< const ReClass* >( ValueType ), Field );
									}
									else
									{
										PSY_LOG( "%p->%p", FieldCopyInfo->DstData_, Value );
									}
									Value = &FieldCopyInfo->DstData_;
								}

								DstIter->add( Value );
								SrcIter->next();
							}
						}
						else
						{
							while( SrcIter->isValid() )
							{
								auto Key = SrcIter->getKey();
								auto Value = SrcIter->getValue();

								// Check in the field info if it's a pointer type, and set in the destination.
								if( !SrcFieldAccessor.isShallowCopy() )
								{
									if( SrcFieldAccessor.isContainerOfPointerKeys() )
									{
										Key = *reinterpret_cast< void** >( Key );
										KeyType = SrcFieldAccessor.getKeyUpperClass( Key );
										FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( Key, nullptr, KeyType );
										BcAssert( FieldCopyInfo );
										if( !KeyType->getTypeSerialiser()->copy( FieldCopyInfo->DstData_, Value ) )
										{
											copyClassData( FieldCopyInfo->DstData_, Key, static_cast< const ReClass* >( KeyType ), Field );
										}
										Key = &FieldCopyInfo->DstData_;
									}
									if( SrcFieldAccessor.isContainerOfPointerValues() )
									{
										Value = *reinterpret_cast< void** >( Value );
										ValueType = SrcFieldAccessor.getValueUpperClass( Value );
										FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( Value, nullptr, ValueType );
										BcAssert( FieldCopyInfo );
										if( !ValueType->getTypeSerialiser()->copy( FieldCopyInfo->DstData_, Value ) )
										{
											copyClassData( FieldCopyInfo->DstData_, Value, static_cast< const ReClass* >( ValueType ), Field );
										}
										Value = &FieldCopyInfo->DstData_;
									}
								}

								DstIter->add( Key, Value );
								SrcIter->next();
							}
						}

						delete DstIter;
						delete SrcIter;
					}
				}
			}

			CopyingClass = CopyingClass->getSuper();
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CopyClass
void ReCopyClass( void* DstObject, void* SrcObject, const ReClass* InClass )
{
	BcAssert( DstObject != nullptr );
	BcAssert( SrcObject != nullptr );
	BcAssert( InClass != nullptr );

	PSY_LOGSCOPEDCATEGORY( "Reflection" );
	PSY_LOG( "ReCopyClass: \"%s\"", (*InClass->getName()).c_str() );
	PSY_LOGSCOPEDINDENT;

	// Now create all objects that exist as pointers in fields, and mark up.
	ObjectCopyContext ObjectCopyContext;
	auto* RootFieldCopyInfo = ObjectCopyContext.getFieldCopyInfo( SrcObject, DstObject, InClass );
	if( RootFieldCopyInfo != nullptr )
	{
		RootFieldCopyInfo->DstData_ = DstObject;
		ObjectCopyContext.gatherFields( SrcObject, InClass );
		ObjectCopyContext.copyClassData( DstObject, SrcObject, InClass, nullptr );
	}
}

//////////////////////////////////////////////////////////////////////////
// ConstructObject
ReObject* ReConstructObject( 
	const ReClass* InClass, 
	const std::string& InName, 
	ReObject* InOwner, 
	ReObject* InBasis,
	std::function< void( ReObject* ) > postCreateFunc )
{
	BcAssert( InClass != nullptr );

	PSY_LOGSCOPEDCATEGORY( "Reflection" );
	PSY_LOG( "ReConstructObject: \"%s\", name \"%s\", basis \"%s\"", 
		(*InClass->getName()).c_str(),
		InName.c_str(),
		InBasis ? (*InBasis->getName()).c_str(): "nullptr" );
	PSY_LOGSCOPEDINDENT;

	auto NewObject = InBasis == nullptr ? 
		InClass->create< ReObject >() : InClass->create< ReObject >(); // TODO: Reconsider createNoInit for the latter.

	// If we've succeeded...
	if( NewObject != nullptr )
	{
		// Call post create.
		if( postCreateFunc != nullptr )
		{
			postCreateFunc( NewObject );
		}

		// If we have a basis, we need to perform a deep copy.
		if( InBasis != nullptr )
		{
			ReCopyClass( NewObject, InBasis, NewObject->getClass() );
		}

		// Setup owner and basis.
		NewObject->Name_ = InName;
		NewObject->Owner_ = InOwner;
		NewObject->Basis_ = InBasis;
	}
			
	return NewObject;
}
