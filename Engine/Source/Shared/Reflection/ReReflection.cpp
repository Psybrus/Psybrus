#include "Reflection/ReReflection.h"

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
		// Search for matching source data.
		auto Iter = std::find_if( FieldCopyInfoList_.begin(), FieldCopyInfoList_.end(), [ &SrcData ]( FieldCopyInfo& FieldCopyInfo )
		{
			return FieldCopyInfo.SrcData_ == SrcData;
		} );

		FieldCopyInfo* FoundFieldCopyInfo = nullptr;

		// If we don't find one, create a new copy + the destination object.
		if( Iter == FieldCopyInfoList_.end() )
		{
			if( InClass->getTypeSerialiser() )
			{
				FieldCopyInfoList_.push_back( FieldCopyInfo() );
				FoundFieldCopyInfo = &FieldCopyInfoList_.back();
				FoundFieldCopyInfo->DstData_ = DstData == nullptr ? InClass->constructNoInit< void >() : DstData;
				FoundFieldCopyInfo->SrcData_ = SrcData;
				FoundFieldCopyInfo->SrcClass_ = InClass;
			}
		}
		else
		{
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
					if( !SrcFieldAccessor.isNullptr() && 
						!SrcFieldAccessor.isTransient() &&
						!SrcFieldAccessor.isShallowCopy() )
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
	void copyClassData( void* DstObject, void* SrcObject, const ReClass* InClass )
	{
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
				auto FieldType = Field->getType();
				ReFieldAccessor SrcFieldAccessor( SrcObject, Field );
					
				// Only copy non-transient fields.
				if( !SrcFieldAccessor.isTransient() && ! SrcFieldAccessor.isNullptr() )
				{
					ReFieldAccessor DstFieldAccessor( DstObject, Field );

					// Is it a container?
					if( !Field->isContainer() )
					{
						const ReClass* FieldClass = SrcFieldAccessor.getUpperClass();

						// Check in the field info if it's a pointer type, and set in the destination.
						if( Field->isPointerType() )
						{
							// Only copy data if it's not a shallow copy.
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
								copyClassData( DstFieldAccessor.getData(), SrcFieldAccessor.getData(), FieldClass );
							}
						}
					}
					else
					{
						// Container copying.
						auto DstIter = DstFieldAccessor.newWriteIterator();
						auto SrcIter = SrcFieldAccessor.newReadIterator();

						auto KeyType = Field->getKeyType();
						auto ValueType = Field->getValueType();

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
									FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( Value, nullptr, SrcFieldAccessor.getValueUpperClass( Value ) );
									if( !ValueType->getTypeSerialiser()->copy( FieldCopyInfo->DstData_, Value ) )
									{
										copyClassData( FieldCopyInfo->DstData_, Value, static_cast< const ReClass* >( ValueType ) );
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
										FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( Key, nullptr, SrcFieldAccessor.getKeyUpperClass( Key ) );
										BcAssert( FieldCopyInfo );
										if( !KeyType->getTypeSerialiser()->copy( FieldCopyInfo->DstData_, Value ) )
										{
											copyClassData( FieldCopyInfo->DstData_, Key, static_cast< const ReClass* >( KeyType ) );
										}
										Key = &FieldCopyInfo->DstData_;
									}
									if( SrcFieldAccessor.isContainerOfPointerValues() )
									{
										Value = *reinterpret_cast< void** >( Value );
										FieldCopyInfo* FieldCopyInfo = getFieldCopyInfo( Value, nullptr, SrcFieldAccessor.getValueUpperClass( Value ) );
										BcAssert( FieldCopyInfo );
										if( !ValueType->getTypeSerialiser()->copy( FieldCopyInfo->DstData_, Value ) )
										{
											copyClassData( FieldCopyInfo->DstData_, Value, static_cast< const ReClass* >( ValueType ) );
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
void ReCopyClass( void* DstObject, void* SrcObject, const ReType* InType )
{
	if( InType->isTypeOf< ReClass >() )
	{
		const ReClass* InClass = static_cast< const ReClass* >( InType );
		ReCopyClass( DstObject, SrcObject, InClass );
	}
	else
	{
		BcAssert( false );
	}
}
			
//////////////////////////////////////////////////////////////////////////
// CopyClass
void ReCopyClass( void* DstObject, void* SrcObject, const ReClass* InClass )
{
	// Now create all objects that exist as pointers in fields, and mark up.
	ObjectCopyContext ObjectCopyContext;
	auto* RootFieldCopyInfo = ObjectCopyContext.getFieldCopyInfo( SrcObject, DstObject, InClass );
	if( RootFieldCopyInfo != nullptr )
	{
		RootFieldCopyInfo->DstData_ = DstObject;
		ObjectCopyContext.gatherFields( SrcObject, InClass );
		ObjectCopyContext.copyClassData( DstObject, SrcObject, InClass );
	}				
}

//////////////////////////////////////////////////////////////////////////
// ConstructObject
ReObject* ReConstructObject( const ReClass* InClass, const std::string& InName, ReObject* InOwner, ReObject* InBasis )
{
	auto NewObject = InBasis == nullptr ? 
		InClass->construct< ReObject >() : InClass->constructNoInit< ReObject >();

	if( NewObject != nullptr )
	{
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
