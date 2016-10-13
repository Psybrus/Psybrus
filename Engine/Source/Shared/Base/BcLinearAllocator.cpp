#include "Base/BcLinearAllocator.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLinearAllocator::BcLinearAllocator( size_t Size, size_t Alignment ):
	Size_( Size ),
	Alignment_( Alignment ),
	Offset_( 0 )
{
	Base_.reset( new BcU8[ Size ] );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcLinearAllocator::~BcLinearAllocator()
{
}

//////////////////////////////////////////////////////////////////////////
// reset
void BcLinearAllocator::reset()
{
	Offset_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// allocate
void* BcLinearAllocator::allocate( size_t Bytes )
{
	Bytes = BcCalcAlignment( static_cast< BcU64 >( Bytes ), static_cast< BcU64 >( Alignment_ ) );
	size_t ThisOffset = Offset_.fetch_add( Bytes );
	if( ( ThisOffset + Bytes ) < Size_ )
	{
		return &Base_[ ThisOffset ];
	}
	return nullptr;
}
